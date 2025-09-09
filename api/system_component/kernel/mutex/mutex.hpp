#ifndef __CLASS_MUTEX_H__
#define __CLASS_MUTEX_H__

#include "port_os.h"

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
class Mutex
{
  O_MEMORY
private:
  port_os_mutex_t m_mutex;
  Thread*         m_owner;
  uint32_t        m_lock_counter;

public:
  explicit Mutex()
  {
    m_mutex = pt_port_os_mutex_create();
    b_port_os_mutex_release(m_mutex);
    m_owner        = nullptr;
    m_lock_counter = 0;
  }

  void lock();

  void unlock()
  {
    if (--m_lock_counter == 0)
    {
      m_owner = nullptr;
      b_port_os_mutex_release(m_mutex);
    }
  }

  bool try_lock(uint32_t timeout = 0);

  virtual ~Mutex()
  {
    v_port_os_mutex_delete(m_mutex);
  }
};

class Mutex_Guard
{
  O_MEMORY
private:
  Mutex& m_lock;

public:
  explicit Mutex_Guard(Mutex& lock) : m_lock(lock)
  {
    m_lock.lock();
  }

  virtual ~Mutex_Guard()
  {
    m_lock.unlock();
  }
};

class Read_Write_Lock
{
  O_MEMORY
private:
  port_os_semaphore_t m_mutex;
  port_os_semaphore_t m_write_wait_binary;
  uint32_t            m_read_counter;
  Thread*             m_write_owner;
  uint32_t            m_read_recursion;
  uint32_t            m_write_recursion;

public:
  explicit Read_Write_Lock();
  bool lock_read(uint32_t timeout = WAIT_FOREVER);
  bool lock_write(uint32_t timeout = WAIT_FOREVER);
  void unlock();
  virtual ~Read_Write_Lock();
};

} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __CLASS_MUTEX_H__ */
