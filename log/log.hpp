#ifndef __LOG_LOG_HPP__
#define __LOG_LOG_HPP__

#include "logger.hpp"
#include "logstream.hpp"

using namespace logger;

#define Logger_Init(file_name) Logger::Instance()->open(file_name)
#define ADEBUG Logger::get_instance()->log_stream(Logger::LOG_DEBUG,__FILE__, __LINE__)
#define AINFO  Logger::get_instance()->log_stream(Logger::LOG_INFO,__FILE__, __LINE__) 
#define AWARN  Logger::get_instance()->log_stream(Logger::LOG_WARNING,__FILE__, __LINE__) 
#define AERROR Logger::get_instance()->log_stream(Logger::LOG_ERROR,__FILE__, __LINE__)
#define AFATAL Logger::get_instance()->log_stream(Logger::LOG_FATAL,__FILE__, __LINE__)


#define log_debug(format, ...) \
    Logger::get_instance()->log(Logger::LOG_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define log_info(format, ...) \
    Logger::get_instance()->log(Logger::LOG_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define log_warn(format, ...) \
    Logger::get_instance()->log(Logger::LOG_WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define log_error(format, ...) \
    Logger::get_instance()->log(Logger::LOG_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define log_fatal(format, ...) \
    Logger::get_instance()->log(Logger::LOG_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)


#if !defined(RETURN_IF_NULL)
#define RETURN_IF_NULL(ptr)          \
  if (ptr == nullptr) {              \
    AWARN << #ptr << " is nullptr."; \
    return;                          \
  }
#endif


#if !defined(RETURN_VAL_IF_NULL)
#define RETURN_VAL_IF_NULL(ptr, val) \
  if (ptr == nullptr) {              \
    AWARN << #ptr << " is nullptr."; \
    return val;                      \
  }
#endif

#if !defined(RETURN_IF)
#define RETURN_IF(condition)           \
  if (condition) {                     \
    AWARN << #condition << " is met."; \
    return;                            \
  }
#endif

#if !defined(RETURN_VAL_IF)
#define RETURN_VAL_IF(condition, val)  \
  if (condition) {                     \
    AWARN << #condition << " is met."; \
    return val;                        \
  }
#endif

#endif
