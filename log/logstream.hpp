#ifndef LOG_LOGSTREAM_HPP
#define LOG_LOGSTREAM_HPP
#include "logger.hpp"
#include <sstream>
#include <iostream>

namespace logger{

class LogStream{
public:
    LogStream(Logger::LogLevel level, const char *file, int line)
        : level_(level), file_(file), line_(line) {} 

    /* 利用 RAII，此临时对象析构时将所有的log信息输出 */
    ~LogStream(){
        Logger::get_instance()->log(level_, file_, line_, "%s", stream_.str().c_str());
    }

    template<typename T>
    LogStream& operator<<(const T &msg){
        stream_ << msg;
        return *this;
    }


    // 注意：这里我们假设 m_file和 m_line 指向的字符串在LogStream对象生命周期内始终有效
    // 并且拷贝构造函数复制了other的ostringstream的内容到新对象的ostringstream中
    LogStream(const LogStream &other)
         : level_(other.level_), file_(other.file_), line_(other.line_), stream_(other.stream_.str()) {
        std::cout << "debug logstream!" << std::endl;
    }


private:
    Logger::LogLevel level_;
    const char *file_;
    int line_;
    std::ostringstream stream_;
};


}


#endif  // LOG_LOGSTREAM_HPP
