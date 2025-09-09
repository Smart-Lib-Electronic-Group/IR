#ifndef __MESSAGE_QUEUE_HPP__
#define __MESSAGE_QUEUE_HPP__

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
template <typename T>
class Message_Queue
{
  O_MEMORY
private:
  port_os_message_t m_message;
  uint32_t          m_size;

public:
  explicit Message_Queue(uint32_t size)
  {
    m_size    = size;
    m_message = pt_port_os_message_create(size, sizeof(T));
  }

  bool send(const T& data, uint32_t timeout = WAIT_FOREVER)
  {
    return b_port_os_message_send(m_message, &data, timeout);
  }

  bool receive(T& data, uint32_t timeout = WAIT_FOREVER)
  {
    return b_port_os_message_receive(m_message, &data, timeout);
  }

  T receive(uint32_t timeout = WAIT_FOREVER)
  {
    T data;
    if (b_port_os_message_receive(m_message, &data, timeout))
      return data;
    else
      return T();
  }

  bool peek(T& data, uint32_t timeout = WAIT_FOREVER)
  {
    return b_port_os_message_peek(m_message, &data, timeout);
  }

  T peek(uint32_t timeout = WAIT_FOREVER)
  {
    T data;
    if (b_port_os_message_peek(m_message, &data, timeout))
      return data;
    else
      return T();
  }

  uint32_t available() const
  {
    return ul_port_os_message_available(m_message);
  }

  uint32_t space_available() const
  {
    return ul_port_os_message_spaces_available(m_message);
  }

  bool empty() const
  {
    return available() == 0;
  }

  bool full() const
  {
    return available() == m_size;
  }

  void reset()
  {
    v_port_os_message_reset(m_message);
  }

  virtual ~Message_Queue()
  {
    v_port_os_message_delete(m_message);
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __MESSAGE_QUEUE_HPP__ */