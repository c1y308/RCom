
#include "dispatcher.hpp"


namespace transport {

Dispatcher::Dispatcher() : is_shutdown_(false) {}

Dispatcher::~Dispatcher() { shutdown(); }


void Dispatcher::shutdown() {
  is_shutdown_.store(true);
  std::cout << "Shutdown" << std::endl;
}

bool Dispatcher::has_channel(uint64_t channel_id) {
  return msg_listeners_.Has(channel_id);
}

}
