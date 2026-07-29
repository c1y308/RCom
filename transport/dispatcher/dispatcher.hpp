
#ifndef TRANSPORT_DISPATCHER_DISPATCHER_H_
#define TRANSPORT_DISPATCHER_DISPATCHER_H_

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "role_attributes.hpp"
#include "atomic_hash_map.hpp"
#include "atomic_rw_lock.hpp"
#include "message_info.hpp"
#include "listener_handler.hpp"
#include "global_data.hpp"
#include "log.hpp"

namespace transport {

using namespace common;
using base::AtomicHashMap;

using namespace config;
class Dispatcher;
using DispatcherPtr = std::shared_ptr<Dispatcher>;

/* listener回调函数*/
template <typename MessageT>
using MessageListener = std::function<void(const std::shared_ptr<MessageT>&, const MessageInfo&)>;


class Dispatcher {

public:
    Dispatcher();
    virtual ~Dispatcher();  // 作为基类，析构函数声明为 virtual

    virtual void shutdown();

    /* 广播监听注册接口 */
    template <typename MessageT>
    void add_listener(const RoleAttributes& reader_attr,
                      const MessageListener<MessageT>& callback);
    
    /* 点对点监听注册接口 */
    template <typename MessageT>
    void add_listener(const RoleAttributes& reader_attr,
                      const RoleAttributes& writer_attr,
                      const MessageListener<MessageT>& callback);

    /* 移除广播监听接口 */
    template <typename MessageT>
    void remove_listener(const RoleAttributes& reader_attr);
    
    /* 移除点对点监听接口 */
    template <typename MessageT>
    void remove_listener(const RoleAttributes& reader_attr,
                         const RoleAttributes& writer_attr);

    bool has_channel(uint64_t channel_id);

protected:
    std::atomic<bool> is_shutdown_;

    //保存每个 channel_id(key) 对应的 ListenerHandler(value)
    AtomicHashMap<uint64_t, listener_handler_base_ptr> channel_handlers_;

    base::AtomicRWLock handlers_lock_;

};


template <typename MessageT>  // MessageT是消息类型
void Dispatcher::add_listener(const RoleAttributes& reader_attr,
                              const MessageListener<MessageT>& callback){
    if(is_shutdown_.load()){
        return;
    }
    //拿到 reader 的 channel_id, 也就找到了对应的 channel
    uint64_t channel_id = reader_attr.channel_id;

    std::shared_ptr<ListenerHandler<MessageT>> typed_handler;
    listener_handler_base_ptr* base_handler = nullptr;
    
    //如果此 channel 已经有 ListenerHandler 了
    if(channel_handlers_.get(channel_id, &base_handler)){
        //取出此 channel_id 对应的 ListenerHandler
        typed_handler = std::dynamic_pointer_cast<ListenerHandler<MessageT>>(*base_handler);
        if (typed_handler == nullptr) {
          AERROR  <<  "please ensure that readers with the same channel["
                  << reader_attr.channel_name
                  << "] in the same process have the same message type";
          return;
        }
    } else{
        // 此 channel_id 没有对应的 ListenerHandler
        std::cout << "new reader for channel:"
                  << GlobalData::get_channel_by_id(channel_id);
        //新建一个 ListenerHandler
        typed_handler.reset(new ListenerHandler<MessageT>());
        //建立channel 与 ListenerHandler 的对应关系，保存到 channel_handlers_ 中
        channel_handlers_.set(channel_id, typed_handler);
    }
    //为此channel 的 ListenerHandler 连接回调函数，一个 id 可以绑定多个回调函数
    typed_handler->connect(reader_attr.id, callback);

}

template <typename MessageT>
void Dispatcher::add_listener(const RoleAttributes& reader_attr,
                              const RoleAttributes& writer_attr,
                              const MessageListener<MessageT>& callback)
{
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = reader_attr.channel_id;
  std::shared_ptr<ListenerHandler<MessageT>> typed_handler;

  listener_handler_base_ptr* base_handler = nullptr;
  if(channel_handlers_.get(channel_id, &base_handler)){
        typed_handler = std::dynamic_pointer_cast<ListenerHandler<MessageT>>(*base_handler);
    if (typed_handler == nullptr) {
      AERROR << "please ensure that readers with the same channel["
             << reader_attr.channel_name
             << "] in the same process have the same message type";
      return;
    }
  } else {
      std::cout << "new reader for channel:"
                << GlobalData::get_channel_by_id(channel_id);
      typed_handler.reset(new ListenerHandler<MessageT>());
      channel_handlers_.set(channel_id, typed_handler);
  }

  typed_handler->connect(reader_attr.id, writer_attr.id, callback);
}


template <typename MessageT>
void Dispatcher::remove_listener(const RoleAttributes& reader_attr) {
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = reader_attr.channel_id;

  listener_handler_base_ptr* base_handler = nullptr;
  if (channel_handlers_.get(channel_id, &base_handler)) {
    (*base_handler)->disconnect(reader_attr.id);
  }
}


template <typename MessageT>
void Dispatcher::remove_listener(const RoleAttributes& reader_attr,
                                const RoleAttributes& writer_attr) {
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = reader_attr.channel_id;

  listener_handler_base_ptr* base_handler = nullptr;
  if (channel_handlers_.get(channel_id, &base_handler)) {
    (*base_handler)->disconnect(reader_attr.id, writer_attr.id);
  }
}



}


#endif
