#ifndef _LOGGER_H
#define _LOGGER_H
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>

/*!
 * \brief A basic static Logger class with configurable log levels.
 *
 * The log output is written to a file, a .csv file and to stdout.
 * File outputs and console output can have different levels and can be disabled.
 *
 * The Logger should be thread-save.
 *
 * The .csv file contains the same content as the normal log file, but does not format it to be read.
 * Instead, it contains these colums: \n
 * \c level (int value);\c time (as returned by \c time());\c message
 */
class Logger
{
public:
    /*!
     * \brief Represents a LogLevel from debug to none.
     */
    enum LogLevel
    {
        /*!
         * \brief The level for pure debug output.
         */
        LEVEL_DEBUG,
        /*!
         * \brief The level for not very important information.
         */
        LEVEL_INFO,
        /*!
         * \brief The level for warnings which do not have an effect on the function of the program.
         */
        LEVEL_WARNING,
        /*!
         * \brief The level for errors which affect the program, but do not require a restart.
         */
        LEVEL_ERROR,
        /*!
         * \brief The level for severe errors which require the program to restart.
         */
        LEVEL_SEVERE,
        /*!
         * \brief Cannot be selected, used to disable logging.
         */
        LEVEL_NONE
    };

public:
    /*!
     * \brief Opens the Logger.
     * \param dirname The path of the log directory.
     * \param fileLevel The LogLevel of the log file.
     * \param consoleLevel The LogLevel of the console, disabled by default.
     *
     */
    void Open(const std::string& dirname, LogLevel fileLevel, LogLevel consoleLevel = LEVEL_NONE);
    /*!
     * \brief Closes the Logger and the file.
     */
    void Close();

    /*!
     * \brief Sets the LogLevel for file output.
     * \param level The LogLevel for the file. LogLevel::LEVEL_NONE disables file output.
     */
    inline void SetFileLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fileLevel = level;
    }
    /*!
     * \brief Sets the LogLevel for console output.
     * \param level The LogLevel for the console. LogLevel::LEVEL_NONE disables console output.
     */
    inline void SetConsoleLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_consoleLevel = level;
    }

    /*!
     * \brief Outputs a debug message.
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_DEBUG, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Debug(const std::string& message) { Log(message, LEVEL_DEBUG); }
    /*!
     * \brief Outputs a debug message with prepended tag.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_DEBUG, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Debug(const std::string& tag, const std::string& message) { Log(tag, message, LEVEL_DEBUG); }
    /*!
     * \brief Outputs an info message.
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_INFO, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Info(const std::string& message) { Log(message, LEVEL_INFO); }
    /*!
     * \brief Outputs a debug message with prepended tag.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_INFO, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Info(const std::string& tag, const std::string& message) { Log(tag, message, LEVEL_INFO); }
    /*!
     * \brief Outputs a warning message.
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_WARNING, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Warning(const std::string& message) { Log(message, LEVEL_WARNING); }
    /*!
     * \brief Outputs a debug message with prepended tag.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_WARNING, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Warning(const std::string& tag, const std::string& message) { Log(tag, message, LEVEL_WARNING); }
    /*!
     * \brief Outputs an error message.
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_ERROR, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Error(const std::string& message) { Log(message, LEVEL_ERROR); }
    /*!
     * \brief Outputs a debug message with prepended tag.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_ERROR, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Error(const std::string& tag, const std::string& message) { Log(tag, message, LEVEL_ERROR); }
    /*!
     * \brief Outputs a severe error message.
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_SEVERE, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Severe(const std::string& message) { Log(message, LEVEL_SEVERE); }
    /*!
     * \brief Outputs a debug message with prepended tag.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     *
     * If the LogLevel is higher than LEVEL_SEVERE, the message is not printed.
     *
     * The output includes current date, time and the level of the message.
     */
    inline void Severe(const std::string& tag, const std::string& message) { Log(tag, message, LEVEL_SEVERE); }
    /*!
     * \brief Returns the lowest LogLevel specified.
     * \returns The lower LogLevel (displaying more messages) of file and console.
     */
    inline LogLevel GetLevel()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_fileLevel < m_consoleLevel ? m_fileLevel : m_consoleLevel;
    }
    /*!
     * \brief Suggest a padding width for tag logs
     * \param suggestion A width (number of chars) to suggest
     */
    void SuggestTagPadding(size_t suggestion)
    {
        if (suggestion > tag_padding)
        {
            tag_padding = suggestion;
        }
    }

private:
    /*!
     * \brief Outputs a message if \c level it is not lower than the current LogLevel.
     * \param message The message to output.
     * \param level The LogLevel of the message. Should not be LEVEL_NONE.
     *
     * The output includes current date, time and the level of the message.
     */
    void Log(const std::string& message, LogLevel level);

    /*!
     * \brief Outputs a message with prepended tag if \c level it is not lower than the current LogLevel.
     * \param tag The tag to be prepended to message
     * \param message The message to output.
     * \param level The LogLevel of the message. Should not be LEVEL_NONE.
     *
     * The output includes current date, time and the level of the message.
     */
    void Log(const std::string& tag, const std::string& message, LogLevel level);

    /*!
     * \brief Opens the current log file.
     * \param dirname The name of the log directory.
     * \param blocking Indicates whether this function should block or run in the background.
     *
     * The files inside the log directory have the name "yyyy_mm_dd.log". There is a new file for each day.
     *
     * \todo Compress or remove files older than one week.
     * \todo Create log dir if it does not exist.
     */
    void OpenCurrentFile(const std::string& dirname, bool blocking = false);

    /*!
     * \brief Returns a formatted string of the supplied time
     * \param time The time to format
     * \return String containing formatted time
     */
    std::string GetTimeStringFromTime(tm* time);

private:
    /*!
     * \brief The filename of the log directory.
     */
    std::string m_dirname;
    /*!
     * \brief The current day of the month for which the log file is opened.
     */
    int m_mday;
    /*!
     * \brief The log file output stream.
     */
    std::ofstream m_file;
    /*!
     * \brief The log .csv file output stream.
     *
     * Used to provide easier access to log data.
     */
    std::ofstream m_csvFile;
    /*!
     * \brief The LogLevel for file output.
     */
    LogLevel m_fileLevel;
    /*!
     * \brief The LogLevel for console output.
     */
    LogLevel m_consoleLevel;
    /*!
     * \brief Is locked to prevent splitting of messages.
     */
    std::mutex m_mutex;
    /*!
     * \brief Variable to hold the total padding of tag log
     */
    size_t tag_padding;
};
#endif
