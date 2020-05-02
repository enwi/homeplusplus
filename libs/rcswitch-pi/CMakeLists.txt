# Custom CMakeLists.txt (not present in original git repo) by Jan Rogall
cmake_minimum_required(VERSION 2.8.3)
project(RCSwitch)

find_library(wiringPi_LIB wiringPi)

set(RCSwitch_SOURCES RCSwitch.cpp)
set(RCSwitch_HEADERS RCSwitch.h)
add_library(RCSwitch STATIC ${RCSwitch_SOURCES})
# headers
target_include_directories(RCSwitch INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
# rcswitch depends on wiringpi
target_link_libraries(RCSwitch ${wiringPi_LIB})
install(TARGETS RCSwitch DESTINATION lib)
install(FILES ${RCSwitch_HEADERS} DESTINATION include)