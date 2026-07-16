#ifndef __TRANSPORT_MESSAGE_INFO_HPP__
#define __TRANSPORT_MESSAGE_INFO_HPP__

#include "identity.hpp"

namespace transport{

/**
 * @brief  MessageInfo 是除开要发送的数据外的额外信息，可以理解为一帧数据的标识
 */
class MessageInfo {

public:
    MessageInfo();
    MessageInfo(const Identity& sender_id, uint64_t seq_num);
    MessageInfo(const Identity& sender_id, uint64_t seq_num, const Identity& spare_id);

    //  拷贝构造函数
    MessageInfo(const MessageInfo& another);
    MessageInfo& operator=(const MessageInfo& another);
    virtual ~MessageInfo();

    bool operator==(const MessageInfo& another) const;
    bool operator!=(const MessageInfo& another) const;

    //序列化和反序列化操作
    bool serialize_to(std::string* dst) const;
    bool serialize_to(char* dst, std::size_t len) const;
    bool deserialize_from(const std::string& src);
    bool deserialize_from(const char* src, std::size_t len);

    // getter and setter(id, channel id, spare id)
    inline const Identity& sender_id() const { return sender_id_; }
    inline void set_sender_id(const Identity& sender_id) { sender_id_ = sender_id; }

    inline uint64_t channel_id() const { return channel_id_; }
    inline void     set_channel_id(uint64_t channel_id) { channel_id_ = channel_id; }

    inline uint64_t seq_num() const { return seq_num_; }
    inline void set_seq_num(uint64_t seq_num) { seq_num_ = seq_num; }

    inline const Identity& spare_id() const { return spare_id_; }
    inline void set_spare_id(const Identity& spare_id) { spare_id_ = spare_id; }

    static const std::size_t kSize;
private:
    Identity sender_id_;        // 发送者ID
    Identity spare_id_;         // 备用发送者 ID
    uint64_t channel_id_ = 0;   // 通道 ID
    uint64_t seq_num_ = 0;

};


}  // namespace transport

#endif
