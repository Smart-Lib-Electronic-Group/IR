#ifndef __SIGNAL_HPP__
#define __SIGNAL_HPP__

#include <forward_list>
#include <tuple>
#include <type_traits>

#include "thread.hpp"
#include "mutex.hpp"
#include "object.hpp"

namespace OwO
{
namespace system
{
namespace kernel
{
class Thread;
}

class Base_Signal
{
  O_MEMORY
private:
  friend class Object;

protected:
  virtual void m_disconnect(void* conn) {}
  virtual void m_execute(void* conn) {}
};

template <typename... Args>
class Signal : public Base_Signal
{
  O_MEMORY
private:
  friend class Object;

  struct connection
  {
    O_MEMORY
    Object* receiver;
    void (Object::*slot)(Args...);
    void (*g_slot)(Args...);
    Connection_Type     type;
    std::tuple<Args...> args;
  };

  std::forward_list<connection*> m_connections;
  kernel::Mutex                  m_mutex;

protected:
  void m_disconnect(void* conn) override
  {
    connection* it = static_cast<connection*>(conn);
    m_connections.remove(it);
    delete it;
  }

  void m_execute(void* conn) override
  {
    connection* it = static_cast<connection*>(conn);
    if (it->receiver)
    {
      if (it->receiver->is_alive() == true)
        std::apply(
          [it](auto&&... args)
          {
            (it->receiver->*(it->slot))(args...);
          },
          it->args);
    }
    else
    {
      std::apply(
        [it](auto&&... args)
        {
          (*it->g_slot)(args...);
        },
        it->args);
    }
  }

  void m_delete_connection(connection* conn)
  {
    kernel::Mutex_Guard locker(m_mutex);
    if (conn->receiver)
      if (conn->receiver->is_alive() == true)
        conn->receiver->delete_connection(dynamic_cast<Base_Signal*>(this), static_cast<void*>(conn));
    delete conn;
  }

  void m_dispatch(connection* conn, Args... args)
  {
    conn->args = std::make_tuple(args...);
    if (conn->receiver)
    {
      kernel::Thread* thread = conn->receiver->thread();
      if (conn->type == Connection_Type::Connection_Auto)
      {
        if (thread == kernel::Thread::current_thread())
          m_execute(static_cast<void*>(conn));
        else
          thread->post_slot_signal(dynamic_cast<Base_Signal*>(this), static_cast<void*>(conn));
      }
      else if (conn->type == Connection_Type::Connection_Direct)
        m_execute(static_cast<void*>(conn));
      else if (conn->type == Connection_Type::Connection_Queued)
        thread->post_slot_signal(dynamic_cast<Base_Signal*>(this), static_cast<void*>(conn));
    }
    else
      m_execute(static_cast<void*>(conn));
  }

public:
  Signal() {}

  bool connect(void (*slot)(Args...))
  {
    kernel::Mutex_Guard locker(m_mutex);

    auto it = m_connections.begin();
    while (it != m_connections.end())
    {
      if ((*it)->g_slot == slot)
        return false;
      ++it;
    }

    connection* conn = new connection { nullptr, nullptr, slot, Connection_Auto };
    m_connections.push_front(conn);

    return true;
  }

  template <typename T>
  bool connect(T* receiver, void (T::*slot)(Args...), Connection_Type type = Connection_Auto)
  {
    static_assert(std::is_base_of<Object, T>::value, "Receiver must be a subclass of Object.");

    kernel::Mutex_Guard locker(m_mutex);

    auto it = m_connections.begin();
    while (it != m_connections.end())
    {
      if ((*it)->receiver == receiver && (*it)->slot == reinterpret_cast<void (Object::*)(Args...)>(slot))
        return false;
      ++it;
    }

    connection* conn = new connection { dynamic_cast<Object*>(receiver), reinterpret_cast<void (Object::*)(Args...)>(slot), nullptr, type };
    m_connections.push_front(conn);

    receiver->add_connection(dynamic_cast<Base_Signal*>(this), static_cast<void*>(conn));
    return true;
  }

  bool disconnect(void (*slot)(Args...))
  {
    if (m_connections.empty())
      return true;

    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_connections.begin();
    while (it != m_connections.end())
    {
      if ((*it)->g_slot == slot)
      {
        m_connections.remove(*it);
        m_delete_connection(*it);
        return true;
      }
      ++it;
    }

    return false;
  }

  template <typename T>
  bool disconnect(T* receiver, void (T::*slot)(Args...))
  {
    static_assert(std::is_base_of<Object, T>::value, "Receiver must be a subclass of Object.");
    static_assert(nullptr == slot, "Slot must not be null.");

    if (m_connections.empty())
      return true;

    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_connections.begin();
    while (it != m_connections.end())
    {
      if ((*it)->receiver == receiver && (*it)->slot == slot)
      {
        m_connections.remove(*it);
        m_delete_connection(*it);
        return true;
      }
      ++it;
    }

    return false;
  }

  template <typename T>
  void disconnect(T* receiver)
  {
    static_assert(std::is_base_of<Object, T>::value, "Receiver must be a subclass of Object.");

    if (m_connections.empty())
      return;

    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_connections.begin();
    while (it != m_connections.end())
    {
      if ((*it)->receiver == receiver)
      {
        m_connections.remove(*it);
        m_delete_connection(*it);
      }
      ++it;
    }
  }

  void emit(Args... args)
  {
    if (m_connections.empty())
      return;

    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_connections.begin();
    while (it != m_connections.end())
    {
      m_dispatch(*it, args...);
      ++it;
    }
  }

  void operator()(Args... args)
  {
    emit(args...);
  }

  virtual ~Signal()
  {
    if (m_connections.empty())
      return;

    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_connections.begin();
    while (it != m_connections.end())
    {
      m_delete_connection(*it);
      ++it;
    }
    m_connections.clear();
  }
};

} /* namespace system */
} /* namespace OwO */

#define signals public
#define slots   public

#endif /* __SIGNAL_HPP__ */