#ifndef __REGISTER_HPP__
#define __REGISTER_HPP__

#include "object.hpp"
#include "relay.hpp"
#include "delixi_meter.hpp"

namespace OwO
{
namespace protocol
{
namespace modbus
{
enum Modbus_Mode
{
  Modbus_TCP,
  Modbus_RTU,
};

class Register : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Register)
  NO_MOVE(Register)
private:
  uint16_t*                     m_registers;
  uint16_t                      m_size;
  mutable system::kernel::Mutex m_mutex;

  friend class Modbus_Slave;
  friend class Modbus_Master;

protected:
  void read(uint8_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length))
      return;

    m_mutex.lock();
    for (uint16_t i = 0; i < length; i++)
    {
      values[i * 2]     = m_registers[pos + i] >> 8;
      values[i * 2 + 1] = m_registers[pos + i] & 0xFF;
    }
    m_mutex.unlock();
  }

  void write(const void* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length))
      return;

    m_mutex.lock();
    for (uint16_t i = 0; i < length; i++)
    {
      m_registers[pos + i] = (static_cast<const uint8_t*>(values)[i * 2] << 8) | static_cast<const uint8_t*>(values)[i * 2 + 1];
    }
    m_mutex.unlock();
  }

  void mask_write(const uint16_t and_mask, const uint16_t or_mask, const uint16_t pos)
  {
    if (!is_valid(pos))
      return;

    m_mutex.lock();
    m_registers[pos] &= and_mask;
    m_registers[pos] |= or_mask;
    m_mutex.unlock();
  }

  bool is_valid(const uint16_t pos, const uint16_t length = 0) const
  {
    return pos + length <= m_size;
  }

public:
  Register(const std::string& name = "Register", Object* parent = nullptr, const uint16_t size = 128) : Object(name, parent)
  {
    m_size      = size;
    m_registers = static_cast<uint16_t*>(Malloc(m_size * sizeof(uint16_t)));
    memset(m_registers, 0, m_size * sizeof(uint16_t));
    m_mutex.unlock();
  }

  Register& set(const bool value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = value;
    m_mutex.unlock();
    return *this;
  }

  Register& set(const int value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = value;
    m_mutex.unlock();
    return *this;
  }

  Register& set(const int* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(int));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const char value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = value;
    m_mutex.unlock();
    return *this;
  }

  Register& set(const char* values, const uint16_t length, const uint16_t pos, bool reversal = true)
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return *this;

    m_mutex.lock();
    if (false == reversal)
      memcpy(m_registers + pos, values, length * sizeof(char));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
        m_registers[pos + i] = (values[i * 2] << 8) | values[i * 2 + 1];

      if (length % 2 == 1)
        m_registers[pos + length / 2] = values[length - 1] << 8;
    }
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint8_t value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = value;
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint8_t* values, const uint16_t length, const uint16_t pos, bool reversal = true)
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return *this;

    m_mutex.lock();
    if (false == reversal)
      memcpy(m_registers + pos, values, length * sizeof(uint8_t));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
        m_registers[pos + i] = (values[i * 2] << 8) | values[i * 2 + 1];

      if (length % 2 == 1)
        m_registers[pos + length / 2] = values[length - 1] << 8;
    }
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint16_t value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = value;
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint16_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(uint16_t));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint32_t value, const uint16_t pos)
  {
    if (!is_valid(pos, 2))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, &value, sizeof(uint32_t));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint32_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(uint32_t));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint64_t value, const uint16_t pos)
  {
    if (!is_valid(pos, 4))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, &value, sizeof(uint64_t));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const uint64_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 4))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(uint64_t));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const float value, const uint16_t pos)
  {
    if (!is_valid(pos, 2))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, &value, sizeof(float));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const float* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(float));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const double value, const uint16_t pos)
  {
    if (!is_valid(pos, 4))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, &value, sizeof(double));
    m_mutex.unlock();
    return *this;
  }

  Register& set(const double* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 4))
      return *this;

    m_mutex.lock();
    memcpy(m_registers + pos, values, length * sizeof(double));
    m_mutex.unlock();
    return *this;
  }

  Register& no_lock_set(const bool value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_registers[pos] = value;

    return *this;
  }

  Register& no_lock_set(const int value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_registers[pos] = value;

    return *this;
  }

  Register& no_lock_set(const int* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(int));

    return *this;
  }

  Register& no_lock_set(const char value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_registers[pos] = value;

    return *this;
  }

  Register& no_lock_set(const char* values, const uint16_t length, const uint16_t pos, bool reversal = true)
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return *this;

    if (false == reversal)
      memcpy(m_registers + pos, values, length * sizeof(char));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
        m_registers[pos + i] = (values[i * 2] << 8) | values[i * 2 + 1];

      if (length % 2 == 1)
        m_registers[pos + length / 2] = values[length - 1] << 8;
    }

    return *this;
  }

  Register& no_lock_set(const uint8_t value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_registers[pos] = value;

    return *this;
  }

  Register& no_lock_set(const uint8_t* values, const uint16_t length, const uint16_t pos, bool reversal = true)
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return *this;

    if (false == reversal)
      memcpy(m_registers + pos, values, length * sizeof(uint8_t));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
        m_registers[pos + i] = (values[i * 2] << 8) | values[i * 2 + 1];

      if (length % 2 == 1)
        m_registers[pos + length / 2] = values[length - 1] << 8;
    }

    return *this;
  }

  Register& no_lock_set(const uint16_t value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_registers[pos] = value;

    return *this;
  }

  Register& no_lock_set(const uint16_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(uint16_t));

    return *this;
  }

  Register& no_lock_set(const uint32_t value, const uint16_t pos)
  {
    if (!is_valid(pos, 2))
      return *this;

    memcpy(m_registers + pos, &value, sizeof(uint32_t));

    return *this;
  }

  Register& no_lock_set(const uint32_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(uint32_t));

    return *this;
  }

  Register& no_lock_set(const uint64_t value, const uint16_t pos)
  {
    if (!is_valid(pos, 4))
      return *this;

    memcpy(m_registers + pos, &value, sizeof(uint64_t));

    return *this;
  }

  Register& no_lock_set(const uint64_t* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 4))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(uint64_t));

    return *this;
  }

  Register& no_lock_set(const float value, const uint16_t pos)
  {
    if (!is_valid(pos, 2))
      return *this;

    memcpy(m_registers + pos, &value, sizeof(float));

    return *this;
  }

  Register& no_lock_set(const float* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 2))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(float));

    return *this;
  }

  Register& no_lock_set(const double value, const uint16_t pos)
  {
    if (!is_valid(pos, 4))
      return *this;

    memcpy(m_registers + pos, &value, sizeof(double));

    return *this;
  }

  Register& no_lock_set(const double* values, const uint16_t length, const uint16_t pos)
  {
    if (!is_valid(pos, length * 4))
      return *this;

    memcpy(m_registers + pos, values, length * sizeof(double));

    return *this;
  }

  bool& get(bool& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    m_mutex.lock();
    value = m_registers[pos];
    m_mutex.unlock();
    return value;
  }

  char& get(char& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    m_mutex.lock();
    value = m_registers[pos];
    m_mutex.unlock();
    return value;
  }

  char* get(char* values, const uint16_t length, const uint16_t pos, bool reversal = true) const
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return values;

    m_mutex.lock();
    if (false == reversal)
      memcpy(values, m_registers + pos, length * sizeof(char));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
      {
        values[i * 2]     = m_registers[pos + i] >> 8;
        values[i * 2 + 1] = m_registers[pos + i] & 0xFF;
      }

      if (length % 2 == 1)
        values[length - 1] = m_registers[pos + length / 2] >> 8;
    }
    m_mutex.unlock();
    return values;
  }

  uint8_t& get(uint8_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    m_mutex.lock();
    value = m_registers[pos];
    m_mutex.unlock();
    return value;
  }

  uint8_t* get(uint8_t* values, const uint16_t length, const uint16_t pos, bool reversal = true) const
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return values;

    m_mutex.lock();
    if (false == reversal)
      memcpy(values, m_registers + pos, length * sizeof(uint8_t));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
      {
        values[i * 2]     = m_registers[pos + i] >> 8;
        values[i * 2 + 1] = m_registers[pos + i] & 0xFF;
      }

      if (length % 2 == 1)
        values[length - 1] = m_registers[pos + length / 2] >> 8;
    }
    m_mutex.unlock();
    return values;
  }

  uint16_t& get(uint16_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    m_mutex.lock();
    value = m_registers[pos];
    m_mutex.unlock();
    return value;
  }

  uint16_t* get(uint16_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length))
      return values;

    m_mutex.lock();
    memcpy(values, m_registers + pos, length * sizeof(uint16_t));
    m_mutex.unlock();
    return values;
  }

  uint32_t& get(uint32_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 2))
      return value;

    m_mutex.lock();
    memcpy(&value, m_registers + pos, sizeof(uint32_t));
    m_mutex.unlock();
    return value;
  }

  uint32_t* get(uint32_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 2))
      return values;

    m_mutex.lock();
    memcpy(values, m_registers + pos, length * sizeof(uint32_t));
    m_mutex.unlock();
    return values;
  }

  uint64_t& get(uint64_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 4))
      return value;

    m_mutex.lock();
    memcpy(&value, m_registers + pos, sizeof(uint64_t));
    m_mutex.unlock();
    return value;
  }

  uint64_t* get(uint64_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 4))
      return values;

    m_mutex.lock();
    memcpy(values, m_registers + pos, length * sizeof(uint64_t));
    m_mutex.unlock();
    return values;
  }

  float& get(float& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 2))
      return value;

    m_mutex.lock();
    memcpy(&value, m_registers + pos, sizeof(float));
    m_mutex.unlock();
    return value;
  }

  float* get(float* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 2))
      return values;

    m_mutex.lock();
    memcpy(values, m_registers + pos, length * sizeof(float));
    m_mutex.unlock();
    return values;
  }

  double& get(double& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 4))
      return value;

    m_mutex.lock();
    memcpy(&value, m_registers + pos, sizeof(double));
    m_mutex.unlock();
    return value;
  }

  double* get(double* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 4))
      return values;

    m_mutex.lock();
    memcpy(values, m_registers + pos, length * sizeof(double));
    m_mutex.unlock();
    return values;
  }

  bool& no_lock_get(bool& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    value = m_registers[pos];

    return value;
  }

  char& no_lock_get(char& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    value = m_registers[pos];

    return value;
  }

  char* no_lock_get(char* values, const uint16_t length, const uint16_t pos, bool reversal = true) const
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return values;

    if (false == reversal)
      memcpy(values, m_registers + pos, length * sizeof(char));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
      {
        values[i * 2]     = m_registers[pos + i] >> 8;
        values[i * 2 + 1] = m_registers[pos + i] & 0xFF;
      }

      if (length % 2 == 1)
        values[length - 1] = m_registers[pos + length / 2] >> 8;
    }

    return values;
  }

  uint8_t& no_lock_get(uint8_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    value = m_registers[pos];

    return value;
  }

  uint8_t* no_lock_get(uint8_t* values, const uint16_t length, const uint16_t pos, bool reversal = true) const
  {
    if (!is_valid(pos, (length % 2) ? length / 2 + 1 : length / 2))
      return values;

    if (false == reversal)
      memcpy(values, m_registers + pos, length * sizeof(uint8_t));
    else
    {
      for (uint16_t i = 0; i < length / 2; i++)
      {
        values[i * 2]     = m_registers[pos + i] >> 8;
        values[i * 2 + 1] = m_registers[pos + i] & 0xFF;
      }

      if (length % 2 == 1)
        values[length - 1] = m_registers[pos + length / 2] >> 8;
    }

    return values;
  }

  uint16_t& no_lock_get(uint16_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos))
      return value;

    value = m_registers[pos];

    return value;
  }

  uint16_t* no_lock_get(uint16_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length))
      return values;

    memcpy(values, m_registers + pos, length * sizeof(uint16_t));

    return values;
  }

  uint32_t& no_lock_get(uint32_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 2))
      return value;

    memcpy(&value, m_registers + pos, sizeof(uint32_t));

    return value;
  }

  uint32_t* no_lock_get(uint32_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 2))
      return values;

    memcpy(values, m_registers + pos, length * sizeof(uint32_t));

    return values;
  }

  uint64_t& no_lock_get(uint64_t& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 4))
      return value;

    memcpy(&value, m_registers + pos, sizeof(uint64_t));

    return value;
  }

  uint64_t* no_lock_get(uint64_t* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 4))
      return values;

    memcpy(values, m_registers + pos, length * sizeof(uint64_t));

    return values;
  }

  float& no_lock_get(float& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 2))
      return value;

    memcpy(&value, m_registers + pos, sizeof(float));

    return value;
  }

  float* no_lock_get(float* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 2))
      return values;

    memcpy(values, m_registers + pos, length * sizeof(float));

    return values;
  }

  double& no_lock_get(double& value, const uint16_t pos) const
  {
    if (!is_valid(pos, 4))
      return value;

    memcpy(&value, m_registers + pos, sizeof(double));

    return value;
  }

  double* no_lock_get(double* values, const uint16_t length, const uint16_t pos) const
  {
    if (!is_valid(pos, length * 4))
      return values;

    memcpy(values, m_registers + pos, length * sizeof(double));

    return values;
  }

  Register& clear()
  {
    m_mutex.lock();
    memset(m_registers, 0, m_size * sizeof(uint16_t));
    m_mutex.unlock();
    return *this;
  }

  Register& clear(const uint16_t pos)
  {
    if (!is_valid(pos))
      return *this;

    m_mutex.lock();
    m_registers[pos] = 0;
    m_mutex.unlock();
    return *this;
  }

  Register& clear(const uint16_t pos, const uint16_t length)
  {
    if (!is_valid(pos, length))
      return *this;

    m_mutex.lock();
    memset(m_registers + pos, 0, length * sizeof(uint16_t));
    m_mutex.unlock();
    return *this;
  }

  const uint16_t size() const
  {
    return m_size;
  }

  system::kernel::Mutex& mutex() const
  {
    return m_mutex;
  }

  uint16_t& operator[](const uint16_t index)
  {
    return m_registers[index];
  }

  Register& set(const device::Delixi_Meter& meter, const uint16_t pos)
  {
    if (!is_valid(pos, sizeof(device::Delixi_Meter::meter_data_t) / 2))
      return *this;

    meter.mutex().lock();
    write(&meter.get_data(), sizeof(device::Delixi_Meter::meter_data_t) / 2, pos);
    meter.mutex().unlock();
    return *this;
  }

  bool get(device::Relay& value, const uint16_t pos)
  {
    if (!is_valid(pos))
      return false;

    m_mutex.lock();
    value = m_registers[pos];
    m_mutex.unlock();
    return value;
  }

  virtual ~Register()
  {
    Free(m_registers);
  }
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace OwO */

#endif /* __REGISTER_HPP__ */
