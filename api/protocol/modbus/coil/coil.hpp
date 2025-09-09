#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "register.hpp"

namespace OwO
{
namespace protocol
{
namespace modbus
{
class Coil : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Coil)
  NO_MOVE(Coil)
private:
  uint8_t*                      m_coils;
  uint16_t                      m_size;
  mutable system::kernel::Mutex m_mutex;

  friend class Modbus_Slave;
  friend class Modbus_Master;

protected:
  void read(uint8_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length))
      return;

    memset(values, 0, (length % 8 == 0) ? length / 8 : length / 8 + 1);

    m_mutex.lock();
    for (uint16_t i = 0; i < length; i++)
    {
      values[i / 8] |= (m_coils[(pos + i) / 8] & (0x01 << (i % 8)));
    }
    m_mutex.unlock();
  }

  void write(const uint8_t value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return;

    m_mutex.lock();
    m_coils[pos / 8] &= ~(0x01 << (pos % 8));
    m_coils[pos / 8] |= (value & (0x01 << (pos % 8)));
    m_mutex.unlock();
  }

  void write(const uint8_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length))
      return;

    m_mutex.lock();
    for (uint16_t i = 0; i < length; i++)
    {
      m_coils[(pos + i) / 8] &= ~(0x01 << ((pos + i) % 8));
      m_coils[(pos + i) / 8] |= (values[i / 8] & (0x01 << (i % 8)));
    }
    m_mutex.unlock();
  }

  bool is_valid(const uint16_t pos, const uint16_t length = 0) const
  {
    return pos + length < m_size;
  }

public:
  Coil(const std::string& name = "Register", Object* parent = nullptr, const uint16_t size = 128) : system::Object(name, parent)
  {
    m_size  = size;
    m_coils = static_cast<uint8_t*>(Malloc((size % 8 == 0) ? size / 8 : size / 8 + 1));
    memset(m_coils, 0, (size % 8 == 0) ? size / 8 : size / 8 + 1);
    m_mutex.unlock();
  }

  Coil& set(bool value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_coils[pos / 8] &= ~(0x01 << (pos % 8));
    m_coils[pos / 8] |= (value << (pos % 8));
    m_mutex.unlock();
    return *this;
  }

  Coil& no_lock_set(bool value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_coils[pos / 8] &= ~(0x01 << (pos % 8));
    m_coils[pos / 8] |= (value << (pos % 8));
    return *this;
  }

  Coil& get(bool& value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    value = (m_coils[pos / 8] >> (pos % 8)) & 0x01;
    m_mutex.unlock();
    return *this;
  }

  Coil& no_lock_get(bool& value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    value = (m_coils[pos / 8] >> (pos % 8)) & 0x01;
    return *this;
  }

  bool get(const uint16_t pos) const
  {
    if (!is_valid(pos))
      return false;

    m_mutex.lock();
    bool value = (m_coils[pos / 8] >> (pos % 8)) & 0x01;
    m_mutex.unlock();
    return value;
  }

  bool no_lock_get(const uint16_t pos) const
  {
    if (!is_valid(pos))
      return false;

    bool value = (m_coils[pos / 8] >> (pos % 8)) & 0x01;
    return value;
  }

  Coil& on(const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_coils[pos / 8] |= 0x01 << (pos % 8);
    m_mutex.unlock();
    return *this;
  }

  Coil& off(const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_coils[pos / 8] &= ~0x01 << (pos % 8);
    m_mutex.unlock();
    return *this;
  }

  Coil& toggle(const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_coils[pos / 8] ^= (0x01 << (pos % 8));
    m_mutex.unlock();
    return *this;
  }

  bool is_on(const uint16_t pos, const uint16_t length = 1) const
  {
    if (!is_valid(pos, length))
      return false;

    m_mutex.lock();
    bool value = true;
    for (uint16_t i = 0; i < length; i++)
    {
      value &= (m_coils[(pos + i) / 8] & (0x01 << (pos % 8)));
    }
    m_mutex.unlock();
    return value;
  }

  bool is_off(const uint16_t pos, const uint16_t length = 1) const
  {
    if (!is_valid(pos, length))
      return false;

    m_mutex.lock();
    bool value = true;
    for (uint16_t i = 0; i < length; i++)
    {
      value &= ~(m_coils[(pos + i) / 8] & (0x01 << (pos % 8)));
    }
    m_mutex.unlock();
    return value;
  }

  void clear()
  {
    m_mutex.lock();
    memset(m_coils, 0, (m_size % 8 == 0) ? m_size / 8 : m_size / 8 + 1);
    m_mutex.unlock();
  }

  void clear(const uint16_t pos, const uint16_t length = 1)
  {
    if (!is_valid(pos, length))
      return;

    m_mutex.lock();
    for (uint16_t i = 0; i < length; i++)
    {
      m_coils[(pos + i) / 8] &= ~(0x01 << ((pos + i) % 8));
    }
    m_mutex.unlock();
  }

  uint16_t size() const
  {
    return m_size;
  }

  system::kernel::Mutex& mutex() const
  {
    return m_mutex;
  }

  virtual ~Coil()
  {
    Free(m_coils);
  }
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace OwO */

#endif /* __COIL_HPP__ */
