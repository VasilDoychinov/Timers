// wt_events_db.cpp: as defined in wheel_timer.hpp: construction, operations, ... (wt_ will stand for wheel_timer_)
//

#include "wheel_timer.hpp"

                                  // cWTimerEventsDB_:: constructors, ...

                                  // cWTimerEventsDB_:: operations
bool
cWTimerEventsDB_::add_event(const Key& k, const Value& v)                // k: contructed with make_key(),
{
   try {
      const auto& it = this->_events.emplace(k, v) ;
                                                                         // Log_to(0, ": just added const&: ", it) ;
   } catch (...) { return false ; }                                      // Strong Exception Safety guarantee
   return true ;
}

bool
cWTimerEventsDB_::add_event(Key&& k, Value&& v)                          // k: contructed with make_key(),
{
   try {
      const auto& it = this->_events.emplace(std::move(k), std::move(v)) ;
                                                                         // Log_to(0, ": just added &&: ", it) ;
   } catch (...) { return false ; }                                      // Strong Exception Safety guarantee
   return true ;
}
                                  // cWTimerEventsDB_:: helpers
std::ostream& operator<< (std::ostream& os, const cWTimerEventsDB_& wt)
{
   // os << "> events DB holds " << wt._events.size() << " events:" ;
   for (const auto& [k, ev] : wt._events) os << "\n: [" << k.first << ", " << k.second << "]: " << ev ;

   return os ;
}

std::ostream& operator<< (std::ostream& os, const cWTimerEventsDB_::Iterator& it)
{                                                                        // an element of events DB
   auto [r, t] = it->first ;
   os << "[" << r << ", " << t << "]: " << it->second ;
   return os ;
}

// eof wt_events_db.cpp
