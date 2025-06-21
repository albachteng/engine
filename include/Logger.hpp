#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * Robust logging system with configurable levels and outputs
 * 
 * Features:
 * - Multiple log levels (DEBUG, INFO, WARN, ERROR, FATAL)
 * - Thread-safe operations
 * - Configurable output destinations (console, file, both)
 * - Automatic timestamping
 * - Zero overhead in release builds when disabled
 * - Easy-to-use macros
 */

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

enum class LogOutput {
    CONSOLE_ONLY,
    FILE_ONLY,
    BOTH
};

class Logger {
private:
    static std::unique_ptr<Logger> s_instance;
    static std::mutex s_mutex;
    
    LogLevel m_minLevel;
    LogOutput m_output;
    std::ofstream m_logFile;
    std::mutex m_logMutex;
    
    Logger(LogLevel minLevel = LogLevel::INFO, LogOutput output = LogOutput::CONSOLE_ONLY);
    
    std::string getCurrentTimestamp() const;
    std::string logLevelToString(LogLevel level) const;
    void writeToOutputs(const std::string& message, LogLevel level);

public:
    ~Logger();
    
    // Singleton access
    static Logger& getInstance();
    static void initialize(LogLevel minLevel = LogLevel::INFO, 
                          LogOutput output = LogOutput::CONSOLE_ONLY,
                          const std::string& logFileName = "engine.log");
    static void shutdown();
    
    // Core logging function
    void log(LogLevel level, const std::string& message, 
             const std::string& file = "", int line = 0);
    
    // Configuration
    void setMinLevel(LogLevel level);
    void setOutput(LogOutput output);
    bool openLogFile(const std::string& filename);
    
    // Convenience methods
    void debug(const std::string& message, const std::string& file = "", int line = 0);
    void info(const std::string& message, const std::string& file = "", int line = 0);
    void warn(const std::string& message, const std::string& file = "", int line = 0);
    void error(const std::string& message, const std::string& file = "", int line = 0);
    void fatal(const std::string& message, const std::string& file = "", int line = 0);
};

// Convenience macros for easy logging with file/line information
#ifdef DEBUG
    #define LOG_DEBUG(msg) Logger::getInstance().debug(msg, __FILE__, __LINE__)
#else
    #define LOG_DEBUG(msg) ((void)0) // No-op in release builds
#endif

#define LOG_INFO(msg)  Logger::getInstance().info(msg, __FILE__, __LINE__)
#define LOG_WARN(msg)  Logger::getInstance().warn(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg, __FILE__, __LINE__)

// Stream-style logging macros for formatted output
#ifdef DEBUG
    #define LOG_DEBUG_STREAM(stream) \
        do { \
            std::ostringstream oss; \
            oss << stream; \
            LOG_DEBUG(oss.str()); \
        } while(0)
#else
    #define LOG_DEBUG_STREAM(stream) ((void)0)
#endif

#define LOG_INFO_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        LOG_INFO(oss.str()); \
    } while(0)

#define LOG_WARN_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        LOG_WARN(oss.str()); \
    } while(0)

#define LOG_ERROR_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        LOG_ERROR(oss.str()); \
    } while(0)

#define LOG_FATAL_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        LOG_FATAL(oss.str()); \
    } while(0)
