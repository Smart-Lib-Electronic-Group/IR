#ifndef __OROM_HPP__
#define __OROM_HPP__

#include "ioport.hpp"

namespace OwO
{
namespace system
{
class ORom : public Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(ORom)
  NO_MOVE(ORom)
private:
  system::kernel::Mutex m_mutex;
  IOPort*               m_port;

public:
  ORom(const std::string& name = "orom", Object* parent = nullptr) : Object(name, parent), m_port(nullptr) {}

  virtual bool open(IOPort* port)
  {
    if (is_open())
      return false;

    m_port    = port;
    m_is_open = true;
    return true;
  }

  virtual bool close()
  {
    if (!is_open())
      return true;

    m_is_open = false;
    return true;
  }

  uint32_t read(uint32_t position, void* data, uint32_t len)
  {
    if (!is_open())
      return false;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_port->read(data, len, position);
  }

  uint32_t write(uint32_t position, const void* data, uint32_t len)
  {
    if (!is_open())
      return false;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_port->write(data, len, position);
  }

  uint32_t erase(uint32_t position, uint32_t len, const char erase_bit = 0x00)
  {
    if (!is_open())
      return false;

    system::kernel::Mutex_Guard lock(m_mutex);
    return m_port->erase(position, len, erase_bit);
  }

  bool read(uint32_t position, char& data)
  {
    return m_port->read(&data, 1, position) == 1;
  }

  bool read(uint32_t position, int& data)
  {
    return m_port->read(&data, 4, position) == 4;
  }

  bool read(uint32_t position, float& data)
  {
    return m_port->read(&data, 4, position) == 4;
  }

  bool read(uint32_t position, double& data)
  {
    return m_port->read(&data, 8, position) == 8;
  }

  bool read(uint32_t position, uint8_t& data)
  {
    return m_port->read(&data, 1, position) == 1;
  }

  bool read(uint32_t position, uint16_t& data)
  {
    return m_port->read(&data, 2, position) == 2;
  }

  bool read(uint32_t position, uint32_t& data)
  {
    return m_port->read(&data, 4, position) == 4;
  }

  bool read(uint32_t position, uint64_t& data)
  {
    return m_port->read(&data, 8, position) == 8;
  }

  bool write(uint32_t position, const char data)
  {
    return m_port->write(&data, 1, position) == 1;
  }

  bool write(uint32_t position, const int data)
  {
    return m_port->write(&data, 4, position) == 4;
  }

  bool write(uint32_t position, const float data)
  {
    return m_port->write(&data, 4, position) == 4;
  }

  bool write(uint32_t position, const double data)
  {
    return m_port->write(&data, 8, position) == 8;
  }

  bool write(uint32_t position, const uint8_t data)
  {
    return m_port->write(&data, 1, position) == 1;
  }

  bool write(uint32_t position, const uint16_t data)
  {
    return m_port->write(&data, 2, position) == 2;
  }

  bool write(uint32_t position, const uint32_t data)
  {
    return m_port->write(&data, 4, position) == 4;
  }

  bool write(uint32_t position, const uint64_t data)
  {
    return m_port->write(&data, 8, position) == 8;
  }

  bool erase(uint32_t position, const char erase_bit = 0x00)
  {
    return m_port->erase(1, position, erase_bit) == 1;
  }

  virtual ~ORom()
  {
    close();
  }
};
} /* namespace system */
} /* namespace OwO */

#endif /* __OROM_HPP__ */
