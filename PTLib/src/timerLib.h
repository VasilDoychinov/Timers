#ifndef TIMERLIB_H
#define TIMERLIB_H


#include <time.h>
#include <signal.h>

#include <stdint.h>
#include <stdbool.h>
#include <memory.h>

#include <assert.h>


// defined constants

#define NANOS_IN_SEC          1000000000L
#define WTIMER_UNITS_IN_SEC   1000                                      // defines the Units: milli-secs here
#define WTIMER_UNITS_IN_NANOS (WTIMER_UNITS_IN_SEC / NANOS_IN_SEC)      // multiplier nano-s to units
#define WTIMER_NANOS_IN_UNIT  (NANOS_IN_SEC / WTIMER_UNITS_IN_SEC)

typedef enum { TIMER_INIT, TIMER_RUNNING, TIMER_PAUSED,
               TIMER_CANCELLED, TIMER_DELETED, TIMER_RESUMED,
               TIMER_ERROR
} wTimerState_t ;


typedef struct Timer_xxx_ {

  timer_t   _t ;                                                         // POSIX timer
  void     (* _cb)(struct Timer_xxx_*, void *) ;                         // gets access to this & user args
  void*     _user_args ;                                                 // access to user data
  uint64_t  _exp ;                                                       // expires after: in milli-seconds (LINUX)
  uint64_t  _period ;                                                    // for periodic timers, 0 - otherwise
  uint32_t  _max_fires ;                                                 // # to fire, 0 - no limit
  bool      _is_exponential ;
  clockid_t _clock ;                                                     // clock type

  // dynamic attributes
  uint32_t  _count_fires ;                                               // count of fires
  uint64_t  _time_remaining ;                                            // at when being paused

  struct itimerspec  _ts ; // the expiration time needed to start it up
  uint64_t  _exp_back_off ; // for exponential timers

  wTimerState_t _state ;

} wTimer_t ;   // a wrapper arond POSIX timer: all periods in milli-seconds

typedef void (* wTimerCB_t)(wTimer_t *, void *) ;

// APIs

// initialize a Timer into wTimer_t; @return - if successful
//            if successfule - the state is set to _INIT
bool w_timer_initialize(clockid_t clock,                                 // clock to be used
                        wTimer_t* t, wTimerCB_t cb, void* ua,            // timer to initialize, callback, user args
                        uint64_t exp, uint64_t period, bool is_exp,      // expiration, next period, is exponential
                        uint32_t mf) ;                                   // max # of fires

bool w_timer_start(wTimer_t* wt) ;                                       // starts wTimer as per ::_ts
unsigned long w_timer_ms_to_fire(wTimer_t* wt) ;                         // milli-secs to next 'fire'
void w_timer_pause(wTimer_t* wt) ;                                       // pause a running Timer
void w_timer_resume(wTimer_t* wt) ;                                      // resume a paused Timer
void w_timer_cancel(wTimer_t* wt) ;                                      // cancel(stop) a Timer: can be started
void w_timer_delete(wTimer_t* wt) ;                                      // (stop &) delete a Timer:

static inline wTimerState_t w_timer_state(wTimer_t* t)                   // returns the current state
{ return t->_state ; }

static inline wTimerState_t w_timer_set_state(wTimer_t* t, wTimerState_t st) // @return: the old
{ wTimerState_t w = t->_state ; t->_state = st ; return w ; }


// helpers
static inline uint64_t timespec_to_millis(struct timespec* ts)            // @return: ts in milli-secs: enough with Linux
{ return ts->tv_sec * WTIMER_UNITS_IN_SEC + ts->tv_nsec * WTIMER_UNITS_IN_NANOS ; }

static void millis_into_timespec(uint64_t ms, struct timespec* ts)        // milli_secs into timerspec
{ ts->tv_sec = ms / WTIMER_UNITS_IN_SEC, ts->tv_nsec = (ms % WTIMER_UNITS_IN_SEC) * WTIMER_NANOS_IN_UNIT ; }


#endif // TIMERLIB_H
