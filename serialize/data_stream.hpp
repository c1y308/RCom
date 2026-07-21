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

/* vector 容器的写入实现 */
template <typename T>
void DataStream::write(const std::vector<T> & value)
{
    /* 首先写入类型(char) */
    char type = DataType::VECTOR;
    write((char *)&type, sizeof(char));

    /* 然后写入长度(int) */
    int len = value.size();
    write(len);

    /* 转换为char类型写入数据 */
    const T* ptr = value.data();
    const char* char_ptr = reinterpret_cast<const char*>(ptr);
    write(char_ptr, value.size());
}


/* list 容器的写入实现 */
template <typename T>
void DataStream::write(const std::list<T> & value)
{
    /* 首先写入类型(char) */
    char type = DataType::LIST;
    write((char *)&type, sizeof(char));

    /* 然后写入长度(int) */
    int len = value.size();
    write(len);

    /* 遍历list中的每个元素并写入 */
    for (auto it = value.begin(); it != value.end(); it++)
    {
        write((*it));
    }
}


/* map 容器的写入实现 */
template <typename K, typename D>
void DataStream::write(const std::map<K, D> & value)
{
    /* 首先写入类型(char) */
    char type = DataType::MAP;
    write((char *)&type, sizeof(char));

    /* 然后写入长度(int) */
    int len = value.size();
    write(len);

    /* 最后遍历写入 key 和 data */
    for (auto it = value.begin(); it != value.end(); it++)
    {
        write(it->first);
        write(it->second);
    }
}

/* set 容器的写入实现 */
template <typename T>
void DataStream::write(const std::set<T> & value)
{
    /* 首先写入类型(char) */
    char type = DataType::SET;
    write((char *)&type, sizeof(char));

    /* 然后写入长度(int) */
    int len = value.size();
    write(len);

    /* 最后遍历写入 set 中的每个元素 */
    for (auto it = value.begin(); it != value.end(); it++)
    {
        write(*it);
    }
}

/* 枚举类型的写入 */
template <typename T, typename dummy>
void DataStream::write(const T& value){
    /* 将其转换为 int32_t 类型写入 */
    write(static_cast<int32_t>(value));
}

/* 使用递归手法来写入 variadic arguments */
template <typename T, typename... Args>
void DataStream::write_args(const T& head, const Args&... args) {
    write(head);
    write_args(args...);
}

/* 读取指针永远停留在下一个待读取数据的类型位置 */

/* vector 容器的读取 */
template <typename T>
bool DataStream::read(std::vector<T> & value)
{
    value.clear();
    /* 读取数据类型是否符合 */
    if (buf_[pos_] != DataType::VECTOR)
    {
        return false;
    }

    /* 读取此次需要读取数据的长度 */
    ++pos_;
    int len;
    read(len);

    /* 本质调用 read(char*, int)函数进行读取 */
    value.resize(len);
    T* ptr = value.data();
    char* char_ptr = reinterpret_cast<char*>(ptr);
    read(char_ptr, len);
    return true;
}

/* list 容器的读取 */
template <typename T>
bool DataStream::read(std::list<T> & value)
{
    value.clear();
    /* 读取数据类型是否符合 */
    if (buf_[pos_] != DataType::LIST)
    {
        return false;
    }

    /* 读取此次需要读取数据的长度 */
    ++pos_;
    int len;
    read(len);

    /* 本质调用 read(T )函数进行读取 */
    for (int i = 0; i < len; i++)
    {
        T v;
        read(v);
        value.push_back(v);
    }
    return true;
}


template <typename K, typename D>
bool DataStream::read(std::map<K, D> & value)
{
    value.clear();
    /* 读取数据类型是否符合 */
    if (buf_[pos_] != DataType::MAP)
    {
        return false;
    }

    /* 读取此次需要读取数据的长度 */
    ++pos_;
    int len;
    read(len);

    /* 本质调用 read(T )函数进行读取 */
    for (int i = 0; i < len; i++)
    {
        K k;
        read(k);

        D v;
        read(v);
        value[k] = v;
    }
    return true;
}


template <typename T>
bool DataStream::read(std::set<T> & value)
{
    /* 读取数据类型是否符合 */
    value.clear();
    if (buf_[pos_] != DataType::SET)
    {
        return false;
    }

    /* 读取此次需要读取数据的长度 */
    ++pos_;
    int len;
    read(len);

    /* 本质调用 read(T )函数进行读取 */
    for (int i = 0; i < len; i++)
    {
        T v;
        read(v);
        value.insert(v);
    }
    return true;
}

/* 针对 enum 数据类型将其转换为 int32_t进行读取 */
template <typename T, typename dummy>
bool DataStream::read(T& value)
{
    int32_t& intValue = reinterpret_cast<int32_t&>(value);
    return read(intValue);
}

/* 使用递归手法来读取 variadic arguments */
template <typename T, typename ...Args>
bool DataStream::read_args(T & head, Args&... args)
{
    read(head);
    return read_args(args...);
}

template <typename T>
DataStream& DataStream::operator<<(const std::vector<T>& value) {
    write(value);
    return *this;
}

template <typename T>
DataStream& DataStream::operator<<(const std::list<T>& value) {
    write(value);
    return *this;
}

template <typename K, typename D>
DataStream& DataStream::operator<<(const std::map<K, D>& value) {
    write(value);
    return *this;
}

template <typename T>
DataStream& DataStream::operator<<(const std::set<T>& value) {
    write(value);
    return *this;
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, DataStream&>::type
DataStream::operator<<(const T& value) {
    write(value);
    return *this;
}

template <typename T>
DataStream& DataStream::operator>>(std::vector<T>& value) {
    read(value);
    return *this;
}

template <typename T>
DataStream& DataStream::operator>>(std::list<T>& value) {
    read(value);
    return *this;
}

template <typename K, typename D>
DataStream& DataStream::operator>>(std::map<K, D>& value) {
    read(value);
    return *this;
}

template <typename T>
DataStream& DataStream::operator>>(std::set<T>& value) {
    read(value);
    return *this;
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, DataStream&>::type
DataStream::operator>>(T& value) {
    read(value);
    return *this;
}

}  // namespace serialize

#endif  // SERIALIZE_DATA_STREAM_HPP
