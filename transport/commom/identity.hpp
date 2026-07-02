#ifndef TRANSPORT_COMMON_IDENTITY_H
#define TRANSPORT_COMMON_IDENTITY_H
#include <cstdint>
#include <string>
namespace transport{

constexpr uint8_t ID_SIZE = 8;

class Identity{
public:
    explicit Identity(bool need_gengerate = true);
    Identity(const Identity &other);
    Identity& operator=(const Identity &other);
    ~Identity();

    bool operator==(const Identity& other) const;

    /* 标识符相关 */
    std::string to_string() const;
    std::size_t length() const;

    /* 哈希值相关 */
    uint64_t    hash_value() const;
    const char* data() const {return data_;}
    void        set_data(const char* data);


private:
    void update_hash_value();
    char data_[ID_SIZE];   // 标识符
    uint64_t hash_value_;  // 哈希值

};


}

#endif
