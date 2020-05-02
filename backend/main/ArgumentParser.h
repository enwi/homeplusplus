#ifndef _ARGUMENT_PARSER_H
#define _ARGUMENT_PARSER_H
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

#include "../utility/Logger.h"

#pragma region Arguments

/*!
 * \brief Contains all configuration options from the command line.
 * \see parseArguments(int args, char** args)
 */
struct Arguments
{
    /*!
     * \brief The directory the database file is located at.
     */
    std::string m_directory = "";
    /*!
     * \brief The filename of the log file.
     */
    std::string m_logDir = "";
    /*!
     * \brief The Logger::LogLevel of the console.
     */
    Logger::LogLevel m_consoleLogLevel = Logger::LEVEL_NONE;
    /*!
     * \brief The Logger::LogLevel of the log file.
     */
    Logger::LogLevel m_logLevel = Logger::LEVEL_WARNING;
    /*!
     * \brief Whether debug mode is enabled.
     */
    bool m_debug = false;
};

#pragma endregion

#pragma region Parser

/*!
 * \brief Returns the value of the specified command line option.
 * \param begin The beginning of the search area.
 * \param end The end of the search area.
 * \param option The option to search for.
 * \returns The string found behind \c option, or 0 if it was not found.
 */
inline const char* GetCmdOption(const char* const* begin, const char* const* end, const std::string& option)
{
    const char* const* itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

/*!
 * \brief Checks whether a command line option exists.
 * \param begin The beginning of the search area.
 * \param end The end of the search area.
 * \param option The option to search for.
 * \returns Whether \c option was found.
 */
inline bool CmdOptionExists(const char* const* begin, const char* const* end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

/*!
 * \brief Parses Arguments from the command line.
 * \param argc The number of arguments.
 * \param args The command line arguments.
 * \returns The parsed Arguments.
 *
 * Command line options are:
 * \li -help: displays a help message
 * \li -debug
 * \li -dir directory
 * \li -logDir logDir
 * \li -logL logLevel
 * \li -cLogL consoleLogLevel
 */
inline Arguments ParseArguments(int argc, const char* const* args)
{
    Arguments result;
    if (CmdOptionExists(args, args + argc, "-help") || CmdOptionExists(args, args + argc, "--help"))
    {
        std::cout << "Usage: " << std::endl;
        std::cout << args[0] << " [-debug][-dir directory]"
                  << "[-logDir logDir][-logL logLevel][-cLogL cLogLevel]" << std::endl;
        exit(0);
    }
    if (CmdOptionExists(args, args + argc, "-debug"))
    {
        result.m_debug = true;
    }
    else
    {
        result.m_debug = false;
    }
    const char* directory = GetCmdOption(args, args + argc, "-dir");
    if (directory)
    {
        result.m_directory = std::string(directory);
    }
    const char* logDir = GetCmdOption(args, args + argc, "-logDir");
    if (logDir)
    {
        result.m_logDir = logDir;
    }
    const char* logL = GetCmdOption(args, args + argc, "-logL");
    if (logL)
    {
        result.m_logLevel = static_cast<Logger::LogLevel>(atoi(logL));
    }
    const char* cLogL = GetCmdOption(args, args + argc, "-cLogL");
    if (cLogL)
    {
        result.m_consoleLogLevel = static_cast<Logger::LogLevel>(atoi(cLogL));
    }
    return result;
}

#pragma endregion

#endif