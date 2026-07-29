#ifndef TRANSPORT_DISPATCHER_SHM_DISPATCHER_H_
#define TRANSPORT_DISPATCHER_SHM_DISPATCHER_H_
#include "dispatcher.hpp"
#include "../../common/declare_singleton.hpp"
namespace transport {

class ShmDispatcher : public Dispatcher {
public:
    ~ShmDispatcher();

    void shutdown() override;

    template <typename MessageT>
    void add_listener(const RoleAttributes& reader_attr,
                      const MessageListener<MessageT>& callback);

    template <typename MessageT>
    void add_listener(const RoleAttributes& reader_attr,
                      const RoleAttributes& writer_attr,
                      const MessageListener<MessageT>& callback);

private:
    DECLARE_SINGLETON(ShmDispatcher)

};


}

#endif  // TRANSPORT_DISPATCHER_SHM_DISPATCHER_H_