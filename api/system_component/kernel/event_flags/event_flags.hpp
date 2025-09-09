#ifndef __EVENT_FLAGS_HPP__
#define __EVENT_FLAGS_HPP__

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
/// @brief 类 事件标志
class Event_Flags
{
  O_MEMORY
private:
  port_os_event_t m_event;

public:
  enum event_option
  {
    Wait_Any      = 0x01,
    Wait_All      = 0x02,
    Clear_On_Exit = 0x10,
  };

  explicit Event_Flags()
  {
    m_event = pt_port_os_event_create();
  }

  uint32_t wait(uint32_t bits, uint32_t timeout = WAIT_FOREVER, uint16_t options = Wait_Any)
  {
    if (0 != (options & Clear_On_Exit))
    {
      uint32_t ret = ul_port_os_event_wait(m_event, bits, timeout, (options & Wait_All) != 0);
      ul_port_os_event_clear(m_event, bits);
      return ret;
    }
    else
      return ul_port_os_event_wait(m_event, bits, timeout, (options & Wait_All) != 0);
  }

  bool set(uint32_t bits)
  {
    return ((ul_port_os_event_set(m_event, bits) & bits) == bits);
  }

  bool clear(uint32_t bits)
  {
    return ((ul_port_os_event_clear(m_event, bits) & bits) == bits);
  }

  virtual ~Event_Flags()
  {
    v_port_os_event_delete(m_event);
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __EVENT_FLAGS_HPP__ */