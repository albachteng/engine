#include "../include/Logger.hpp"
#include <ctime>

// Static member definitions
std::unique_ptr<Logger> Logger::s_instance = nullptr;
std::mutex Logger::s_mutex;

Logger::Logger(LogLevel minLevel, LogOutput output) 
    : m_minLevel(minLevel), m_output(output) {
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = std::unique_ptr<Logger>(new Logger());
    }
    return *s_instance;
}

void Logger::initialize(LogLevel minLevel, LogOutput output, const std::string& logFileName) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance = std::unique_ptr<Logger>(new Logger(minLevel, output));
    
    if (output == LogOutput::FILE_ONLY || output == LogOutput::BOTH) {
        s_instance->openLogFile(logFileName);
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::writeToOutputs(const std::string& message, LogLevel level) {
    if (m_output == LogOutput::CONSOLE_ONLY || m_output == LogOutput::BOTH) {
        if (level >= LogLevel::ERROR) {
            std::cerr << message << std::endl;
        } else {
            std::cout << message << std::endl;
        }
    }
    
    if ((m_output == LogOutput::FILE_ONLY || m_output == LogOutput::BOTH) && m_logFile.is_open()) {
        m_logFile << message << std::endl;
        m_logFile.flush(); // Ensure immediate write for important logs
    }
}

void Logger::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    if (level < m_minLevel) {
        return; // Skip logging if below minimum level
    }
    
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] "
        << "[" << logLevelToString(level) << "] ";
    
    if (!file.empty() && line > 0) {
        // Extract just the filename from full path for cleaner output
        size_t lastSlash = file.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos) ? 
                              file.substr(lastSlash + 1) : file;
        oss << "[" << filename << ":" << line << "] ";
    }
    
    oss << message;
    
    writeToOutputs(oss.str(), level);
}

void Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_minLevel = level;
}

void Logger::setOutput(LogOutput output) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_output = output;
}

bool Logger::openLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    
    m_logFile.open(filename, std::ios::app);
    if (m_logFile.is_open()) {
        m_logFile << "\n=== Engine Logger Started: " << getCurrentTimestamp() << " ===\n";
        return true;
    }
    return false;
}

// Convenience methods
void Logger::debug(const std::string& message, const std::string& file, int line) {
    log(LogLevel::DEBUG, message, file, line);
}

void Logger::info(const std::string& message, const std::string& file, int line) {
    log(LogLevel::INFO, message, file, line);
}

void Logger::warn(const std::string& message, const std::string& file, int line) {
    log(LogLevel::WARN, message, file, line);
}

void Logger::error(const std::string& message, const std::string& file, int line) {
    log(LogLevel::ERROR, message, file, line);
}

void Logger::fatal(const std::string& message, const std::string& file, int line) {
    log(LogLevel::FATAL, message, file, line);
}