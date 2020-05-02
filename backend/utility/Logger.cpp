#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip> // std::setw
#include <iostream>
#include <memory>
#include <sstream> // std::stringstream
#include <stdexcept>
#include <thread>

#include <absl/strings/str_cat.h>

#include "consoleColor.h"

/*std::string Logger::s_dirname;
int Logger::s_mday;
std::ofstream Logger::s_file;
std::ofstream Logger::s_csvFile;
Logger::LogLevel Logger::s_fileLevel;
Logger::LogLevel Logger::s_consoleLevel;
std::mutex Logger::s_mutex;*/

void Logger::Open(const std::string& dirname, LogLevel fileLevel, LogLevel consoleLevel)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dirname = dirname;
        m_fileLevel = fileLevel;
        m_consoleLevel = consoleLevel;
    }
    // no lock here, otherwise deadlock in OpenCurrentFile when blocking=true!
    OpenCurrentFile(dirname, true);
}

void Logger::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // Block all outputs
    m_consoleLevel = LEVEL_NONE;
    m_fileLevel = LEVEL_NONE;
    // Close files
    m_file.close();
    m_csvFile.close();
}

void Logger::Log(const std::string& tag, const std::string& message, LogLevel level)
{
    size_t tag_size = tag.size();
    if (tag_size > tag_padding)
    {
        tag_padding = tag_size;
    }
    std::stringstream ss;
    ss << "[" << std::left << std::setw(tag_padding) << tag << "] " << message;
    Log(ss.str(), level);
}

void Logger::Log(const std::string& message, LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (level < m_consoleLevel && level < m_fileLevel)
    {
        return;
    }
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm* time = localtime(&now);
    std::string levelStr;
    // Check if a new file needs to be opened
    if (time->tm_mday != m_mday && m_fileLevel != LogLevel::LEVEL_NONE)
    {
        OpenCurrentFile(m_dirname);
    }
    switch (level)
    {
    case LEVEL_DEBUG:
        console::SetColor(console::color::FG_DARK_GREY);
        levelStr = "[ DEBUG ]";
        break;
    case LEVEL_INFO:
        levelStr = "[ INFO  ]";
        break;
    case LEVEL_WARNING:
        console::SetColor(console::color::FG_MAGENTA);
        levelStr = "[WARNING]";
        break;
    case LEVEL_ERROR:
        console::SetColor(console::color::FG_RED);
        levelStr = "[ ERROR ]";
        break;
    case LEVEL_SEVERE:
        console::SetColor(console::color::FG_RED);
        levelStr = "[SEVERE ]";
        break;
    case LEVEL_NONE:
        levelStr = "[ NONE  ]";
        break;
    }
    std::string formattedMessage = absl::StrCat(GetTimeStringFromTime(time), levelStr, message,"\n");

    if (level >= m_consoleLevel)
    {
        std::cout << formattedMessage;
        console::SetColor(console::color::RESET);
        std::cout << std::flush;
    }
    if (level >= m_fileLevel)
    {
        if (m_file.is_open())
        {
            m_file << formattedMessage << std::flush;
        }
        if (m_csvFile.is_open())
        {
            std::string csvMessage;
            // CSV format: Level(int);Time(int);Message\n
            csvMessage = absl::StrCat(level, ";",  now, ";", message , "\n");
            m_csvFile << csvMessage << std::flush;
        }
    }
}

void Logger::OpenCurrentFile(const std::string& dirname, bool blocking)
{
    const auto updateFileFn = [&, dirname]() {
        time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm* time = localtime(&now);
        std::cout << GetTimeStringFromTime(time) << "[ INFO  ][Logger] Opening log file in dir: \"" << dirname << "\""
                  << std::endl;
        std::string filename;
        if (!dirname.empty())
        {
            filename = dirname + (dirname.back() == '/' ? "" : "/");
        }
        // Reserve 4 for year, 2 for month, 2 for day, 4 for .log, 2 for _
        char fnameBuf[15] = {};
        sprintf(fnameBuf, "%04d_%02d_%02d.log", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
        filename += fnameBuf;
        // Update day now, to prevent multiple calls of this
        m_mday = time->tm_mday;

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open())
        {
            m_file.close();
        }
        if (m_csvFile.is_open())
        {
            m_csvFile.close();
        }
        m_file.open(filename, std::ios_base::app);
        m_csvFile.open(filename + ".csv", std::ios_base::app);
        // Check if files could be opened
        if (m_file.fail() || m_csvFile.fail())
        {
            console::SetColor(console::color::FG_RED);
            std::cout << "--------------------------------------------------" << std::endl
                      << "--------------------------------------------------" << std::endl
                      << "-- Logger ERROR!                                --" << std::endl
                      << "-- Failed to open log file!                     --" << std::endl
                      << "-- Maybe the log dir does not exist?            --" << std::endl
                      << "-- Please create it before continuing!          --" << std::endl
                      << "-- Otherwise all logs of this session are lost! --" << std::endl
                      << "--------------------------------------------------" << std::endl
                      << "--------------------------------------------------" << std::endl;

            console::SetColor(console::color::FG_WHITE);
            // This exception will terminate the entire program because it came from a different thread
            // throw std::ios_base::failure("Failed to open log file with path \"" + filename + "\"");
        }
    };
    std::thread t(updateFileFn);

    if (blocking)
    {
        // Wait for thread to finish
        t.join();
    }
    else
    {
        // Let it run in the background
        t.detach();
    }
}

std::string Logger::GetTimeStringFromTime(tm* time)
{
    char timeStr[32];
    std::sprintf(timeStr, "[%02d.%02d.%04d - %02d:%02d:%02d]", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900,
        time->tm_hour, time->tm_min, time->tm_sec);
    return std::string(timeStr);
}
