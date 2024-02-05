// test_timerLib.c: test timerLib
//

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

// #include <pthread.h>

#include "timerLib.h"

typedef struct {
   int    _ch ;
   char*  _txt ;
} Menu_t ;

// helpers

unsigned long long v_time_ns_lapse(struct timespec* t1, struct timespec* t0) ;   // t1 >= t0


void usr_call_back(wTimer_t* wt, void* data) ;

int main()
{
    printf("\n> Hello World! [main() thread id: %d]", gettid());

    Menu_t  menu_txt = { 0, "1)Pause 2)Resume 3)Restart 4)Reschedule 5)Cancel 6)Remaining Time 7)Quit: " } ;

    wTimer_t   timer ;
    bool res = w_timer_initialize(CLOCK_MONOTONIC, &timer, usr_call_back, (void *)&menu_txt, 5000, 5000, false, 0) ;
    if (res)   {
       if (!w_timer_start(&timer)) { printf("> w_timer_start(): error") ; }
    }
    printf("\n> main() paused > timer running...\n") ;

    while (true)   {
       int  choice ; scanf("%d", &choice) ;
       switch (choice)   {
          case 1: printf("> pause timer") ; break ;
          case 2: printf("> resume timer") ; break ;
          case 3: printf("> restart timer") ; break ;
          case 4: printf("> reschedule timer") ; break ;
          case 5: printf("> cancel timer") ; break ;
          case 6: printf("> state & remaining time") ; break ;
          default: break ;
       }
       if (choice == 7)   break ;
    }
    printf("\n> to delete timer\n") ;

    printf("\n\n> That's it...\n") ;
    return 0;
}

void usr_call_back(wTimer_t *wt, void *data)
{
   struct timespec   tp ; clock_gettime(wt->_clock, &tp) ;

   assert(wt) ;
   pid_t   id = gettid() ;                                               // calling thread id

   Menu_t* m = (Menu_t *)data ;

   printf("\n> called from thread (%d) @ %lds > %s", id, tp.tv_sec, m->_txt) ;
   struct timespec ntp ; clock_gettime(wt->_clock, &ntp) ;
   printf("\n: quits to calling thread (%d) after @ %lluns\n", id, v_time_ns_lapse(&ntp, &tp)) ;
}
/* void usr_call_back(wTimer_t *wt, void *data)
{
   assert(wt) ;
   struct timespec   tp ; clock_gettime(wt->_clock, &tp) ;

   Menu_t* m = (Menu_t *)data ;

   printf("@ %lds > %s\n", tp.tv_sec, m->_txt) ;

   fd_set reads ; FD_ZERO(&reads) ; FD_SET(fileno(stdin), &reads) ;
   struct timeval tout = { 0, 0 } ;
   tout.tv_sec = wt->_ts.it_interval.tv_sec - 1 ;
   tout.tv_usec = wt->_ts.it_interval.tv_nsec / 1000 ;
   // long sss = wt->_ts.it_interval.tv_sec ;
   // long msms = wt->_ts.it_interval.tv_nsec ;
   // printf("\n: period: %ld s %ld ms\n", sss, msms) ;

   int res = select(fileno(stdin) + 1, &reads, NULL, NULL, &tout) ;
   printf("\n: select() returns %d\n", res) ;

   int  choice = 12 ;
   if (FD_ISSET(fileno(stdin), &reads))  {
      // std::cout << "\n> stop ? (y/n): " << std::flush ;
      // clear stdin
      char lbuff[100] ; fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
      // get the choice
      fgets(lbuff, sizeof(lbuff), stdin), fflush(stdin) ;
      choice = atoi(lbuff) ;
      printf("\n: res %d; choice %d: ", res, choice) ;
   }
} */

// helpers

unsigned long long v_time_ns_lapse(struct timespec* t1, struct timespec* t0)   // t1 >= t0
{
   unsigned long long s1  = t1->tv_sec, s0 = t0->tv_sec ;
   unsigned long long ns1 = t1->tv_nsec, ns0 = t0->tv_nsec ;
   assert(s1 > s0 || (s1 == s0 && ns1 >= ns0)) ; // ??? check for ULL_t capacity to hold the result

   return s1 == s0 ? ns1 - ns0
                   :  (s1 - 1 - s0) * NANOS_IN_SEC + NANOS_IN_SEC + ns1 - ns0 ;
}
