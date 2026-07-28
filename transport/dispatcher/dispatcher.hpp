
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
    virtual ~Dispatcher();

    virtual void shutdown();

    template <typename MessageT>
    void add_listener(const RoleAttributes& self_attr,
                      const MessageListener<MessageT>& listener);

    template <typename MessageT>
    void add_listener(const RoleAttributes& self_attr,
                      const RoleAttributes& opposite_attr,
                      const MessageListener<MessageT>& listener);


    template <typename MessageT>
    void remove_listener(const RoleAttributes& self_attr);

    template <typename MessageT>
    void remove_listener(const RoleAttributes& self_attr,
                         const RoleAttributes& opposite_attr);

    bool has_channel(uint64_t channel_id);

protected:
    std::atomic<bool> is_shutdown_;

    //保存回调函数的哈希表，key 为 channel_id，value 为此channel对应的 ListenerHandler
    AtomicHashMap<uint64_t, listener_handler_base_ptr> msg_listeners_;

    base::AtomicRWLock rw_lock_;

};


template <typename MessageT>
void Dispatcher::add_listener(const RoleAttributes& self_attr,
                              const MessageListener<MessageT>& listener){
    if(is_shutdown_.load()){
        return ;
    }
    //拿到channel_id
    uint64_t channel_id = self_attr.channel_id;
    //创建一个新的ListenerHandler(回调函数)
    std::shared_ptr<ListenerHandler<MessageT>> handler;

    listener_handler_base_ptr* handler_base = nullptr;
    
    //如果此 channel_id 已经有 ListenerHandler 了
    if(msg_listeners_.get(channel_id, &handler_base)){
      //取出此 channel_id 对应的 ListenerHandler
      handler = std::dynamic_pointer_cast<ListenerHandler<MessageT>>(*handler_base);
      if (handler == nullptr) {
        AERROR  <<  "please ensure that readers with the same channel["
                << self_attr.channel_name
                << "] in the same process have the same message type";
        return;
      }
    } else{
        // 此 channel_id 没有对应的 ListenerHandler
        ADEBUG  << "new reader for channel:"
                << GlobalData::get_channel_by_id(channel_id);
        //新建一个 ListenerHandler
        handler.reset(new ListenerHandler<MessageT>());
        //建立channel_id 与 ListenerHandler的对应关系，保存到 msg_listeners_ 中
        msg_listeners_.set(channel_id, handler);
    }
    //为此 ListenerHandler 连接槽函数，一个 id 可以绑定多个槽函数
    handler->connect(self_attr.id, listener);

}

template <typename MessageT>
void Dispatcher::add_listener(const RoleAttributes& self_attr,
                              const RoleAttributes& opposite_attr,
                              const MessageListener<MessageT>& listener)
{
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = self_attr.channel_id;
  std::shared_ptr<ListenerHandler<MessageT>> handler;

  listener_handler_base_ptr* handler_base = nullptr;
  if(msg_listeners_.get(channel_id, &handler_base)){
        handler = std::dynamic_pointer_cast<ListenerHandler<MessageT>>(*handler_base);
    if (handler == nullptr) {
      std::cout << "please ensure that readers with the same channel["
                << self_attr.channel_name
                << "] in the same process have the same message type"<< std::endl;
      return;
    }
  } else {
        std::cout << "new reader for channel:"
           << GlobalData::get_channel_by_id(channel_id);
        handler.reset(new ListenerHandler<MessageT>());
        msg_listeners_.set(channel_id, handler);
  }

  handler->connect(self_attr.id, opposite_attr.id, listener);
}


template <typename MessageT>
void Dispatcher::remove_listener(const RoleAttributes& self_attr) {
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = self_attr.channel_id;

  listener_handler_base_ptr* handler_base = nullptr;
  if (msg_listeners_.get(channel_id, &handler_base)) {
    (*handler_base)->disconnect(self_attr.id);
  }
}


template <typename MessageT>
void Dispatcher::remove_listener(const RoleAttributes& self_attr,
                                const RoleAttributes& opposite_attr) {
  if (is_shutdown_.load()) {
    return;
  }
  uint64_t channel_id = self_attr.channel_id;

  listener_handler_base_ptr* handler_base = nullptr;
  if (msg_listeners_.get(channel_id, &handler_base)) {
    (*handler_base)->disconnect(self_attr.id, opposite_attr.id);
  }
}



}


#endif