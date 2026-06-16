#include "logger.hpp"
#include "logstream.hpp"
#include<ctime>
#include<stdexcept>
#include<cstring>
#include<cstdarg>


namespace logger{  

const char* Logger::s_level_[LOG_COUNT] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

Logger::Logger() : max_(0), len_(0), level_(LOG_DEBUG), console_(true) {}

Logger::~Logger() {
    close();
}

void Logger::close()
{
    fout_.close();
}

void Logger::open(const string &filename)
{
    filename_ = filename;
    fout_.open(filename_, std::ios::app);  // 以追加模式打开日志文件(open函数不会创建文件)
    if (fout_.fail()) {
        throw std::runtime_error("Failed to open log file: " + filename_);
        return;
    }
    fout_.seekp(0, std::ios::end);  // 将写指针移动到文件末尾
    len_ = fout_.tellp();           // 获取当前文件长度
}

void Logger::log(LogLevel level, const char *file, int line, const char *format, ...)
{
    if (level < level_) {
        return;
    }

    if(fout_.fail()){
        throw std::runtime_error("Log file is not open: " + filename_);
        return;
    }


    // 构建完整的日志信息
    std::ostringstream log_stream;
    time_t now = time(nullptr);
    struct tm *local_time = localtime(&now);
    char time_buffer[32];
    memset(time_buffer, 0, sizeof(time_buffer));
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);

    int len = 0;
    const char *fmt = "%s %s %s:%d ";
    len = snprintf(nullptr, 0, fmt, time_buffer, s_level_[level], file, line);
    if(len > 0){
        char *header = new char[len + 1];
        snprintf(header, len + 1, fmt, time_buffer, s_level_[level], file, line);
        header[len] = 0;  // 确保字符串以null结尾
        log_stream << header;
        delete[] header;
        len_ += len;
    }

    va_list args_ptr;
    va_start(args_ptr, format);
    len = vsnprintf(nullptr, 0, format, args_ptr);
    va_end(args_ptr);
    if(len > 0){
        char *message = new char[len + 1];
        va_start(args_ptr, format);
        vsnprintf(message, len + 1, format, args_ptr);
        va_end(args_ptr);
        message[len] = 0;  // 确保字符串以null结尾
        log_stream << message;
        delete[] message;
        len_ += len;
    }

    log_stream << "\n";
    const string &log_str = log_stream.str();
    if(console_){
        std::cout << log_str << std::endl;
    }

    fout_ << log_str;
    // 刷新输出缓冲区，确保日志信息及时写入文件
    fout_.flush();

    if(max_ > 0 && len_ >= max_){
        rotate();
    }
}

/* 限制文件大小 */
void Logger::max(int max_bytes)
{
    max_ = max_bytes;
}

void Logger::level(LogLevel level)
{
    level_ = level;
}

void Logger::console(bool enable)
{
    console_ = enable;
}


/* 如果超出文件大小则新创建一个文件 */
void Logger::rotate()
{
    close();
    time_t now = time(nullptr);
    struct tm *local_time = localtime(&now);
    char time_buffer[32];
    memset(time_buffer, 0, sizeof(time_buffer));
    strftime(time_buffer, sizeof(time_buffer), ".%Y-%m-%d_%H-%M-%S", local_time);
    string new_filename = filename_ + "." + time_buffer;
    if (std::rename(filename_.c_str(), new_filename.c_str()) != 0) {
        throw std::runtime_error("Failed to rotate log file: " + filename_);
        return;
    }
    open(filename_);
}

LogStream Logger::log_stream(LogLevel level, const char *file, int line)
{
    return LogStream(level, file, line);
}


}  // namespace logger