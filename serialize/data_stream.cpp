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

}  // namespace serialize
