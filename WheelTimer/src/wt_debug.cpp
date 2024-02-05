// wt_debug.cpp: store and output debug info, like measurements, etc
//

#include "wheel_timer.hpp"


cWTimerDebug_::cWTimerDebug_(size_t cap, uint32_t ticks) : _coll{}, _tir{ticks}
{
   try {
      _coll.reserve(cap) ;
   } catch (...) { _coll = {} ; }
}

                                  // external
std::ostream& operator<< (std::ostream& os, const cWTimerDebug_& wtd)
{
   size_t                     count_jits = wtd._coll.size() ;
   cWTimerDebug_::Jitter_type max_jit = 0 ;
   cWTimerDebug_::Jitter_type avg_jit = 0 ;

   os << "> DEBUG > " ;
   if (count_jits > 0) {
      int   max_jit = 0 ;
      os << count_jits << " jitters ( _/! = deadline met/MISSED )\n:: " ;
      uint32_t count = 0 ; uint32_t first_missed = count_jits ;              // cosmetics
      for (const auto [jit, missed_period]: wtd._coll) {
         os << jit << (missed_period ? (first_missed = count, '!') : '_') ;
         avg_jit += jit ;
         if (jit > max_jit)   max_jit = jit ;
         if (++count == wtd._tir) { count = 0 ; os << "\n:: " ; }            // cosmetics
      }
      os << "\n> avg jitter " << (avg_jit / count_jits) << " max_jitter " << max_jit ;
      if (first_missed < count_jits)   os << " > first MISSED " << first_missed << '\n' ;
      else                             os << " > all deadlines met\n" ;
   } else os << "no informations collected" ;

   return os ;
}


// eof wt_debug.cpp
