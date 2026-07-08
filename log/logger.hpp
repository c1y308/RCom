#ifndef LOG_LOGGER_HPP
#define LOG_LOGGER_HPP
#include "../common/declare_singleton.hpp"

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <fstream>
using std::ofstream;

namespace logger{

class LogStream;

class Logger{
    DECLARE_SINGLETON(Logger)
public:
    enum LogLevel{
        LOG_DEBUG = 0,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL,
        LOG_COUNT
    };

    void open(const string &filename);
    void close();
    void log(LogLevel level, const char *message, int line, const char *format, ...);
    void max(int max_bytes);
    void level(LogLevel level);
    void console(bool enable);

    template<typename T>
    Logger& operator<<(const T& msg){
            stream_ << msg;
            return *this;
        }
    
    /* 以流式写入 */
    LogStream log_stream(LogLevel level, const char *file, int line);

private:
    ~Logger();
    void rotate();

private:
    string filename_;

    ofstream fout_;
    std::ostringstream stream_;

    int max_;
    int len_;
    int level_;

    bool console_;
    
    static const char* s_level_[LOG_COUNT];
};


}

#endif
