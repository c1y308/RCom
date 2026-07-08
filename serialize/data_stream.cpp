#include "data_stream.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
namespace serialize {
namespace {

template <typename T>
void reverse_bytes(T& value) {
    char* first = reinterpret_cast<char*>(&value);
    char* last = first + sizeof(T);
    std::reverse(first, last);
}

}  // namespace

DataStream::DataStream()
    : pos_(0), byteorder_(byteorder()) {}


DataStream::DataStream(const std::string& str)
    : DataStream(str.data(), str.size()) {}

DataStream::DataStream(const char* data, std::size_t size)
    : pos_(0), byteorder_(byteorder()) {

    buf_.clear();  //清空vector
    reserve(size);
    write(data, size);
}

/* 判断机器大小端 */
DataStream::ByteOrder DataStream::byteorder() const {
    const std::uint32_t value = 0x12345678U;
    char bytes[sizeof(value)] = {};
    std::memcpy(bytes, &value, sizeof(value));
    return static_cast<unsigned char>(bytes[0]) == 0x12U ? BigEndian : LittleEndian;
}


void DataStream::show() const
{
    int size = buf_.size();
    std::cout << "data size = " << size << std::endl;

    int i = 0;  // 每次读取完毕永远指向下一个待读的位置
    while (i < size)
    {
        switch ((DataType)buf_[i])
        {
            case DataType::BOOL:
                if (static_cast<bool>(buf_[++i]) == false)
                    std::cout << "false";
                else
                    std::cout << "true";
                ++i;
                break;
                
            case DataType::CHAR:
                std::cout << static_cast<char>(buf_[++i]);
                ++i;
                break;
                
            case DataType::INT32:
                std::cout << *((int32_t *)(&buf_[++i]));
                i += 4;
                break;

            case DataType::INT64:
                std::cout << *((int64_t *)(&buf_[++i]));
                i += 8;
                break;

            case DataType::FLOAT:
                std::cout << *((float *)(&buf_[++i]));
                i += 4;
                break;

            case DataType::DOUBLE:
                std::cout << *((double *)(&buf_[++i]));
                i += 8;
                break;

            case DataType::STRING:
                if ((DataType)buf_[++i] == DataType::INT32)
                {
                    int len = *((int *)(&buf_[++i]));
                    i += 4;
                    std::cout << std::string(&buf_[i], len);
                    i += len;
                }
                else
                {
                    throw std::logic_error("parse string error");
                }
                break;

            case DataType::VECTOR:
                if ((DataType)buf_[++i] == DataType::INT32)
                {
                    int len = *((int *)(&buf_[++i]));
                    i += 4;
                }
                else
                {
                    throw std::logic_error("parse vector error");
                }
                break;
            case DataType::MAP:
                if ((DataType)buf_[++i] == DataType::INT32)
                {
                    int len = *((int *)(&buf_[++i]));
                    i += 4;
                }
                else
                {
                    throw std::logic_error("parse map error");
                }
                break;

            case DataType::SET:
                if ((DataType)buf_[++i] == DataType::INT32)
                {
                    int len = *((int *)(&buf_[++i]));
                    i += 4;
                }
                else
                {
                    throw std::logic_error("parse set error");
                }
                break;

            case DataType::CUSTOM:
                break;
            default:
                break;
            }
    }
    std::cout << std::endl;
}

/* 每次写入时调用，判断 vector 是否需要再分配 (len 为 char 类型的个数) */
void DataStream::reserve(std::size_t len) {
    if (len > buf_.max_size() - buf_.size()) {
        throw std::length_error("wanted DataStream buffer is too large");
    }
    const std::size_t required = buf_.size() + len;
    if (required > buf_.capacity()) {
        buf_.reserve(required);
    }
}

/* 基础 wrtie 函数：依据char* 和 len 进行逐字节写入（本质调用 memcpy 函数） */
void DataStream::write(const char* data, int len) {
    if (len < 0) {
        throw std::invalid_argument("DataStream::write length is negative");
    }
    if (len == 0) {
        return;
    }
    if (data == nullptr) {
        throw std::invalid_argument("DataStream::write data is null");
    }

    reserve(len);
    int size = buf_.size();
    buf_.resize(buf_.size() + len);
    std::memcpy(&buf_[size], data, static_cast<std::size_t>(len));
}

/* 基础数据类型的写入 */
void DataStream::write(bool value) {
    char type = DataType::BOOL;
    write((char *)&type, sizeof(char));
    write((char *)&value, sizeof(char));
}

void DataStream::write(char value) {
    char type = DataType::CHAR;
    write((char *)&type, sizeof(char));
    write((char *)&value, sizeof(char));
}

void DataStream::write(std::int32_t value) {
    char type = DataType::INT32;
    write((char *)&type, sizeof(char));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    write((char *)&value, sizeof(int32_t));
}

void DataStream::write(std::uint32_t value) {
    char type = DataType::UINT32;
    write((char *)&type, sizeof(char));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(uint32_t);
        std::reverse(first, last);
    }
    write((char *)&value, sizeof(uint32_t));
}

void DataStream::write(std::int64_t value) {
    char type = DataType::INT64;
    write((char *)&type, sizeof(char));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    write((char *)&value, sizeof(int64_t));
}

void DataStream::write(std::uint64_t value) {
    char type = DataType::UINT64;
    write((char*)&type ,sizeof(char));
    if(byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char*)&value;
        char * last = first + sizeof(uint64_t);
        std::reverse(first, last);
    }
    write((char*)&value, sizeof(uint64_t));
}

void DataStream::write(float value) {
    char type = DataType::FLOAT;
    write((char *)&type, sizeof(char));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(float);
        std::reverse(first, last);
    }
    write((char *)&value, sizeof(float));
}

void DataStream::write(double value) {
    char type = DataType::DOUBLE;
    write((char *)&type, sizeof(char));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(double);
        std::reverse(first, last);
    }
    write((char *)&value, sizeof(double));
}

void DataStream::write(const char* value) {
    char type = DataType::STRING;
    write((char *)&type, sizeof(char));
    int len = strlen(value);
    write(len);
    write(value, len);;
}

void DataStream::write(const std::string& value) {
    char type = DataType::STRING;
    write((char *)&type, sizeof(char));
    int len = value.size();
    write(len);
    write(value.data(), len);
}

void DataStream::write(const Serializable& value) {
    value.serialize(*this);
}

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

    uint64_t start = timer::Time::now().to_microseconds();

    /* 转换为char类型写入数据 */
    const T* ptr = value.data();
    const char* char_ptr = reinterpret_cast<const char*>(ptr);
    write(char_ptr, value.size());

    // 记录结束时间
    uint64_t end = timer::Time::now().to_microseconds();

    // 计算耗时（以微秒为单位）
    uint64_t elapsed  = end - start;
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

void DataStream::write_args() {}


bool DataStream::read(char* data, int len) {
    if (len < 0 || data == nullptr) {
        return false;
    }
    if (len == 0) {
        return true;
    }

    std::memcpy(data, (char *)&buf_[pos_], len);
    pos_ += len;
    return true;
}

bool DataStream::read(bool & value)
{
    if (buf_[pos_] != DataType::BOOL)
    {
        return false;
    }
    ++pos_;
    value = buf_[pos_];
    ++pos_;
    return true;
}

bool DataStream::read(char& value) {
    if (buf_[pos_] != DataType::CHAR)  // 此次有疑问？
    {
        return false;
    }
    ++pos_;
    value = buf_[pos_];
    ++pos_;
    return true;
}

bool DataStream::read(std::int32_t& value) {
    if (buf_[pos_] != DataType::INT32)
    {
        return false;
    }
    ++pos_;
    value = *((int32_t *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    pos_ += 4;
    return true;
}

bool DataStream::read(std::uint32_t& value) {
    if (buf_[pos_] != DataType::UINT32)
    {
        return false;
    }
    ++pos_;
    value = *((uint32_t *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(uint32_t);
        std::reverse(first, last);
    }
    pos_ += 4;
    return true;
}

bool DataStream::read(std::int64_t& value) {
    if (buf_[pos_] != DataType::INT64)
    {
        return false;
    }
    ++pos_;
    value = *((int64_t *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    pos_ += 8;
    return true;
}

bool DataStream::read(std::uint64_t& value) {
    if (buf_[pos_] != DataType::UINT64)
    {
        return false;
    }
    ++pos_;
    value = *((uint64_t *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(uint64_t);
        std::reverse(first, last);
    }
    pos_ += 8;
    return true;
}

bool DataStream::read(float& value) {
    if (buf_[pos_] != DataType::FLOAT)
    {
        return false;
    }
    ++pos_;
    value = *((float *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(float);
        std::reverse(first, last);
    }
    pos_ += 4;
    return true;
}

bool DataStream::read(double& value) {
    if (buf_[pos_] != DataType::DOUBLE)
    {
        return false;
    }
    ++pos_;
    value = *((double *)(&buf_[pos_]));
    if (byteorder_ == ByteOrder::BigEndian)
    {
        char * first = (char *)&value;
        char * last = first + sizeof(double);
        std::reverse(first, last);
    }
    pos_ += 8;
    return true;
}

bool DataStream::read(std::string& value) {
    if (buf_[pos_] != DataType::STRING)
    {
        return false;
    }
    ++pos_;
    int len;
    read(len);
    if (len < 0)
    {
        return false;
    }
    value.assign((char *)&(buf_[pos_]), len);
    pos_ += len;
    return true;
}

bool DataStream::read(Serializable& value) {
    return value.unserialize(*this);
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

bool DataStream::read_args() {
    return true;
}


const char* DataStream::data() const {
    return buf_.data();
}

std::size_t DataStream::size() const {
    return buf_.size();
}


void DataStream::clear() {
    buf_.clear();
    pos_ = 0;
}

void DataStream::reset() {
    pos_ = 0;
}

bool DataStream::save(const std::string& filename) const {
    std::ofstream fout(filename, std::ios::binary);
    if (!fout) {
        return false;
    }
    fout.write(data(), static_cast<std::streamsize>(size()));
    fout.flush();
    fout.close();
    return static_cast<bool>(fout);
}

bool DataStream::load(const std::string& filename) {
    std::ifstream fin(filename, std::ios::binary);
    if (!fin) {
        return false;
    }

    std::stringstream ss;
    ss << fin.rdbuf();
    const std::string content = ss.str();
    clear();
    reserve(content.size());
    write(content.data(), content.size());
    return true;
}

DataStream& DataStream::operator<<(bool value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(char value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(std::int32_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(std::uint32_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(std::int64_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(std::uint64_t value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(float value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(double value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(const char* value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(const std::string& value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator<<(const Serializable& value) {
    write(value);
    return *this;
}

DataStream& DataStream::operator>>(bool& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(char& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(std::int32_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(std::uint32_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(std::int64_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(std::uint64_t& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(float& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(double& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(std::string& value) {
    read(value);
    return *this;
}

DataStream& DataStream::operator>>(Serializable& value) {
    read(value);
    return *this;
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
