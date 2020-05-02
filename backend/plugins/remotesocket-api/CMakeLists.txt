option(HomePlusPlus_REMOTE_SOCKET "Use remote sockets (requires wiringPi)" OFF)

if(HomePlusPlus_REMOTE_SOCKET)
    target_sources(HomePlusPlus
        PRIVATE
            RemoteSocketAPI.cpp
            RemoteSocketAPI.h
            RemoteSocketTransmitter.cpp
            RemoteSocketTransmitter.h
            RemoteSocketType.cpp
            RemoteSocketType.h
    )

     # RCSwitch library
     add_subdirectory("${HomePlusPlus_LIB_DIR}/rcswitch-pi" "${CMAKE_BINARY_DIR}/rcswitch-pi")
     # This call requires CMake 3.13 or later, see next section
     target_link_libraries(HomePlusPlus_LIBRARIES INTERFACE RCSwitch)
     
     # HomePlusPlus_REMOTE_SOCKET is only needed in Main.cpp (not in headers), so it can be private
     target_compile_definitions(HomePlusPlus PRIVATE -DHOMEPLUSPLUS_REMOTE_SOCKET)
     target_include_directories(HomePlusPlus_LIBRARIES INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()
