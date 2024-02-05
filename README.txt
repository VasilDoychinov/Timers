   TIMERS and Timer Wrappers
   
> INCLUDES   

>>> Project 1: POSIX TImers Wrapper
 
    Tools: C, POSIX Timers, Qt(CMake)
          -> Targets Timer Library as presented in PTLib directory
          
    Directory structure: (Source Tree)
       - PTLib: main directory contains
                - (README file) 
                - Qt Project files
                - test_timerLib.c: some tests
                - src/: source code in timerLib.[c,h]
                    . timerLib.h: types and function prototypes
                    . timerLib.c: APIs
                    
    Current State: prototype
    
>>> Project 2: Wheel Timer & Thread Pool:  Real Time level Guranteed: Best Effort

    Tools: C++ 17, STL, Qt(Cmake)
          -> Targets cWTImer_ class
          
    Directory structure" (Source Tree)
       - WheelTimer: main directory contains
                - (README file) 
                - Qt Project files
                - test_WTimer.cpp: some tests
                - src/: source code files
                    . wheel_timer.hpp: major types, ...
                    . wheel_timer.cpp: cWTimer_ class implementation
                    . wt_debug.cpp: debug (& control) utilities
                    . wt_events[_db].cpp: events handling
                - ../Time/: std::chrono:: wrappers in the contained files
                    
    Current State: prototype
                

