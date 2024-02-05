cmake_minimum_required(VERSION 3.5)

project(WheelTimer LANGUAGES CXX)

set(MY_LOGGER_DIR "~/src/Logger")
set(MY_TIME_DIR "~/Study/Timers/Time")

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

add_executable(WheelTimer test_WTimer.cpp ${MY_LOGGER_DIR}/lib/Logger_impl.cpp # Logger (library)
                          ${MY_TIME_DIR}/timing.hpp ${MY_TIME_DIR}/timing.cpp
   src/wheel_timer.hpp       src/wheel_timer.cpp
   src/wt_events.cpp         src/wt_events_db.cpp
   src/wt_debug.cpp
)

target_include_directories(WheelTimer PUBLIC ${MY_LOGGER_DIR}/include ${MY_TIME_DIR})

target_link_libraries(WheelTimer PUBLIC pthread)
