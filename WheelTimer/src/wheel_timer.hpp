// wheel_timer.hpp: sample implementation of a Periodic Wheel Timer
//    ( BEST EFFORT: scheduling through sleep_for() / wait_for() compensated for the expected delay )
//                  Requires C++ 17
//    Basics:
//    - Timer will run as a separate thread
//    - class cWTimer_ defined
//    - two dimensional co-ordinates: round (x ...) x tick (2x for now but could be extended)
//      NB: an event will be executed at (Round, tick) + Event's period (in ticks)
//    - events registering is NOT THREAD-SAFE yet
//

#ifndef WHEEL_TIMER_HPP
#define WHEEL_TIMER_HPP

#include <iostream>

#include <stdint.h>
#include <utility>
#include <optional>

#include <map>

#include <functional>
#include <thread>
#include <future>

// #include <chrono>

#include <assert.h>
#include <sstream>                                                       // for debug

#include "Logger_decl.hpp"                                               // Logger: for debug
#include "Logger_helpers.hpp"

#include "timing.hpp"                                                    // wrappers around std::chrono



using WTimerCB_t = void* (*)(void *, size_t) ;                           // C-style; alternatives - bind(), std::function<>, ...

struct AppCB_ { // packed with agrs(C-style or not), avoiding gcc -fconcepts-ts that would allow for auto parameters

  public:
    using WTimerCB_return_t = void * ;
    WTimerCB_return_t operator() () { return _cb(_cb_args, _args_size) ; }

  public:
    AppCB_(WTimerCB_t cb, void* args = nullptr, size_t args_size = 0)
          : _cb{cb}, _cb_args{args}, _args_size{args_size} {}
    // all specials  = default (for now):: due to simplicity, opposite to std::function<> - ??? options for concurrent Apps

    constexpr operator bool() const& { return _cb != nullptr ; }         // if a Valid call-back
    void   set_args(void* args, size_t size) & { _cb_args = args, _args_size = size ; }

  private:
    WTimerCB_t   _cb{} ;                                                 // call-back (C-style)
    void*        _cb_args{} ;                                            // ??? C-style arguments for the call-back
    size_t       _args_size{0} ;                                         // ... size of args
}; // struct AppCB_

class cWTimerEvent_ {    // ??? Move-only

  public:
                                  // constructors & destructor
    cWTimerEvent_(uint32_t period_in_ticks, bool isRecurrent = false,
                  const AppCB_& func = AppCB_{nullptr}, bool isInlay = false) ;
    // all specials  = default ;

                                  // operations
    decltype(auto) in_ticks() const& { return std::make_pair(_wt_ticks, _is_recurrent) ; }

    bool           is_recurrent() const& { return _is_recurrent ; }
    bool           is_inlay()     const& { return _inlay ; }
    AppCB_         call_back()    const& { return _cb ; }

                                  // helpers
    friend std::ostream& operator<< (std::ostream& os, const cWTimerEvent_& wt) ;

  private:
                                  // properties
    uint32_t   _wt_ticks{0} ;                                            // period in Sequencer's ticks
    bool       _is_recurrent{false} ;                                    // periodic or one-time event

    AppCB_     _cb ;                                                     // to be executed
    bool       _inlay{false} ;                                           // call _cb immediately or dispatch it
}; // class cWTimerEvent_: still a mark only


class cWTimerEventsDB_ {          // Storage and access to scheduled Events

  using Key = std::pair<uint32_t, uint32_t> ;                            // Tick coords: execute it at: 1st is 0, 2nd is Current
  using Value = cWTimerEvent_ ;
  using Collection = std::multimap<Key, Value> ;                         // the Container: ??? Hash table
  using ConstIterator = Collection::const_iterator ;
  using Iterator = Collection::iterator ;
  using Element_type = Collection::value_type ;

  private:

  public:
                                  // constructors & destructor
    // explicit cWTimerEventsDB_() : _ticks_in_round{0}, _events{}, _sth{} {}
    explicit cWTimerEventsDB_() : _events{}, _sth{} {}
    cWTimerEventsDB_(cWTimerEventsDB_&&) = default ;
    cWTimerEventsDB_& operator= (cWTimerEventsDB_&&) = default ;
    ~cWTimerEventsDB_() { if (_sth.joinable()) _sth.join() ; }

                                  // operations
    template <typename ... Args> Key make_key(Args... args) { // prepare & return a Key
                /* Args... to key in updates */
                auto   t = std::forward_as_tuple(args...) ;
                return std::make_pair(std::get<0>(t), std::get<1>(t)) ; // the current design of Key
             }

    template <typename ... Args> decltype(auto) extract(Args... args)   // prepare & return a Node; see make_key()
             { return _events.extract(this->make_key(std::forward<Args>(args)...)) ; }

    bool add_event(const Key& k, const Value& v) ;
    bool add_event(Key&& k, Value&& v) ;

                                  // descriptive
    size_t size() const& { return _events.size() ; }
    size_t countof(const Key& k) const& { return _events.count(k) ; }

                                  // helpers
    friend std::ostream& operator<< (std::ostream& os, const cWTimerEventsDB_& wt) ;
    friend std::ostream& operator<< (std::ostream& os, const Iterator& it) ;

  private:

    // uint32_t      _ticks_in_round{0} ;                                   // # of ticks in a round: two levels only
    Collection    _events{} ;                                            // for all scheduled events
    std::thread   _sth{} ;                                               // ??? the Managing thread
}; // class cWTimerEventsDB_


struct cWTimerDebug_ {            // store and output debug info, like measurements, etc
   using Jitter_type = int ;
   using Debug_type = std::pair<Jitter_type, bool> ;   // jitter x (if adjusted period met)

   public:
     explicit cWTimerDebug_(size_t cap = 0, uint32_t ticks = 0) ;
     template <typename T>
     void insert(T&& el) { if (_coll.size() < _coll.capacity()) _coll.emplace_back(std::forward<T>(el)) ; }

     friend std::ostream& operator<< (std::ostream& os, const cWTimerDebug_& wtd) ;

   public:
     std::vector<Debug_type>   _coll{} ;
     uint32_t                  _tir{} ;                                  // cosmetics targetted
}; // struct cWTimerDebug_


class cWTimer_ { // not a template as to have the possibility of changing characteristics in run-time
  using Rotation_t = uint32_t ;
  using Tick_t = uint32_t ;
  using Request_coords = std::pair<Rotation_t, Tick_t> ;

  private:
                                  // operations
    std::optional<Request_coords> calc_request(const cWTimerEvent_& ev,  // ev would be scheduled for (rotation, tick)
                                               bool fl_cons = false) & ; // 1st call: ignore _is_recurrent flag

    decltype(auto) event_extract(Rotation_t r, Tick_t t) &               // @return the extracted with Key{r, t}
                   { return this->_events.extract(r, t) ; }

    bool execute(AppCB_&& cb, bool isInlay) ;                            // cb is to be executed or send for execution

  public:
                                  // constructors & destructor
    explicit cWTimer_(uint32_t capacity, uint32_t period,                // {# slots, period in millis}
                      int   delay_correction = 0,                        // compensate for wait_for() delay
                      size_t deb_capacity = 0,                           // capacity of debug collection
                      std::string&& id = {}) ;
    ~cWTimer_() ;

                                  // operations
    bool start() ;                                                       // commence Scheduling; @return: if successful
    void stop() ;                                                        // send a signal to stop

    /*bool register_event(AppCB_ cb *??? Copiable/Movable *,             // C-style
                        void* args, size_t args_size * C-style * ,
                        uint32_t period  * in Timer ticks * ,
                        bool is_recurrent * if periodic or one-time *
                       ) ; // register cWTimerEvent_(ie place it in cWTimerEventsDB_, @return - is success
    **/
    bool register_event(const cWTimerEvent_& ev, bool fl_cons = false) ; // schedule 'ev', @return - if successful
    bool register_event(cWTimerEvent_&& ev, bool fl_cons = false) ;      // ...

                                  // descriptive
    operator bool() const& { return _isOK ; }

                                  // external
    friend std::ostream& operator<< (std::ostream& os, const cWTimer_& wt) ;

    friend void _timer_function(cWTimer_* wt, std::future<void> stop) ;  // the Scheduler() actually: called in start()
                                                                         // (incl. repositioning of periodic ones)
                                                                         // current slot events

  private:
                                  // properties:
    const uint32_t    _capacity{0} ;                                     // =:: # of slots
    uint32_t    _period{0} ;                                             // period(of a tick) in milli-seconds
    int         _delay_corr{0} ;                                         // correct delay of wait_for() in micros: initial
    std::string _id{} ;                                                  // Id

    std::thread          _th{} ;                                         // thread performing
    std::promise<void>   _sstop{} ;                                      // signal STOP to _timer_function()

                                  // dynamic attributes
    Rotation_t   _tick ;                                                 // # of the current slot
    Tick_t       _rotation ;                                             // ...  rotation

    cWTimerEventsDB_   _events{} ;                                       // all Scheduled events

    bool            _isOK{false} ;
    cWTimerDebug_   _deb_coll{} ;                                        // collect debug information
}; // class cWTimer_

#endif // WHEEL_TIMER_HPP
