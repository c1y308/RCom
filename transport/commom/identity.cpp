#include "identity.hpp"
#include "../../common/str_hash.hpp"
#include<uuid/uuid.h>
#include <cstring>
namespace transport{

/* 构造函数 */
Identity::Identity(bool need_generate) : hash_value_(0) {
    std::memset(data_, 0, ID_SIZE);

    if(need_generate){
        uuid_t uuid;
        uuid_generate(uuid);
        std::memcpy(data_, uuid, ID_SIZE);
        update_hash_value();
    }
}

/* 拷贝构造函数 */
Identity::Identity(const Identity &other){
    std::memcpy(data_, other.data_, ID_SIZE);
    hash_value_ = other.hash_value_;
}

/* 拷贝赋值函数 */
Identity& Identity::operator=(const Identity& other) {
  if (this != &other) {
    std::memcpy(data_, other.data_, ID_SIZE);
    hash_value_ = other.hash_value_;
  }
  return *this;
}

Identity::~Identity() {}

/* 依据标识符进行比较 */
bool Identity::operator==(const Identity &other) const{
    return std::memcmp(data_, other.data_, ID_SIZE) == 0;
}

/* 更新哈希值 */
void Identity::update_hash_value(){
    hash_value_ = common::str_hash(std::string(data_, ID_SIZE));
}

/* 获取原始哈希值 */
uint64_t Identity::hash_value() const { return hash_value_; }

/* 获取 string 格式的哈希值 */
std::string Identity::to_string() const { return std::to_string(hash_value_); }


/* 设置标识符 */
void Identity::set_data(const char* data) {
    if (data == nullptr) {
    return;
    }
    std::memcpy(data_, data, sizeof(data_));
    update_hash_value();
}

/* 获取标识符长度 */
size_t Identity::length() const { return ID_SIZE; }



}
