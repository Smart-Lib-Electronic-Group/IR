#include "thread.hpp"
#include "signal.hpp"

using namespace OwO::system;
using namespace kernel;

O_METAOBJECT_NOSUPER(Object);

Object* Object::find_child(const std::string& name, bool recursion)
{
  Mutex_Guard locker(m_mutex);
  auto        it = m_children.begin();
  while (it != m_children.end())
  {
    if ((*it)->m_name == name)
      return *it;
    ++it;
  }

  if (true == recursion)
  {
    Object* ret = nullptr;
    it          = m_children.begin();
    while (it != m_children.end())
    {
      ret = (*it)->find_child(name, recursion);
      if (nullptr != ret)
        return ret;
      ++it;
    }
  }

  return nullptr;
}

std::list<Object*> Object::find_children(const std::string& name, bool recursion)
{
  Mutex_Guard        locker(m_mutex);
  std::list<Object*> ret;

  auto it = m_children.begin();
  while (it != m_children.end())
  {
    if ((*it)->m_name == name)
      ret.push_back(*it);
    ++it;
  }

  if (true == recursion)
  {
    it = m_children.begin();
    while (it != m_children.end())
    {
      ret.splice(ret.end(), (*it)->find_children(name, recursion));
      ++it;
    }
  }

  return ret;
}

void Object::move_to_thread(Thread* thread)
{
  m_thread = thread;
}

Thread* Object::thread() const
{
  return m_thread;
}

void Object::post_later_delete()
{
  m_thread->m_message->send({ Thread::later_delete_message, nullptr, static_cast<void*>(this) });
}

void Object::post_slot_signal(Base_Signal* signal, void* message)
{
  m_thread->m_message->send({ Thread::slot_message, signal, message });
}

void Object::message_slot_handler(Base_Signal* signal, void* message)
{
  signal->m_execute(message);
}

void Object::message_later_delete_handler(void* message)
{
  delete static_cast<Object*>(message);
}

void Object::add_connection(Base_Signal* signal, void* conn)
{
  kernel::Mutex_Guard locker(m_mutex);
  m_signal_connections.push_back({ signal, conn });
}

void Object::delete_connection(Base_Signal* signal, void* conn)
{
  kernel::Mutex_Guard locker(m_mutex);
  auto                it = m_signal_connections.begin();
  while (it != m_signal_connections.end())
  {
    if ((*it).signal == signal && (*it).conn == conn)
    {
      m_signal_connections.erase(it);
      return;
    }
    ++it;
  }
}

void Object::disconnect()
{
  Mutex_Guard locker(m_mutex);
  auto        it = m_signal_connections.begin();
  while (it != m_signal_connections.end())
  {
    (*it).signal->m_disconnect((*it).conn);
    ++it;
  }
  m_signal_connections.clear();
}

Object::Object(const std::string& name, Object* parent)
{
  {
    Atomic_Guard atomic;
    m_is_alive = true;
  }
  m_name   = name;
  m_thread = Thread::current_thread();
  set_parent(parent);

  if (parent)
  {
    if (true == parent->inherits("Thread"))
      m_thread = dynamic_cast<Thread*>(parent);
  }
}
