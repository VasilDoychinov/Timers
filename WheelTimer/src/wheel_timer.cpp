// wheel_timer.cpp: implementation
//


#include "wheel_timer.hpp"

                                                               // cWTimer_:: functionality
                                  // cWTimer_:: constructors, destructor

cWTimer_::cWTimer_(uint32_t capacity, uint32_t period, int dc,
                   size_t deb_cap, std::string&& id)
        : _capacity{capacity}, _period{period}, _delay_corr{dc}
        , _id{std::move(id)}                                             // description
        , _th{}, _sstop{}                                                // the Timer
        , _tick{0}, _rotation{0}                                         // current state
        , _events{}                                                      // the Scheduled
        , _isOK{false}, _deb_coll{deb_cap, capacity}
{

}

cWTimer_::~cWTimer_()
{
   if (*this)   this->stop() ;                                           // stop Timer, if running
   if (_th.joinable())   _th.join() ;

   Log_to(0, "\n> collected information:\n", this->_deb_coll) ;
}


                                  // cWTimer_:: operations
void _timer_function(cWTimer_* wt, std::future<void> stop) ;

bool
cWTimer_::start()
{
   _th = std::thread(std::move(_timer_function), this, this->_sstop.get_future()) ;
   _isOK = true ;                                                        // ie running
   return true ;
}

void
cWTimer_::stop()
{
   this->_sstop.set_value() ;
   this->_isOK = false ;
   return ;
}

                                  // cWTimer_:: operations:: events

bool
cWTimer_::register_event(const cWTimerEvent_& ev, bool fl_cons)          // schedule 'ev', @return - if successful
{
                                                                         // Log_to(0, ": to register const event&: ", ev) ;
   auto  res = this->calc_request(ev, fl_cons) ;
   if (!res)   return true ;                                             // ??? should not be (re)scheduled
   auto [round, tick] = *res ;
                                                                         /* auto key = _events.make_key(round, tick) ;
                                                                         // Log_to(0, ": count of key(", round, ", ", tick, "): ",
                                                                         //           _events.countof(key)) ;*/
   return _events.add_event(_events.make_key(round, tick), ev) ;
}

bool
cWTimer_::register_event(cWTimerEvent_&& ev, bool fl_cons)               // schedule 'ev', @return - if successful
{
                                                                         // Log_to(0, ": to register event&&: ", ev) ;
   auto  res = this->calc_request(ev, fl_cons) ;
   if (!res)   return true ;                                             // ??? should not be (re)scheduled
   auto [round, tick] = *res ;
                                                                         /* auto key = _events.make_key(round, tick) ;
                                                                         // Log_to(0, ": count of key(", round, ", ", tick, "): ",
                                                                         //           _events.countof(key)) ;*/
   return _events.add_event(_events.make_key(round, tick), std::move(ev)) ;
}

                                  // cWTimer_:: private ops

std::optional<cWTimer_::Request_coords>                                  // will be Key in cWTimerEventsDB_
cWTimer_::calc_request(const cWTimerEvent_ &ev, bool fl_cons) &          // depends on the definition of Request_coords
{
   assert(this->_capacity != 0) ;

   auto [ticks, recurr] = ev.in_ticks() ;                                // round, tick, ...
   if (fl_cons && !recurr)     return std::optional<Request_coords>{} ;

   ticks += this->_tick ;
   auto round = this->_rotation + ticks / this->_capacity ;
   ticks %= this->_capacity ;

   return std::optional<Request_coords>{std::make_pair(round, ticks)} ;
}

bool
cWTimer_::execute(AppCB_&& cb, bool isInlay) { //  assert(cb) ;         // execute, as per policy (isInlay)

                                                                        // Log_to(0, ": execute Inlay: ", isInlay, ", app: ", cb ? true : false) ;
   if (isInlay)  { if (cb) return  cb(), true ; }
   // dispatch 'cb' for execution
   return false ;
}
                                  // cWTimer_:: external functions

std::ostream& operator<< (std::ostream& os, const cWTimer_& wt)
{
   os << wt._id << "{slots:" << wt._capacity << ", T:" << wt._period
      << "millis}:" << std::boolalpha << wt._isOK
      << " > rotation:" << wt._rotation << ", tick:" << wt._tick ;
   os << " > # registered events: " << wt._events.size() << wt._events ;
   return os ;
}

void
_timer_function(cWTimer_* wt, std::future<void> stop)           // will operate with Events DB - NOT THREAD SAFE yet
{
   assert(wt && stop.valid()) ;

   const auto desired_period = 1000 * wt->_period ;             // in micro-seconds
   int   avg_jitter = wt->_delay_corr ;                         // compensate for wait_for() delay
   int   jitter = 0 ;
   auto& deb = wt->_deb_coll ;                                  // to collect info into

   auto& tick = wt->_tick ;                                     // co-ordinates
   auto& rotation = wt->_rotation ;
   const auto& capacity = wt->_capacity ;

   Log_to(0, "> Timer started at ", LOG_TIME_LAPSE(Log_start())) ;

   bool     fl_deadline = false ;                               // ::now() - start_tp must be within adjusted period

   auto start_tp = v_time_now() ; decltype(start_tp) end_tp ;
   decltype(wt->_period) period = desired_period - avg_jitter ; // compensate delay
   decltype(period)      work_load_lapse{} ;                    // measure the work-load and take it out from period

   while (stop.wait_for(std::chrono::microseconds(period)) == std::future_status::timeout) {
      // measuring and adjusting section
      end_tp = v_time_now(), jitter = (int)(v_time_lapse(end_tp, start_tp) - period) ; // desired_period) ;
      start_tp = end_tp ;
      avg_jitter = (avg_jitter + jitter) >> 1 ; // div by 2 meant; // if (jitter > max_jitter)     max_jitter = jitter ;
      period = desired_period - avg_jitter ;                    // adjust: delays only expected

      // work-load section, incl internal operations
                                                                /* Log_to(0, "> currently registered ", wt->_events.size(),
                                                                          " events: ", wt->_events) ; */
      for (auto handle = wt->event_extract(rotation, tick) ; handle ;
           handle = wt->event_extract(rotation, tick)) {        // extract all scheduled for {r, t}
                                                                // Log_to(0, ": found <", rotation, ", ", tick, ">") ;
         wt->execute(handle.mapped().call_back(), handle.mapped().is_inlay()) ;
         wt->register_event(std::move(handle.mapped()), true) ; // to be resheduled if Recurrent; otherwise - dropped off
      }

      if (++tick == capacity) { tick = 0, ++rotation ; }        // next {rotation, tick}
      // measure/check section: ::now() - start_tp must be within adjusted period
      work_load_lapse = v_time_lapse(v_time_now(), start_tp) ;

      if (fl_deadline = (work_load_lapse > period)) {           // @end of Tick: period MISSED period < desired_period
         // ??? do something, perhaps
      } // else period -= work_load_lapse ; // ???  to stay within the required period

      // set Debug info
      deb.insert(cWTimerDebug_::Debug_type{jitter, fl_deadline}) ;
      // debug: just completed section
      /**/ Log_to(0, "\n@", LOG_TIME_LAPSE(Log_start()), ": next tick<", rotation, ",", tick,
                ":period:", period, "micros>",
                ":: jitter_was:", jitter, ":: avg work_load_Was: ", work_load_lapse,
                " > debug_size ", deb._coll.size(),
                " > deadline: ", fl_deadline ? "MISSED" : "met", '\n') ; /**/
   }
   Log_to(0, "> _timer_function(): quits after", LOG_TIME_LAPSE(Log_start())) ;

   stop.get() ;                                                 // just in case
} // external cWTimer_::_timer_function()

                                                                // eoc cWTimer_

// eof wheel_timer.cpp
