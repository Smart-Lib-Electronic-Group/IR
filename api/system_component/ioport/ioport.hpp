#ifndef __IOPORT_HPP__
#define __IOPORT_HPP__

#include "object.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
class IOPort : public Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(IOPort)
  NO_MOVE(IOPort)
private:
  system::kernel::Mutex m_mutex;
  uint8_t               m_address;

protected:
  virtual uint32_t m_read(void* data, uint32_t len, uint8_t device_address, uint32_t reg_address)            = 0;
  virtual uint32_t m_write(const void* data, uint32_t len, uint8_t device_address, uint32_t reg_address)     = 0;
  virtual uint32_t m_recv(void* data, uint32_t len, uint8_t device_address)                                  = 0;
  virtual uint32_t m_send(const void* data, uint32_t len, uint8_t device_address)                            = 0;
  virtual uint32_t m_erase(uint32_t len, uint8_t device_address, uint32_t reg_address, const char erase_bit) = 0;

public:
  IOPort(const std::string& name = "IOPort", Object* parent = nullptr) : Object(name, parent)
  {
    m_address = 0;
    m_mutex.lock();
  }

  virtual bool open(uint8_t address)
  {
    if (is_open())
      return false;

    m_address = address;
    m_is_open = true;
    m_mutex.unlock();
    return true;
  }

  uint32_t read(void* data, uint32_t len, uint32_t reg_address)
  {
    if (!is_open())
      return 0;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_read(data, len, m_address, reg_address);
  }

  uint32_t write(const void* data, uint32_t len, uint32_t reg_address)
  {
    if (!is_open())
      return 0;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_write(data, len, m_address, reg_address);
  }

  uint32_t erase(uint32_t len, uint32_t reg_address, const char erase_bit = 0x00)
  {
    if (!is_open())
      return 0;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_erase(len, m_address, reg_address, erase_bit);
  }

  uint32_t recv(void* data, uint32_t len)
  {
    if (!is_open())
      return 0;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_recv(data, len, m_address);
  }

  uint32_t send(const void* data, uint32_t len)
  {
    if (!is_open())
      return 0;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_send(data, len, m_address);
  }

  void set_address(uint8_t address)
  {
    m_address = address;
  }

  virtual bool close()
  {
    if (!is_open())
      return true;

    m_is_open = false;
    m_mutex.lock();
    return true;
  }

  virtual ~IOPort()
  {
    close();
  }
};
} /* namespace system */
} /* namespace OwO */

#endif /* __IOPORT_HPP__ */
