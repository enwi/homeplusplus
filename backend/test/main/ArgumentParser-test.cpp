#include <gtest/gtest.h>

#include "../SilenceCout.h"
#include "main/ArgumentParser.h"

// Define ASAN_DISABLE_DEATH_TESTS to 1 to disable all death tests, which causes segfaults in ASan
#ifndef ASAN_DISABLE_DEATH_TESTS
#define ASAN_DISABLE_DEATH_TESTS 0
#endif
constexpr bool g_performDeathTests = !ASAN_DISABLE_DEATH_TESTS;

TEST(ArgumentParser, CmdOption)
{
    const char* args[] = {"test_exe", "arg1", "arg2", "arg3", "arg4"};
    const char** begin = std::begin(args);
    const char** end = std::end(args);
    EXPECT_EQ(args[2], GetCmdOption(begin, end, "arg1"));
    EXPECT_EQ(args[3], GetCmdOption(begin, end, "arg2"));
    EXPECT_EQ(args[4], GetCmdOption(begin, end, "arg3"));
    EXPECT_EQ(nullptr, GetCmdOption(begin, end, "arg4"));
    EXPECT_EQ(nullptr, GetCmdOption(begin, end, "-"));
    EXPECT_EQ(nullptr, GetCmdOption(begin, end, ""));
}

TEST(ArgumentParser, CmdOptionExists)
{
    const char* args[] = {"test_exe", "arg1", "arg2", "arg3", "arg4"};
    const char** begin = std::begin(args);
    const char** end = std::end(args);
    EXPECT_TRUE(CmdOptionExists(begin, end, "arg1"));
    EXPECT_TRUE(CmdOptionExists(begin, end, "arg2"));
    EXPECT_TRUE(CmdOptionExists(begin, end, "arg3"));
    EXPECT_TRUE(CmdOptionExists(begin, end, "arg4"));
    EXPECT_FALSE(CmdOptionExists(begin, end, "-"));
    EXPECT_FALSE(CmdOptionExists(begin, end, ""));
}

TEST(ArgumentParserDeathTest, ParseArguments)
{
    SilenceCout silence;
    {
        const char* args[] = {"test_exe", "qjiweo", "-help"};
        if (g_performDeathTests)
        {
            EXPECT_EXIT(ParseArguments(3, args), ::testing::ExitedWithCode(0), "");
        }
    }
    // Default arguments
    Arguments d {};
    {
        const char* args[] = {"test_exe", "-debug"};
        Arguments a = ParseArguments(2, args);
        EXPECT_TRUE(a.m_debug);
        // Other args not changed
        EXPECT_EQ(d.m_directory, a.m_directory);
        EXPECT_EQ(d.m_logDir, a.m_logDir);
        EXPECT_EQ(d.m_consoleLogLevel, a.m_consoleLogLevel);
        EXPECT_EQ(d.m_logLevel, a.m_logLevel);
    }
    {
        const char* args[] = {"test_exe", "-dir", "testdir"};
        Arguments a = ParseArguments(3, args);
        EXPECT_EQ("testdir", a.m_directory);
        // Other args not changed
        EXPECT_EQ(d.m_debug, a.m_debug);
        EXPECT_EQ(d.m_logDir, a.m_logDir);
        EXPECT_EQ(d.m_consoleLogLevel, a.m_consoleLogLevel);
        EXPECT_EQ(d.m_logLevel, a.m_logLevel);
    }
    {
        const char* args[] = {"test_exe", "-logDir", "testdir"};
        Arguments a = ParseArguments(3, args);
        EXPECT_EQ("testdir", a.m_logDir);
        // Other args not changed
        EXPECT_EQ(d.m_directory, a.m_directory);
        EXPECT_EQ(d.m_debug, a.m_debug);
        EXPECT_EQ(d.m_consoleLogLevel, a.m_consoleLogLevel);
        EXPECT_EQ(d.m_logLevel, a.m_logLevel);
    }
    {
        const char* args[] = {"test_exe", "-logL", "3"};
        Arguments a = ParseArguments(3, args);
        EXPECT_EQ(static_cast<Logger::LogLevel>(3), a.m_logLevel);
        // Other args not changed
        EXPECT_EQ(d.m_directory, a.m_directory);
        EXPECT_EQ(d.m_logDir, a.m_logDir);
        EXPECT_EQ(d.m_debug, a.m_debug);
        EXPECT_EQ(d.m_consoleLogLevel, a.m_consoleLogLevel);
    }
    {
        const char* args[] = {"test_exe", "-cLogL", "2"};
        Arguments a = ParseArguments(3, args);
        EXPECT_EQ(static_cast<Logger::LogLevel>(2), a.m_consoleLogLevel);
        // Other args not changed
        EXPECT_EQ(d.m_directory, a.m_directory);
        EXPECT_EQ(d.m_logDir, a.m_logDir);
        EXPECT_EQ(d.m_debug, a.m_debug);
        EXPECT_EQ(d.m_logLevel, a.m_logLevel);
    }
    {
        const char* args[]
            = {"test_exe", "-debug", "-dir", "testdir", "-logDir", "logdir", "-cLogL", "1", "-logL", "3"};
        Arguments a = ParseArguments(10, args);
        EXPECT_EQ(static_cast<Logger::LogLevel>(1), a.m_consoleLogLevel);
        EXPECT_EQ("testdir", a.m_directory);
        EXPECT_EQ("logdir", a.m_logDir);
        EXPECT_EQ(true, a.m_debug);
        EXPECT_EQ(static_cast<Logger::LogLevel>(3), a.m_logLevel);
    }
}