// timerLib.c: provides a WRAPPER around POSIX timers.
//   Based on:
//     - call-back is called asynchronously through a thread
//                    (see struct sigevent::sigev_notify = SIGEV_THREAD)
//     - parameters are passed through sigevent::sigval::sival_ptr(wTimer_t *)::_user_data
//

#include "timerLib.h"

// internal functions

   // function wrapper of User's call-back: used with SIGEV_THREAD
   // => sv.sival_ptr is used (expected to point to wTimer_t)
static void _callback_wrapper(union sigval sv) {
   wTimer_t*  wt = (wTimer_t *)(sv.sival_ptr) ; assert(wt) ;
   if (w_timer_state(wt) == TIMER_RESUMED && wt->_period != 0) w_timer_set_state(wt, TIMER_RUNNING) ;

   wt->_cb(wt, wt->_user_args) ;
}

  // arm/disarm Timer according to what's in ::_ts
  //            use POSIX timer_settime(); @return - if succesful
static bool _timer_arm_disarm(wTimer_t* wt)
{
   // assert(wt) ;
   return timer_settime(wt->_t, 0, &(wt->_ts), NULL) == 0 ;              // see timer_settime() for details
}


// APIs follow

// initialize wTimer_t, @return - ther result
//      settings: SIGEV_THREAD for notifying: see $ man sigevent

bool w_timer_initialize(clockid_t clock,                                 // clock to be used
                        wTimer_t* wt, wTimerCB_t cb, void* ua,           // timer to initialize, callback, user args
                        uint64_t exp, uint64_t period, bool is_exp,      // expiration, next period, is exponential
                        uint32_t mf)                                     // max # of fires
{
   assert(wt && cb) ;
   wt->_state = TIMER_DELETED ;

   wt->_clock = clock,
   wt->_cb = cb, wt->_user_args = ua, wt->_exp = exp, wt->_period = period,
   wt->_is_exponential = is_exp, wt->_exp_back_off = 0, wt->_max_fires = mf ;

   struct sigevent  wev ; memset(&wev, 0, sizeof(wev)) ;                 // for timer_t modes: notification, etc

   // create a POSIX timer: see man sigevent
   wev.sigev_notify = SIGEV_THREAD, wev.sigev_notify_function = _callback_wrapper ;
   wev.sigev_value.sival_ptr = wt ;                                      // to be used wthin call-back
   if (timer_create(wt->_clock, &wev, &(wt->_t)) != 0)   return false ;  // POSIX timer_create might fail

   millis_into_timespec(wt->_exp, &(wt->_ts.it_value)),                  // expiration
   millis_into_timespec(wt->_period, &(wt->_ts.it_interval)) ;           // & period(if any)

   wt->_state = TIMER_INIT ;
   return true ;
} // w_timer_initialize()

// w_timer_start(): from States BUT: _DELETED,
bool w_timer_start(wTimer_t* wt)
{
   assert(wt) ; if (wt->_state == TIMER_DELETED)   return false ;
   assert(wt->_ts.it_value.tv_sec > 0 || wt->_ts.it_value.tv_nsec > 0) ; // as it will be stopped otherwise

   bool res = _timer_arm_disarm(wt) ;                                    // arm it as per ::_ts
   w_timer_set_state(wt, res ? TIMER_RUNNING : TIMER_ERROR) ;
   return res ;
}

unsigned long w_timer_ms_to_fire(wTimer_t* wt)
{
   assert(wt) ;
   struct itimerspec   its ; memset(&its, 0, sizeof(its)) ;

   timer_gettime(wt->_t, &its) ;                                         // the remainning T would be in ::it_value
   return timespec_to_millis(&(its.it_value)) ;
}

void w_timer_pause(wTimer_t* wt)                                         // pause a running Timer, @return - if successful
{
   assert(wt &&
          (w_timer_state(wt) == TIMER_RUNNING || w_timer_state(wt) == TIMER_RESUMED)
         ) ;

   wt->_time_remaining = w_timer_ms_to_fire(wt) ;
   millis_into_timespec(0, &(wt->_ts.it_value)), millis_into_timespec(0, &(wt->_ts.it_interval)) ;
   _timer_arm_disarm(wt), w_timer_set_state(wt, TIMER_PAUSED) ;          // stop POSIX Timer & set state in T.
}

void w_timer_resume(wTimer_t* wt)                                        // resume a paused Timer, @return - if successful
{
   assert(wt && w_timer_state(wt) == TIMER_PAUSED) ;

   millis_into_timespec(wt->_time_remaining, &(wt->_ts.it_value)),       // reset time to fire
   millis_into_timespec(wt->_period, &(wt->_ts.it_interval)) ;           // reset period
   wt->_time_remaining = 0 ;

   _timer_arm_disarm(wt), w_timer_set_state(wt, TIMER_RESUMED) ;         // stop POSIX Timer & set state in T.
}


void w_timer_cancel(wTimer_t* wt)                                        // cancel a running Timer, @return - if successful
{
   assert(wt && w_timer_state(wt) != TIMER_INIT && w_timer_state(wt) != TIMER_DELETED) ;

   wt->_time_remaining = 0, // just in case or, clear all dynamic attrs
   millis_into_timespec(0, &(wt->_ts.it_value)), millis_into_timespec(0, &(wt->_ts.it_interval)) ;
   _timer_arm_disarm(wt), w_timer_set_state(wt, TIMER_CANCELLED) ;       // stop POSIX Timer & set state in T.
}

void w_timer_delete(wTimer_t* wt)                                        // cancel a running Timer, @return - if successful
{
   assert(wt) ;

   wTimerState_t   state = w_timer_state(wt) ;

   if (state == TIMER_RUNNING || state == TIMER_RESUMED || state == TIMER_PAUSED) {
      w_timer_cancel(wt) ;
   }

   // ??? clean all

   w_timer_set_state(wt, TIMER_DELETED) ;       // mark it INOPERATIONAL
}


// helpers

// eof timerLib.c
