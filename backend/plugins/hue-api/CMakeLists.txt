target_sources(HomePlusPlus
    PRIVATE
        HueAPI.cpp
        HueAPI.h
        HueDeviceType.cpp
        HueDeviceType.h
)

find_package(hueplusplus QUIET)
if(NOT hueplusplus_FOUND)
    message(STATUS "hueplusplus not found, building it")
    add_subdirectory("${HomePlusPlus_LIB_DIR}/hueplusplus" "${CMAKE_CURRENT_BINARY_DIR}/hueplusplus" EXCLUDE_FROM_ALL)
endif()

# This call requires CMake 3.13 or later, see next section
target_link_libraries(HomePlusPlus_LIBRARIES INTERFACE hueplusplusstatic)

target_include_directories(HomePlusPlus_LIBRARIES INTERFACE ${CMAKE_CURRENT_LIST_DIR})

