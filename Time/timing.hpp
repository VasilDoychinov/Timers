// timing.hpp: wrapper around C++ chrono stuff
//

#ifndef TIMING_HPP
#define TIMING_HPP

#include <chrono>
// using std::chrono_literals ;

template <typename Clock = std::chrono::high_resolution_clock>
std::chrono::time_point<Clock> v_time_now() {  return Clock::now() ; }

template <typename Ret = int, typename Clock = std::chrono::high_resolution_clock,
          typename Units = std::chrono::microseconds>
Ret v_time_lapse(std::chrono::time_point<Clock> t2,
                 std::chrono::time_point<Clock> t1)
{
   return std::chrono::duration_cast<Units>(t2 - t1).count() ;
}

#endif // TIMING_HPP
