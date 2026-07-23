#ifndef __MESSAGE_LISTENER_HANDLER_HPP__
#define __MESSAGE_LISTENER_HANDLER_HPP__

#include "atomic_rw_lock.hpp"
#include "rw_lock_guard.hpp"
#include "message_info.hpp"
#include "signal.hpp"
#include "log.hpp"
#include <memory>
#include <unordered_map>
#include <functional>
namespace transport{

using base::AtomicRWLock;    

/**
 * @brief  reader_id 是读者的 Endpoint id
 * @brief  writer_id 是写者的 Endpoint id
 */

/* 提供一个基类，方便使用一个基类指针指向各种子类 */
class ListenerHandlerBase{
public:
    ListenerHandlerBase() {}
    virtual ~ListenerHandlerBase() {}

    virtual void disconnect(uint64_t reader_id) = 0;
    virtual void disconnect(uint64_t reader_id, uint64_t writer_id) = 0;
   // inline bool IsRawMessage() const { return is_raw_message_; }
    virtual void run_from_string(const std::string& str, const MessageInfo& msg_info) = 0;
protected:
    bool is_raw_message_ = false;
};


template <typename MessageT>
class ListenerHandler : public ListenerHandlerBase{
public:
    using message_ptr = std::shared_ptr<MessageT>;  //一个MessageT的指针

    using message_signal = base::Signal<const message_ptr&, const MessageInfo&>;

    // 回调函数 listener 的定义：回调函数传入参数为 message_ptr 和一个 MessageInfo 的引用
    using listener = std::function<void(const message_ptr&, const MessageInfo&)>;
    
    // 依据 message_ptr 的引用 和 MessageInfo 的引用指定 Connection 的模板实例化
    using message_connection = base::Connection<const message_ptr&, const MessageInfo&>;
    // 记录每个 reader_id 对应的 message_connection
    using connection_map = std::unordered_map<uint64_t, message_connection>;

    ListenerHandler() {}
    virtual ~ListenerHandler() {}

    /* 广播模式和点对点模式 */
    void connect(uint64_t reader_id, const listener& listener);
    void connect(uint64_t reader_id, uint64_t writer_id, const listener& listener);

    void disconnect(uint64_t reader_id) override;
    void disconnect(uint64_t reader_id, uint64_t writer_id) override;

    void run(const message_ptr& msg, const MessageInfo& msg_info);
    void run_from_string(const std::string& str,
                     const MessageInfo& msg_info) override;

private:
    using signal_ptr = std::shared_ptr<message_signal>;
    using message_signal_map = std::unordered_map<uint64_t, signal_ptr>;


    message_signal signal_;       // 广播信号
    connection_map signal_conns_; // key:reader_id （记录"谁"订阅了广播信号，并记录它的connection）

    message_signal_map writer_signals_;  // key: writer_id（为每个 Writer 单独创建一个信号）
    //  保存 writer - reader - connection 的映射关系（一对多采用 std::unordered_map 数据结构）
    std::unordered_map<uint64_t, connection_map> writer_signals_conns_;

    base::AtomicRWLock rw_lock_;
};


/* 广播模式：把 reader 的回调函数挂到广播信号上，任何消息来了都会触发。 */
template <typename MessageT>
void ListenerHandler<MessageT>::connect(uint64_t reader_id, const listener& listener){
    auto connection = signal_.connect(listener);  // 为signal_连接一个槽函数
    if(!connection.is_connected())
    {
        return;
    }

    // 加锁
    base::WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    // 同一进程下的多个 reader 有可能监听同一channel，但是他们的id是唯一的
    signal_conns_[reader_id] = connection;
}


/* 点对点模式：为特定的 writer 创建信号，只有该 writer 发送的消息才会触发对应的回调。 */
template <typename MessageT>
void ListenerHandler<MessageT>::connect(uint64_t reader_id, uint64_t writer_id, const listener& listener){

    base::WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if(writer_signals_.find(writer_id) == writer_signals_.end())
    {
        //为这个 writer 新建一个 message_signal
        writer_signals_[writer_id] = std::make_shared<message_signal>();
    }
    //为这个信号添加 listener（槽）
    auto connection = writer_signals_[writer_id]->connect(listener);
    if(!connection.is_connected()){
        std::cout << writer_id << " " << reader_id << " connect failed!" << std::endl;
    }

    // 一个writer可能对应着很多个reader，所以对于同一个writer的多个reader用一张map保存起来
    if(writer_signals_conns_.find(writer_id) == writer_signals_conns_.end())
    {
        writer_signals_conns_[writer_id] = connection_map();
    }

    writer_signals_conns_[writer_id][reader_id] = connection;
}

template <typename MessageT>
void ListenerHandler<MessageT>::disconnect(uint64_t reader_id){
    base::WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if (signal_conns_.find(reader_id) == signal_conns_.end()) {
        return;
    }

    signal_conns_[reader_id].disconnect();
    signal_conns_.erase(reader_id);
}

template <typename MessageT>
void ListenerHandler<MessageT>::disconnect(uint64_t reader_id, uint64_t writer_id){
 
    base::WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if (writer_signals_conns_.find(writer_id) == writer_signals_conns_.end()) {
        return;
    }

    if (writer_signals_conns_[writer_id].find(reader_id) == writer_signals_conns_[writer_id].end()) {
        return;
    }

    writer_signals_conns_[writer_id][reader_id].disconnect();
    writer_signals_conns_[writer_id].erase(reader_id);


}


template <typename MessageT>
void ListenerHandler<MessageT>::run(const message_ptr& msg,
                                    const MessageInfo& msg_info) {
    // 无论如何，先触发广播信号 -- 所有“不管谁发的都要”的 reader 都会收到                                
    signal_(msg, msg_info);

    // 然后看发送者是谁
    uint64_t writer_id = msg_info.sender_id().hash_value();    
    base::ReadLockGuard<AtomicRWLock> lock(rw_lock_);   
    if (writer_signals_.find(writer_id) == writer_signals_.end()) {
        return;
    }  
    // 如果这个发送者有专属信号，触发它 —— "点名要这个Writer"的 Reader 才会收到
    (*writer_signals_[writer_id])(msg, msg_info);        

}

template <typename MessageT>
void ListenerHandler<MessageT>::run_from_string(const std::string& str,
                                              const MessageInfo& msg_info) {
  // auto msg = std::make_shared<MessageT>();
  // serialize::DataStream ds(str);
  // ds >> *msg;

  // run(msg,msg_info);

  AERROR << "run_from_string Error";

}


}  // namespace transport




#endif
