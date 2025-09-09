#ifndef __ATOMIC_HPP__
#define __ATOMIC_HPP__

#include "port_os.h"
#include <atomic>

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
/// @brief 名称空间 系统接口
namespace kernel
{

class Atomic_Guard
{
  O_MEMORY
public:
  Atomic_Guard()
  {
    v_port_os_enter_critical();
  }

  ~Atomic_Guard()
  {
    v_port_os_exit_critical();
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __ATOMIC_HPP__ */