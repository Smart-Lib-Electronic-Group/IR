#ifndef __SEMAPHORE_HPP__
#define __SEMAPHORE_HPP__

#include "port_os.h"

#define SEMAPHORE_MAX_COUNT 0xFF

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
/// @brief 名称空间 系统接口
namespace kernel
{
/// @brief 类 信号量
class Semaphore
{
  O_MEMORY
private:
  port_os_semaphore_t m_semaphore;

public:
  explicit Semaphore(uint32_t size = SEMAPHORE_MAX_COUNT, uint32_t init_count = SEMAPHORE_MAX_COUNT)
  {
    m_semaphore = pt_port_os_semaphore_create(size, init_count);
  }

  bool acquire()
  {
    return b_port_os_semaphore_take(m_semaphore, WAIT_FOREVER);
  }

  bool try_acquire(uint32_t timeout = 0)
  {
    return b_port_os_semaphore_take(m_semaphore, timeout);
  }

  bool release()
  {
    return b_port_os_semaphore_give(m_semaphore);
  }

  virtual ~Semaphore()
  {
    v_port_os_semaphore_delete(m_semaphore);
  }
};

class Semaphore_Guard
{
private:
  Semaphore& m_semaphore;

public:
  explicit Semaphore_Guard(Semaphore& semaphore) : m_semaphore(semaphore)
  {
    m_semaphore.acquire();
  }

  ~Semaphore_Guard()
  {
    m_semaphore.release();
  }
};

} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __SEMAPHORE_HPP__ */