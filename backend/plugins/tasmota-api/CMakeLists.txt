target_sources(HomePlusPlus
    PRIVATE
        TasmotaAPI.cpp
        TasmotaAPI.h
        TasmotaDeviceType.cpp
        TasmotaDeviceType.h
)

target_include_directories(HomePlusPlus_LIBRARIES INTERFACE ${CMAKE_CURRENT_LIST_DIR})