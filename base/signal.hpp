
#ifndef CMW_BASE_SIGNAL_H_
#define CMW_BASE_SIGNAL_H_

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <mutex>



namespace base {

template <typename... Types>
class Slot;

template <typename... Types>
class Connection;


/*槽（观察者）:
  保存了一个回调函数std::function<void(Types...)> cb_和一个标记 bool connected_，
  提供一个 disconnect 函数将标记置为 false。
  重载()操作符，当被调用时就会去运行cb_函数。
*/
template <typename... Types>
class Slot {
 public:
  using callback = std::function<void(Types...)>;  // 回调函数为 void (Types...) 类型的函数

  Slot(const Slot& another)
      : cb_(another.cb_), connected_(another.connected_) {}

  explicit Slot(const callback& cb, bool connected = true)
      : cb_(cb), connected_(connected) {}

  virtual ~Slot() {}

  void operator()(Types... args) {
    if (connected_ && cb_) {
      cb_(args...);
    }
  }

  void disconnect() { connected_ = false; }
  bool is_connected() const { return connected_; }

 private:
  callback cb_;
  bool connected_ = true;
};


/*信号（被观察者）
  slot_list 成员变量：一个链表记录关联在该信号下的所有槽
*/
template <typename... Types>  // 模板参数为回调函数的参数类型(也就是说信号创建时的模板参数类型就是在确定回调函数的参数类型)
class Signal {
 public:
  using callback = std::function<void(Types...)>;
  using slot_ptr = std::shared_ptr<Slot<Types...>>;
  using slot_list = std::list<slot_ptr>;
  using connection_type = Connection<Types...>;

  Signal() {}
  virtual ~Signal() { disconnect_all_slots(); }

  //重载()操作符（相当于给槽的回调函数传入具体参数了！），调用时会调用关联槽的回调函数
  void operator()(Types... args) {
    slot_list local;  // 指针链表（指针为 std::shared_ptr<Slot<Types...>>，Slot 为槽类，内部有对应的回调函数）
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto& slot : slots_) {
        local.emplace_back(slot);
      }
    }

    if (!local.empty()) {
      for (auto& slot : local) {
        (*slot)(args...);  // 调用回调函数
      }
    }
    clear_disconnected_slots();
  }

  //为传入的回调函数的创建一个 Slot 共享指针，然后加入到信号的槽列表并返回一个 Connection 关联实例
  connection_type connect(const callback& cb) {
    auto slot = std::make_shared<Slot<Types...>>(cb);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      slots_.emplace_back(slot);
    }

    return connection_type(slot, this);
  }

  //接收一个 connection 参数，从槽列表中找到该槽，然后将槽的标记置为 false 并从列表中删除。
  bool disconnect(const connection_type& conn) {
    bool find = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto& slot : slots_) {
        if (conn.matches_slot(slot)) {
          find = true;
          slot->disconnect();
        }
      }
    }

    if (find) {
      clear_disconnected_slots();
    }
    return find;
  }

  void disconnect_all_slots() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& slot : slots_) {
      slot->disconnect();
    }
    slots_.clear();
  }

 private:
  Signal(const Signal&) = delete;
  Signal& operator=(const Signal&) = delete;

  void clear_disconnected_slots() {
    std::lock_guard<std::mutex> lock(mutex_);
    slots_.erase(
        std::remove_if(slots_.begin(), slots_.end(),
                       [](const slot_ptr& slot) { return !slot->is_connected(); }),
        slots_.end());
  }

  slot_list slots_;
  std::mutex mutex_;
};


/**
 * 保存了一个信号的指针一个槽的指针，
 * 一个Connection实例就代表了一条关联关系。
 * 通过Slot的标记位显示是否处于关联状态。
*/
template <typename... Types>
class Connection {
 public:
  using slot_ptr = std::shared_ptr<Slot<Types...>>;
  using signal_ptr = Signal<Types...>*;

  Connection() : slot_(nullptr), signal_(nullptr) {}
  Connection(const slot_ptr& slot, const signal_ptr& signal)
      : slot_(slot), signal_(signal) {}
  virtual ~Connection() {
    slot_ = nullptr;
    signal_ = nullptr;
  }

  Connection& operator=(const Connection& another) {
    if (this != &another) {
      this->slot_ = another.slot_;
      this->signal_ = another.signal_;
    }
    return *this;
  }

  bool matches_slot(const slot_ptr& slot) const {
    if (slot != nullptr && slot_ != nullptr) {
      return slot_.get() == slot.get();
    }
    return false;
  }

  bool is_connected() const {
    if (slot_) {
      return slot_->is_connected();
    }
    return false;
  }

  bool disconnect() {
    if (signal_ && slot_) {
      return signal_->disconnect(*this);
    }
    return false;
  }

 private:
  slot_ptr slot_;
  signal_ptr signal_;
};


}


#endif
