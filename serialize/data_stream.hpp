#ifndef SERIALIZE_DATA_STREAM_HPP
#define SERIALIZE_DATA_STREAM_HPP

#include "serializable.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include "../time/time.hpp"

namespace serialize {

class DataStream {
public:
    /* 枚举序列化的数据类型 */
    enum DataType {
        BOOL = 0,
        CHAR,
        INT32,
        INT64,
        UINT32,
        UINT64,
        FLOAT,
        DOUBLE,
        ENUM,
        STRING,
        VECTOR,
        LIST,
        MAP,
        SET,
        CUSTOM
    };

    /* 大端/小端 */
    enum ByteOrder {
        BigEndian,
        LittleEndian
    };

    /* 构造函数：使用 string 或 char* 构造 */
    DataStream();
    explicit DataStream(const std::string &str);
    DataStream(const char *data, std::size_t size);
    ~DataStream() = default;

    /* 打印当前数据 */
    void show() const;

    /* 通用类型的写入 */
    void write(const char* data, int len);
    void write(bool value);
    void write(char value);
    void write(std::int32_t value);
    void write(std::uint32_t value);
    void write(std::int64_t value);
    void write(std::uint64_t value);
    void write(float value);
    void write(double value);
    void write(const char* value);
    void write(const std::string& value);
    void write(const Serializable& value);


    /* STL 容器类型的写入 */
    template <typename T>
    void write(const std::vector<T>& value);

    template <typename T>
    void write(const std::list<T>& value);

    template <typename K, typename D>
    void write(const std::map<K, D> & value);

    template <typename T>
    void write(const std::set<T>& value);

    //采用SFINAE特性保证T为枚举类型
    template <typename T, typename dummy = std::enable_if_t<std::is_enum<T>::value>>
    void write(const T& value);

    template <typename T, typename... Args>
    void write_args(const T &head, const Args&... args);

    void write_args();


    /* 通用类型的写入 */
    bool read(char* data, int len);
    bool read(bool& value);
    bool read(char& value);
    bool read(std::int32_t& value);
    bool read(std::uint32_t& value);
    bool read(std::int64_t& value);
    bool read(std::uint64_t& value);
    bool read(float& value);
    bool read(double& value);
    bool read(std::string& value);
    bool read(Serializable& value);


    /* STL容器类型的写入 */
    template <typename T>
    bool read(std::vector<T>& value);

    template <typename T>
    bool read(std::list<T>& value);

    template <typename K, typename D>
    bool read(std::map<K, D>& value);

    template <typename T>
    bool read(std::set<T>& value);

    //采用SFINAE特性保证T为枚举类型
    template <typename T, typename dummy = std::enable_if_t<std::is_enum<T>::value>>
    bool read(T& value);

    template <typename T, typename... Args>
    bool read_args(T& head, Args&... args);

    bool read_args();

    const char* data() const;
    std::size_t size() const;
    std::size_t byte_size() const;
    void clear();
    void reset();
    bool save(const std::string& filename) const;
    bool load(const std::string& filename);


    /* 操作符重载输出 */
    DataStream& operator<<(bool value);
    DataStream& operator<<(char value);
    DataStream& operator<<(std::int32_t value);
    DataStream& operator<<(std::uint32_t value);
    DataStream& operator<<(std::int64_t value);
    DataStream& operator<<(std::uint64_t value);
    DataStream& operator<<(float value);
    DataStream& operator<<(double value);
    DataStream& operator<<(const char* value);
    DataStream& operator<<(const std::string& value);
    DataStream& operator<<(const Serializable& value);

    template <typename T>
    DataStream& operator<<(const std::vector<T>& value);

    template <typename T>
    DataStream& operator<<(const std::list<T>& value);

    template <typename K, typename D>
    DataStream& operator<<(const std::map<K, D>& value);

    template <typename T>
    DataStream& operator<<(const std::set<T>& value);

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, DataStream&>::type operator<<(const T& value);

    /* 操作符重载输入 */
    DataStream& operator>>(bool& value);
    DataStream& operator>>(char& value);
    DataStream& operator>>(std::int32_t& value);
    DataStream& operator>>(std::uint32_t& value);
    DataStream& operator>>(std::int64_t& value);
    DataStream& operator>>(std::uint64_t& value);
    DataStream& operator>>(float& value);
    DataStream& operator>>(double& value);
    DataStream& operator>>(std::string& value);
    DataStream& operator>>(Serializable& value);

    template <typename T>
    DataStream& operator>>(std::vector<T>& value);

    template <typename T>
    DataStream& operator>>(std::list<T>& value);

    template <typename K, typename D>
    DataStream& operator>>(std::map<K, D>& value);

    template <typename T>
    DataStream& operator>>(std::set<T>& value);

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, DataStream&>::type operator>>(T& value);

private:
    void reserve(std::size_t len);
    ByteOrder byteorder() const;

    std::vector<char> buf_;
    std::size_t pos_;
    ByteOrder byteorder_;
};

}  // namespace serialize

#endif  // SERIALIZE_DATA_STREAM_HPP
