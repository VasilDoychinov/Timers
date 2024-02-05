cmake_minimum_required(VERSION 3.5)

project(PTLib LANGUAGES C)

add_executable(PTLib test_timerLib.c
               src/timerLib.h src/timerLib.c)

target_link_libraries(PTLib PUBLIC rt)

target_include_directories(PTLib PUBLIC ./src)


# include(GNUInstallDirs)
# install(TARGETS PTLib
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
