set(TEST_SOURCES
  "TestMain.cpp"
	"api/Action-test.cpp"
	"api/ActionStorage-test.cpp"
	"api/Rule-test.cpp"
	"api/RuleCondition-test.cpp"
	"api/RuleStorage-test.cpp"
	"api/SubActionImpls-test.cpp"
	"communication/Authenticator-test.cpp"
	"communication/spi-test.cpp"
	"communication/WebsocketChannel-test.cpp"
	"communication/WebsocketCommunication-test.cpp"
	"database/DBActionSerialize-test.cpp"
	"database/DBRuleSerialize-test.cpp"
	"events/ActionsSocketHandler-test.cpp"
	"events/AuthEventHandler-test.cpp"
	"events/EventSystem-test.cpp"
	"events/RulesSocketHandler-test.cpp"
	"main/ArgumentParser-test.cpp"
	"utility/FactoryRegistry-test.cpp"
	"utility/Logger-test.cpp")

get_property(AllHomePlusPlus_SOURCES TARGET HomePlusPlus PROPERTY SOURCES)
add_executable(HomePlusPlus_Test ${TEST_SOURCES} ${AllHomePlusPlus_SOURCES})
message("HomePlusPlus_LIBRARIES: ${HomePlusPlus_LIBRARIES}")
# prevent Main.cpp from defining main(), reduce timeouts
target_compile_definitions(HomePlusPlus_Test PUBLIC MAIN_CPP_NO_MAIN_FUNCTION C_PLUS_PLUS_TESTING)
target_include_directories(HomePlusPlus_Test PUBLIC ${GTest_INCLUDE_DIRS})
target_include_directories(HomePlusPlus_Test PUBLIC HomePlusPlus_LIBRARIES)
target_include_directories(HomePlusPlus_Test PUBLIC ${PROJECT_SOURCE_DIR})
set_property(TARGET HomePlusPlus_Test PROPERTY CXX_STANDARD 14)
set_property(TARGET HomePlusPlus_Test PROPERTY CXX_EXTENSIONS OFF)

option(HomePlusPlus_Test_ASAN "Build address sanitizer tests" OFF)
option(HomePlusPlus_Test_USAN "Build undefined behavior sanitizer tests" OFF)
option(HomePlusPlus_testcov "Build coverage test (only with gcc)" ON)

if(MSVC)
    target_compile_options(HomePlusPlus_Test PRIVATE "/W4")
else()
    target_compile_options(HomePlusPlus_Test PRIVATE ${CXX_WARNING_FLAGS})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(HomePlusPlus_Test PRIVATE "-g")
        if(HomePlusPlus_Test_ASAN)
            #ASan, UBSan
            add_executable(HomePlusPlus_ATest ${TEST_SOURCES} ${AllHomePlusPlus_SOURCES})
            target_compile_definitions(HomePlusPlus_ATest PUBLIC MAIN_CPP_NO_MAIN_FUNCTION C_PLUS_PLUS_TESTING)
            target_include_directories(HomePlusPlus_ATest PUBLIC ${GTest_INCLUDE_DIRS})
            target_include_directories(HomePlusPlus_ATest PUBLIC HomePlusPlus_LIBRARIES)
            target_include_directories(HomePlusPlus_ATest PUBLIC ${PROJECT_SOURCE_DIR})
            set_property(TARGET HomePlusPlus_ATest PROPERTY CXX_STANDARD 14)
            set_property(TARGET HomePlusPlus_ATest PROPERTY CXX_EXTENSIONS OFF)
            target_compile_options(HomePlusPlus_ATest PRIVATE ${CXX_WARNING_FLAGS} "-fsanitize=address" "-fno-omit-frame-pointer" "-g" "-DASAN_DISABLE_DEATH_TESTS=1")


            get_property(HomePlusPlus_ATest_LINK_FLAGS TARGET HomePlusPlus_ATest PROPERTY LINK_FLAGS)
            set_target_properties(HomePlusPlus_ATest PROPERTIES LINK_FLAGS "-fsanitize=address ${HomePlusPlus_ATest_LINK_FLAGS}")
            target_link_libraries(HomePlusPlus_ATest HomePlusPlus_LIBRARIES ${CMAKE_DL_LIBS} gtest gmock)
        endif()

        if(HomePlusPlus_Test_UBSAN)
            add_executable(HomePlusPlus_UTest ${TEST_SOURCES} ${AllHomePlusPlus_SOURCES})
            target_compile_definitions(HomePlusPlus_UTest PUBLIC MAIN_CPP_NO_MAIN_FUNCTION C_PLUS_PLUS_TESTING)
            target_include_directories(HomePlusPlus_UTest PUBLIC ${GTest_INCLUDE_DIRS})
            target_include_directories(HomePlusPlus_UTest PUBLIC HomePlusPlus_LIBRARIES)
            target_include_directories(HomePlusPlus_UTest PUBLIC ${PROJECT_SOURCE_DIR})
            set_property(TARGET HomePlusPlus_UTest PROPERTY CXX_STANDARD 14)
            set_property(TARGET HomePlusPlus_UTest PROPERTY CXX_EXTENSIONS OFF)
            target_compile_options(HomePlusPlus_UTest PRIVATE ${CXX_WARNING_FLAGS} "-fsanitize=undefined" "-g")
            get_property(HomePlusPlus_UTest_LINK_FLAGS TARGET HomePlusPlus_UTest PROPERTY LINK_FLAGS)
            set_target_properties(HomePlusPlus_UTest PROPERTIES LINK_FLAGS "-fsanitize=undefined ${HomePlusPlus_UTest_LINK_FLAGS}")
            target_link_libraries(HomePlusPlus_UTest HomePlusPlus_LIBRARIES ${CMAKE_DL_LIBS} gtest gmock)
        endif()
    endif()
endif()

target_link_libraries(HomePlusPlus_Test PUBLIC HomePlusPlus_LIBRARIES gmock gtest)

if(HomePlusPlus_testcov)
    # Check for coverage test prerequisites
    find_program( GCOV_PATH gcov )
    find_program( LCOV_PATH lcov )

    if(LCOV_PATH AND GCOV_PATH AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        # GCov
        include(CodeCoverage.cmake)
        add_executable(testcov_HomePlusPlus ${TEST_SOURCES} ${AllHomePlusPlus_SOURCES})
        target_link_libraries(testcov_HomePlusPlus HomePlusPlus_LIBRARIES ${CMAKE_DL_LIBS} gtest gmock)
        # prevent Main.cpp from defining main()
        target_compile_definitions(testcov_HomePlusPlus PUBLIC MAIN_CPP_NO_MAIN_FUNCTION C_PLUS_PLUS_TESTING)
        target_include_directories(testcov_HomePlusPlus PUBLIC ${GTest_INCLUDE_DIRS})
        target_include_directories(testcov_HomePlusPlus PUBLIC HomePlusPlus_LIBRARIES)
        target_include_directories(testcov_HomePlusPlus PUBLIC ${PROJECT_SOURCE_DIR})
        set_property(TARGET testcov_HomePlusPlus PROPERTY CXX_STANDARD 14)
        set_property(TARGET testcov_HomePlusPlus PROPERTY CXX_EXTENSIONS OFF)
        target_compile_options(testcov_HomePlusPlus PRIVATE ${CXX_WARNING_FLAGS})
        # this will be already done by APPEND_COVERAGE_COMPILER_FLAGS()
        #set_target_properties(
        #        testcov_HomePlusPlus PROPERTIES
        #        COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage"
        #)
        # Normally this would be -lgcov, but on mac only -Lgcov works
        #set_target_properties(
        #        testcov_HomePlusPlus PROPERTIES
        #        LINK_FLAGS "-O0 -g -Lgcov -fprofile-arcs -ftest-coverage"
        #)
        # exclude some special files we do not want to profile
        set(COVERAGE_EXCLUDES
            '/usr/*'            # unix
            '*/build/*'
            '*/json*'
            '*/test/*'
            '*/asio/*'
            '*/websocketpp/*'   # smarthome
            '*/hueplusplus/*'
            '*/cryptopp/*'
            '*/libs/*'
            '*/v1/*'            # iOS
        )
        APPEND_COVERAGE_COMPILER_FLAGS()
        SETUP_TARGET_FOR_COVERAGE(
                NAME "coveragetest"
                EXECUTABLE testcov_HomePlusPlus
                DEPENDENCIES testcov_HomePlusPlus
        )
    endif()
endif()