#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <typeinfo>

#include "mutex.hpp"
#include "atomic.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
/// @brief 名称空间 系统接口
namespace kernel
{
class Thread;
}
class Base_Signal;

template <typename... Args>
class Signal;

enum Connection_Type
{
  Connection_Auto,
  Connection_Direct,
  Connection_Queued,
};

struct Meta_Object
{
  const char*        class_name;
  const Meta_Object* class_super;
  const uint32_t     class_size;
};

class Object
{
  O_MEMORY
private:
  mutable kernel::Mutex m_mutex;

  /* -------------------------------------- 元对象系统 -------------------------------------- */
public:
  static const Meta_Object static_meta_object;

  virtual const Meta_Object* meta_object() const
  {
    return &static_meta_object;
  };

  const char* class_name() const
  {
    return meta_object()->class_name;
  }

  const uint32_t class_size() const
  {
    return meta_object()->class_size;
  }

  bool inherits(const char* class_name) const
  {
    const Meta_Object* meta = meta_object();

    while (meta)
    {
      if (std::strcmp(meta->class_name, class_name) == 0)
        return true;
      meta = meta->class_super;
    }

    return false;
  }

  /* -------------------------------------- 动态属性系统 -------------------------------------- */

private:
  class Base_Property
  {
    O_MEMORY
  public:
    std::string                   m_name;
    virtual const std::type_info& type() const = 0;
    virtual ~Base_Property() {}
  };

  template <typename T>
  class Property : public Base_Property
  {
    O_MEMORY
  public:
    T m_value;

    explicit Property(const std::string& name, const T& value) : m_value(value)
    {
      m_name = name;
    }

    const std::type_info& type() const override
    {
      return typeid(T);
    }
  };

  std::vector<Base_Property*> m_properties;

  std::vector<Base_Property*>::iterator m_find_property(const std::string& name)
  {
    auto it = m_properties.begin();
    while (it != m_properties.end())
    {
      if ((*it)->m_name == name)
        return it;
      ++it;
    }
    return m_properties.end();
  }

  template <typename T>
  Property<T>* m_find_property(const std::string& name) const
  {
    auto it = m_properties.begin();
    while (it != m_properties.end())
    {
      if ((*it)->m_name == name && (*it)->type() == typeid(T))
        return static_cast<Property<T>*>(*it);
      ++it;
    }
    return nullptr;
  }

  void m_delete_properties()
  {
    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_properties.begin();
    while (it != m_properties.end())
    {
      delete *it;
      ++it;
    }
    m_properties.clear();
  }

public:
  template <typename T>
  void add_property(const std::string& name, const T& value)
  {
    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_find_property(name);
    if (it != m_properties.end())
    {
      if (auto prop = dynamic_cast<Property<T>*>(*it))
      {
        prop->m_value = value;
        return;
      }
      delete *it;
      *it = new Property<T>(name, value);
    }
    else
      m_properties.push_back(new Property<T>(name, value));
  }

  void add_property(const std::string& name, const char* value)
  {
    add_property<std::string>(name, value);
  }

  template <typename T>
  bool change_property(const std::string& name, const T& value)
  {
    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_find_property(name);
    if (it != m_properties.end())
    {
      if (auto prop = dynamic_cast<Property<T>*>(*it))
      {
        prop->m_value = value;
        return true;
      }
    }
    return false;
  }

  bool change_property(const std::string& name, const char* value)
  {
    return change_property<std::string>(name, value);
  }

  template <typename T>
  T property(const std::string& name, const T& defaultValue = T()) const
  {
    kernel::Mutex_Guard locker(m_mutex);
    if (auto prop = m_find_property<T>(name))
      return prop->m_value;
    else
      return defaultValue;
  }

  std::vector<std::string> properties() const
  {
    kernel::Mutex_Guard      locker(m_mutex);
    std::vector<std::string> names;
    auto                     it = m_properties.begin();
    while (it != m_properties.end())
      names.push_back((*it)->m_name);
    return names;
  }

  /* -------------------------------------- 线程管理 -------------------------------------- */

private:
  kernel::Thread* m_thread = nullptr;

public:
  void            move_to_thread(kernel::Thread* thread);
  kernel::Thread* thread() const;

protected:
  void post_later_delete();
  void post_slot_signal(Base_Signal* signal, void* message);
  void message_slot_handler(Base_Signal* signal, void* message);
  void message_later_delete_handler(void* message);

  /* -------------------------------------- 信号连接管理 -------------------------------------- */
private:
  template <typename... Args>
  friend class Signal;

  struct connection_info
  {
    O_MEMORY
    Base_Signal* signal;
    void*        conn;
  };

  std::vector<connection_info> m_signal_connections;

protected:
  void add_connection(Base_Signal* signal, void* conn);
  void delete_connection(Base_Signal* signal, void* conn);
  void disconnect();

  /* -------------------------------------- 对象树管理 -------------------------------------- */
private:
  bool m_is_alive;

protected:
  bool is_alive() const
  {
    return m_is_alive;
  }

private:
  Object*            m_parent = nullptr;
  std::string        m_name;
  std::list<Object*> m_children;

  void m_delete_children()
  {
    kernel::Mutex_Guard locker(m_mutex);
    auto                it = m_children.begin();
    while (it != m_children.end())
    {
      delete *it;
      ++it;
    }
    m_children.clear();
  }

public:
  const bool have_parent() const
  {
    return nullptr != m_parent;
  }

  const bool have_child() const
  {
    return 0 != m_children.size();
  }

  void set_parent(Object* parent)
  {
    kernel::Mutex_Guard locker(m_mutex);
    if (nullptr != m_parent)
      m_parent->m_children.remove(this);

    m_parent = parent;

    if (nullptr != m_parent)
      m_parent->m_children.push_back(this);
  }

  void set_child(Object* child)
  {
    if (nullptr != child)
      child->set_parent(this);
  }

  Object* find_parent(const std::string& name)
  {
    kernel::Mutex_Guard locker(m_mutex);
    if (nullptr == m_parent)
      return nullptr;

    if (m_parent->m_name == name)
      return m_parent;
    else
      return m_parent->find_parent(name);
  }

  Object* find_child(const std::string& name, bool recursion = false);

  std::list<Object*> find_children(const std::string& name, bool recursion = false);

  void remove_parent()
  {
    kernel::Mutex_Guard locker(m_mutex);
    if (nullptr != m_parent)
      m_parent->m_children.remove(this);

    m_parent = nullptr;
  }

  const std::string& name() const
  {
    return m_name;
  }

  Object* parent() const
  {
    return m_parent;
  }

  /* -------------------------------------- 构造与析构 -------------------------------------- */

protected:
  bool m_is_open = false;

public:
  bool is_open() const
  {
    return m_is_open;
  }

  explicit Object(const std::string& name = "", Object* parent = nullptr);
  virtual ~Object()
  {
    {
      kernel::Atomic_Guard atomic;
      m_is_alive = false;
    }
    m_delete_properties();
    m_delete_children();
    remove_parent();
  }
};

template <typename S, typename R, typename... Args>
static bool connect(S* sender, Signal<Args...>& signal, R* receiver, void (R::*slot)(Args...), Connection_Type type = Connection_Auto)
{
  (void)sender;
  return signal.connect(receiver, slot, type);
}

template <typename R, typename... Args>
static bool connect(Signal<Args...>& signal, R* receiver, void (R::*slot)(Args...), Connection_Type type = Connection_Auto)
{
  return signal.connect(receiver, slot, type);
}

template <typename... Args>
static bool connect(Signal<Args...>& signal, void (*slot)(Args...))
{
  return signal.connect(slot);
}

} /* namespace system */
} /* namespace OwO */

/* -------------------------------------- 设置宏定义 -------------------------------------- */

#define O_PROPERTY(TYPE, NAME, VALUE)  add_property<TYPE>(NAME, VALUE);
#define O_PROPERTY_STRING(NAME, VALUE) add_property<std::string>(NAME, VALUE);

#define O_OBJECT                                                       \
public:                                                                \
  static const OwO::system::Meta_Object   static_meta_object;          \
  virtual const OwO::system::Meta_Object* meta_object() const override \
  {                                                                    \
    return &static_meta_object;                                        \
  }

#define O_METAOBJECT(CLASS, SUPER)  const OwO::system::Meta_Object CLASS::static_meta_object = { #CLASS, &SUPER::static_meta_object, sizeof(CLASS) };
#define O_METAOBJECT_NOSUPER(CLASS) const OwO::system::Meta_Object CLASS::static_meta_object = { #CLASS, nullptr, sizeof(CLASS) };

#define NO_COPY(CLASS)                     \
  CLASS(const CLASS&)            = delete; \
  CLASS& operator=(const CLASS&) = delete;

#define NO_MOVE(CLASS)                \
  CLASS(CLASS&&)            = delete; \
  CLASS& operator=(CLASS&&) = delete;

#endif /* __OBJECT_HPP__ */