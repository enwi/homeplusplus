cmake_minimum_required(VERSION 3.13)
project(HomePlusPlus LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB HomePlusPlus_SOURCES
    "api/*.cpp"
    "communication/*.cpp"
    "config/*.cpp"
	  "core-api/*.cpp"
    "database/*.cpp"
    "events/*.cpp"
    "main/*.cpp"
    "utility/*.cpp")
file(GLOB HomePlusPlus_PROTO_FILES
    "main/test.proto"
    "api/action.proto"
    "api/device.proto"
    "api/rule.proto"
    "api/rule_conditions.proto"
    "api/sub_actions.proto")

set(HomePlusPlus_LIB_DIR "${PROJECT_SOURCE_DIR}/../libs" CACHE PATH "Library dir")
set(HomePlusPlus_RESOURCE_DIR "${PROJECT_SOURCE_DIR}/../resources" CACHE PATH "Resource dir (includes database folder)")

# All module files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

set(GCC_WARNING_FLAGS "-Wall" "-Wextra" "-pedantic" "-Wno-unknown-pragmas" "-Wno-psabi")
set(CLANG_WARNING_FLAGS "-Wall" "-Wextra" "-pedantic" "-Wno-unknown-pragmas" "-Wno-unused-local-typedef")
set(MSVC_WARNING_FLAGS "/wd4005" "/wd4068" "/wd4244" "/wd4267" "/wd4800" "/wd4127" "/wd4245" "/wd4324")
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CXX_WARNING_FLAGS ${CLANG_WARNING_FLAGS})
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CXX_WARNING_FLAGS ${GCC_WARNING_FLAGS})
elseif(MSVC)
    set(CXX_WARNING_FLAGS ${MSVC_WARNING_FLAGS})
endif()

add_compile_options(${CXX_WARNING_FLAGS})

if(MSVC)
    add_definitions(/DNOMINMAX /DWIN32_LEAN_AND_MEAN=1 /D_CRT_SECURE_NO_WARNINGS)
endif()

# update submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
	option(GIT_SUBMODULE "Check submodules during build" ON)
	if(GIT_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()
endif()

# find_package(SQLite3)
find_package(CryptoPP QUIET)

if(NOT SQLITE3_FOUND)
    message(STATUS "SQLITE3 not found, building it")
    set(SQLITE_DEFAULT_WAL_SYNCHRONOUS 1 CACHE STRING "Set synchronous to normal for WAL mnode")
    set(SQLITE_DEFAULT_MEMSTATUS 0 CACHE STRING "Disable memory usage tracking")
    add_subdirectory("${HomePlusPlus_LIB_DIR}/sqlite-amalgamation" "${CMAKE_CURRENT_BINARY_DIR}/sqlite3" EXCLUDE_FROM_ALL)
    set(SQLITE3_INCLUDE_DIRS "${HomePlusPlus_LIB_DIR}/sqlite-amalgamation")
    set(SQLITE3_INCLUDE_DIR ${SQLITE3_INCLUDE_DIRS})
    set(SQLITE3_LIBRARY "sqlite3")
endif()

if(NOT CryptoPP_FOUND)
    message(STATUS "CryptoPP not found, building it")
    add_subdirectory("${HomePlusPlus_LIB_DIR}/cryptopp" "${CMAKE_CURRENT_BINARY_DIR}/CryptoPP" EXCLUDE_FROM_ALL)
    include_directories("${HomePlusPlus_LIB_DIR}")
endif()

if(MSVC)
	set(protobuf_MSVC_STATIC_RUNTIME OFF)
endif()
set(protobuf_BUILD_TESTS OFF)
add_subdirectory("${HomePlusPlus_LIB_DIR}/protobuf/cmake" "${CMAKE_CURRENT_BINARY_DIR}/protobuf" EXCLUDE_FROM_ALL)
set(PROTOC_EXECUTABLE "protoc")
set(PROTO_GEN_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTOC_FLAGS "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}" "--cpp_out=${PROTO_GEN_PATH}" 
"--proto_path=${HomePlusPlus_LIB_DIR}/protobuf/src")

set(PROTO_HEADERS)
set(PROTO_SOURCES)

foreach(FILENAME ${HomePlusPlus_PROTO_FILES})
	# convert to relative path
	file(RELATIVE_PATH FILE_RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${FILENAME})
	# get relative directory
	get_filename_component(FILE_DIR ${FILE_RELATIVE} DIRECTORY)
	# get name without extension
	get_filename_component(FILE_NAME ${FILE_RELATIVE} NAME_WE)
	# output path of generated file
	set(OUTPUT_PATH "${PROTO_GEN_PATH}/${FILE_DIR}/${FILE_NAME}")

	list(APPEND PROTO_SOURCES "${OUTPUT_PATH}.pb.cc")
	list(APPEND PROTO_HEADERS "${OUTPUT_PATH}.pb.h")
endforeach()

add_custom_command(COMMAND ${PROTOC_EXECUTABLE} ${PROTOC_FLAGS} ${HomePlusPlus_PROTO_FILES} 
	OUTPUT ${PROTO_SOURCES} ${PROTO_HEADERS} COMMENT "Compiling proto files..." DEPENDS ${HomePlusPlus_PROTO_FILES})
add_library(HomePlusPlus_protobuf_messages ${PROTO_SOURCES})
target_include_directories(HomePlusPlus_protobuf_messages PUBLIC ${PROTO_GEN_PATH})
target_link_libraries(HomePlusPlus_protobuf_messages PUBLIC protobuf::libprotobuf)


# required for sqlpp11-connector-sqlite3
include_directories(${SQLITE3_INCLUDE_DIRS})

set(HinnantDate_ROOT_DIR "${HomePlusPlus_LIB_DIR}/hinnant-date/include")
add_subdirectory("${HomePlusPlus_LIB_DIR}/sqlpp11" "${CMAKE_BINARY_DIR}/sqlpp11" EXCLUDE_FROM_ALL)
add_subdirectory("${HomePlusPlus_LIB_DIR}/sqlpp11-connector-sqlite3" "${CMAKE_BINARY_DIR}/sqlpp11-sqlite3" EXCLUDE_FROM_ALL)

add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_DISABLE_STRING_VIEW)
include_directories("${HomePlusPlus_LIB_DIR}/websocketpp")
include_directories("${HomePlusPlus_LIB_DIR}/asio")
include_directories("${HomePlusPlus_LIB_DIR}/nlohmann-json")
include_directories("${HomePlusPlus_LIB_DIR}/polymorphic_value")

#set(HomePlusPlus_SOURCES ${HomePlusPlus_SOURCES} ${ProtobufMessages})

add_executable(HomePlusPlus ${HomePlusPlus_SOURCES} ${PROTO_SOURCES})
set_property(TARGET HomePlusPlus PROPERTY CXX_STANDARD 14)
set_property(TARGET HomePlusPlus PROPERTY CXX_EXTENSIONS OFF)

add_library(HomePlusPlus_LIBRARIES INTERFACE)

MESSAGE("PROTO GEN PATH: ${PROTO_GEN_PATH}")

if(NOT MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(HomePlusPlus PRIVATE "-g")
    endif()
endif()


# Add GTest for tests
if(BUILD_TESTING)
	find_package(GTest QUIET)

	if(NOT GTest_FOUND)
		message(STATUS "GTest not found, building it")

		# Prevent overriding the parent project's compiler/linker
		# settings on Windows
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    set(temp_testing ${BUILD_TESTING})
    set(BUILD_TESTING OFF)
    add_subdirectory("${HomePlusPlus_LIB_DIR}/googletest" "${CMAKE_BINARY_DIR}/GTest")
    set(BUILD_TESTING ${temp_testing})
		set(INSTALL_GTEST OFF)
	endif()
endif()

# abseil
set(temp_testing ${BUILD_TESTING})
set(BUILD_TESTING OFF)
add_subdirectory("${HomePlusPlus_LIB_DIR}/abseil-cpp" "${CMAKE_BINARY_DIR}/abseil-cpp" EXCLUDE_FROM_ALL)
set(BUILD_TESTING ${temp_testing})

# mosquitto
find_package(Mosquitto REQUIRED)
# include_directories(${MOSQUITTO_INCLUDE_DIRS})
target_include_directories(HomePlusPlus_LIBRARIES INTERFACE ${MOSQUITTO_INCLUDE_DIRS})
message(STATUS "libmosquittopp: ${MOSQUITTO_INCLUDE_DIRS} ${MOSQUITTO_LIBRARIES}")


target_link_libraries(HomePlusPlus_LIBRARIES INTERFACE 
    cryptopp-static
    sqlite3
    ${CMAKE_DL_LIBS}
    absl::base
    absl::any
    absl::flat_hash_map
    absl::strings
    absl::utility
    absl::optional
    absl::hash
    absl::variant
    sqlpp11-connector-sqlite3
    sqlpp11
    HomePlusPlus_protobuf_messages
    ${MOSQUITTO_LIBRARIES}
)

# Plugins
add_subdirectory("plugins")

if(BUILD_TESTING)
    set(HomePlusPlus_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_GEN_PATH})
    add_subdirectory("test")
endif()

# link the libraries to the executable
target_link_libraries(HomePlusPlus PUBLIC HomePlusPlus_LIBRARIES)
target_include_directories(HomePlusPlus PUBLIC HomePlusPlus_LIBRARIES)

# make required log dir
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/log")
file(MAKE_DIRECTORY "${PROTO_GEN_PATH}")
# create run file and make it executable
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/tmp/run" "#!/bin/bash\nsudo gdb --args ${CMAKE_CURRENT_BINARY_DIR}/HomePlusPlus -debug -logDir ${CMAKE_CURRENT_BINARY_DIR}/log -dir ${HomePlusPlus_RESOURCE_DIR} -cLogL 0 -logL 0")
file(COPY "${CMAKE_CURRENT_BINARY_DIR}/tmp/run" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE  "${CMAKE_CURRENT_BINARY_DIR}/tmp")
