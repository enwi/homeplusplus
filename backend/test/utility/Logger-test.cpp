#include <iostream>
#include <regex>
#include <sstream>

#include <gtest/gtest.h>

#include "utility/Logger.h"

// TODO: TEST(Logger, Open)
// TODO: TEST(Logger, Close)
// TODO: Also test file logging

TEST(Logger, Debug)
{
    std::stringstream sstream;
    std::streambuf* b = std::cout.rdbuf(sstream.rdbuf());

    Logger l;
    try
    {
        l.SetFileLevel(Logger::LEVEL_NONE);
        // Does not print at levels above debug
        l.SetConsoleLevel(Logger::LEVEL_NONE);
        l.Debug("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_SEVERE);
        l.Debug("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_ERROR);
        l.Debug("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_WARNING);
        l.Debug("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_INFO);
        l.Debug("test");
        EXPECT_TRUE(sstream.str().empty());

        l.SetConsoleLevel(Logger::LEVEL_DEBUG);
        {
            l.Debug("test");
            const std::string s = sstream.str();
            sstream.str("");
            //[dy.mn.year - hh:mm:ss][level]message\n
            std::regex r {R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ DEBUG \]test\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r)) << "s is " << std::quoted(s);
        }
        {
            l.Debug("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            //[dy.mn.year - hh:mm:ss][level]message\n
            std::regex r {
                R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ DEBUG \]hello world\n what a nice time\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r)) << "s is " << std::quoted(s);
        }
    }
    catch (...)
    {
        std::cout.rdbuf(b);
        std::cout.clear();
        throw;
    }
    std::cout.rdbuf(b);
    std::cout.clear();
}

TEST(Logger, Info)
{
    std::stringstream sstream;
    std::streambuf* b = std::cout.rdbuf(sstream.rdbuf());

    Logger l;
    try
    {
        l.SetFileLevel(Logger::LEVEL_NONE);
        // Does not print at levels above debug
        l.SetConsoleLevel(Logger::LEVEL_NONE);
        l.Info("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_SEVERE);
        l.Info("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_ERROR);
        l.Info("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_WARNING);
        l.Info("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_INFO);
        //[dy.mn.year - hh:mm:ss][level]message\n
        std::regex r1 {R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]test\n.*)"};
        std::regex r2 {
            R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]hello world\n what a nice time\n.*)"};
        {
            l.Info("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Info("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
        l.SetConsoleLevel(Logger::LEVEL_DEBUG);
        {
            l.Info("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Info("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            std::regex r {
                R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]hello world\n what a nice time\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
    }
    catch (...)
    {
        std::cout.rdbuf(b);
        std::cout.clear();
        throw;
    }
    std::cout.rdbuf(b);
    std::cout.clear();
}

TEST(Logger, Warning)
{
    std::stringstream sstream;
    std::streambuf* b = std::cout.rdbuf(sstream.rdbuf());

    Logger l;
    try
    {
        l.SetFileLevel(Logger::LEVEL_NONE);
        // Does not print at levels above debug
        l.SetConsoleLevel(Logger::LEVEL_NONE);
        l.Warning("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_SEVERE);
        l.Warning("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_ERROR);
        l.Warning("test");
        EXPECT_TRUE(sstream.str().empty());
        //[dy.mn.year - hh:mm:ss][level]message\n
        std::regex r1 {R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[WARNING\]test\n.*)"};
        std::regex r2 {
            R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[WARNING\]hello world\n what a nice time\n.*)"};
        l.SetConsoleLevel(Logger::LEVEL_WARNING);
        {
            l.Warning("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Warning("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_INFO);
        {
            l.Warning("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Warning("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
        l.SetConsoleLevel(Logger::LEVEL_DEBUG);
        {
            l.Warning("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Warning("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            std::regex r {
                R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]hello world\n what a nice time\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
    }
    catch (...)
    {
        std::cout.rdbuf(b);
        std::cout.clear();
        throw;
    }
    std::cout.rdbuf(b);
    std::cout.clear();
}

TEST(Logger, Error)
{
    std::stringstream sstream;
    std::streambuf* b = std::cout.rdbuf(sstream.rdbuf());

    Logger l;
    try
    {
        l.SetFileLevel(Logger::LEVEL_NONE);
        // Does not print at levels above debug
        l.SetConsoleLevel(Logger::LEVEL_NONE);
        l.Error("test");
        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_SEVERE);
        l.Error("test");
        EXPECT_TRUE(sstream.str().empty());
        //[dy.mn.year - hh:mm:ss][level]message\n
        std::regex r1 {R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ ERROR \]test\n.*)"};
        std::regex r2 {
            R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ ERROR \]hello world\n what a nice time\n.*)"};
        l.SetConsoleLevel(Logger::LEVEL_ERROR);
        {
            l.Error("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Error("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_WARNING);
        {
            l.Error("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Error("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_INFO);
        {
            l.Error("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Error("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
        l.SetConsoleLevel(Logger::LEVEL_DEBUG);
        {
            l.Error("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Error("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            std::regex r {
                R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]hello world\n what a nice time\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
    }
    catch (...)
    {
        std::cout.rdbuf(b);
        std::cout.clear();
        throw;
    }
    std::cout.rdbuf(b);
    std::cout.clear();
}

TEST(Logger, Severe)
{
    std::stringstream sstream;
    std::streambuf* b = std::cout.rdbuf(sstream.rdbuf());

    Logger l;
    try
    {
        l.SetFileLevel(Logger::LEVEL_NONE);
        // Does not print at levels above debug
        l.SetConsoleLevel(Logger::LEVEL_NONE);
        l.Severe("test");
        EXPECT_TRUE(sstream.str().empty());
        //[dy.mn.year - hh:mm:ss][level]message\n
        std::regex r1 {R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[SEVERE \]test\n.*)"};
        std::regex r2 {
            R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[SEVERE \]hello world\n what a nice time\n.*)"};
        l.SetConsoleLevel(Logger::LEVEL_SEVERE);
        {
            l.Severe("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Severe("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_ERROR);
        {
            l.Severe("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Severe("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_WARNING);
        {
            l.Severe("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Severe("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }

        EXPECT_TRUE(sstream.str().empty());
        l.SetConsoleLevel(Logger::LEVEL_INFO);
        {
            l.Severe("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Severe("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
        l.SetConsoleLevel(Logger::LEVEL_DEBUG);
        {
            l.Severe("test");
            const std::string s = sstream.str();
            sstream.str("");
            EXPECT_TRUE(std::regex_match(s, r1)) << "s is " << std::quoted(s);
        }
        {
            l.Severe("hello world\n what a nice time");
            const std::string s = sstream.str();
            sstream.str("");
            std::regex r {
                R"(.*\[\d{2}\.\d{2}\.\d{4} - \d{2}\:\d{2}\:\d{2}\]\[ INFO  \]hello world\n what a nice time\n.*)"};
            EXPECT_TRUE(std::regex_match(s, r2)) << "s is " << std::quoted(s);
        }
    }
    catch (...)
    {
        std::cout.rdbuf(b);
        std::cout.clear();
        throw;
    }
    std::cout.rdbuf(b);
    std::cout.clear();
}

TEST(Logger, GetLevel)
{
    Logger l;
    // GetLevel always returns lowest level
    l.SetFileLevel(Logger::LEVEL_NONE);
    l.SetConsoleLevel(Logger::LEVEL_NONE);
    EXPECT_EQ(Logger::LEVEL_NONE, l.GetLevel());
    l.SetConsoleLevel(Logger::LEVEL_INFO);
    EXPECT_EQ(Logger::LEVEL_INFO, l.GetLevel());
    l.SetFileLevel(Logger::LEVEL_SEVERE);
    EXPECT_EQ(Logger::LEVEL_INFO, l.GetLevel());
    l.SetFileLevel(Logger::LEVEL_DEBUG);
    EXPECT_EQ(Logger::LEVEL_DEBUG, l.GetLevel());
    l.SetConsoleLevel(Logger::LEVEL_NONE);
    EXPECT_EQ(Logger::LEVEL_DEBUG, l.GetLevel());
}