// wt_events: as defined in wheel_timer.hpp: construction, operations, ... (wt_ will stand for wheel_timer_)
//

#include "wheel_timer.hpp"

                                  // cWTimerEvent_:: constructors, ...
cWTimerEvent_::cWTimerEvent_(uint32_t period_in_ticks, bool isR,
                             const AppCB_& func, bool isInlay)
             : _wt_ticks{period_in_ticks}, _is_recurrent{isR}
             , _cb{func}
             , _inlay{isInlay}                                  // call _cb immediately or dispatch it
{

}

                                  // cWTimerEvent_:: helpers
std::ostream& operator<< (std::ostream& os, const cWTimerEvent_& wt)
{
   os << "ev{period:" << wt._wt_ticks << "t, recurrent:"
      << std::boolalpha << wt._is_recurrent << "}" ;
   return os ;
}

// eof wt_events.cpp
