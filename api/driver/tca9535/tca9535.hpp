#ifndef __TCA9535_HPP__
#define __TCA9535_HPP__

#include "virtual_iic.hpp"
#include "thread.hpp"

#define TCA9535_ADDR 0x20

/// @brief 名称空间 库名
namespace OwO
{
namespace driver
{
class Tca9535 : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Tca9535)
  NO_MOVE(Tca9535)
private:
  virtual_class::VIIC* m_bus;

  static constexpr inline uint8_t port_00_input_register_addr         = 0x00;
  static constexpr inline uint8_t port_01_input_register_addr         = 0x01;
  static constexpr inline uint8_t port_00_output_register_addr        = 0x02;
  static constexpr inline uint8_t port_01_output_register_addr        = 0x03;
  static constexpr inline uint8_t port_00_polarity_inversion_register = 0x04;
  static constexpr inline uint8_t port_01_polarity_inversion_register = 0x05;
  static constexpr inline uint8_t port_00_configuration_register_addr = 0x06;
  static constexpr inline uint8_t port_01_configuration_register_addr = 0x07;

protected:
  bool read_output_register(const uint8_t pin, bool& value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_output_register_addr))
      {
        value = ((data & (1 << pin)) != 0);
        return true;
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_output_register_addr))
      {
        value = ((data & (1 << (15 - pin))) != 0);
        return true;
      }
    }
    return false;
  }

  bool write_output_register(const uint8_t pin, const bool value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_output_register_addr))
      {
        if (value)
        {
          data |= (1 << pin);
        }
        else
        {
          data &= ~(1 << pin);
        }
        return m_bus->write(&data, 1, port_00_output_register_addr);
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_output_register_addr))
      {
        if (value)
        {
          data |= (1 << (15 - pin));
        }
        else
        {
          data &= ~(1 << (15 - pin));
        }
        return m_bus->write(&data, 1, port_01_output_register_addr);
      }
    }
    return false;
  }

  bool read_input_register(const uint8_t pin, bool& value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_input_register_addr))
      {
        value = (data & (1 << pin)) != 0;
        return true;
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_input_register_addr))
      {
        value = (data & (1 << (15 - pin))) != 0;
        return true;
      }
    }
    return false;
  }

  bool read_configuration(const uint8_t pin, bool& value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_configuration_register_addr))
      {
        value = (data & (1 << pin)) != 0;
        return true;
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_configuration_register_addr))
      {
        value = (data & (1 << (15 - pin))) != 0;
        return true;
      }
    }
    return false;
  }

  bool write_configuration(const uint8_t pin, const bool value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_configuration_register_addr))
      {
        if (value)
        {
          data |= (1 << pin);
        }
        else
        {
          data &= ~(1 << pin);
        }
        return m_bus->write(&data, 1, port_00_configuration_register_addr);
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_configuration_register_addr))
      {
        if (value)
        {
          data |= (1 << (15 - pin));
        }
        else
        {
          data &= ~(1 << (15 - pin));
        }
        return m_bus->write(&data, 1, port_01_configuration_register_addr);
      }
    }
    return false;
  }

  bool read_polarity_inversion_register(const uint8_t pin, bool& value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_polarity_inversion_register))
      {
        value = (data & (1 << pin)) != 0;
        return true;
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_polarity_inversion_register))
      {
        value = (data & (1 << (15 - pin))) != 0;
        return true;
      }
    }
    return false;
  }

  bool write_polarity_inversion_register(const uint8_t pin, const bool value)
  {
    uint8_t data = 0;
    if (pin < 8)
    {
      if (m_bus->read(&data, 1, port_00_polarity_inversion_register))
      {
        if (value)
        {
          data |= (1 << pin);
        }
        else
        {
          data &= ~(1 << pin);
        }
        return m_bus->write(&data, 1, port_00_polarity_inversion_register);
      }
    }
    else
    {
      if (m_bus->read(&data, 1, port_01_polarity_inversion_register))
      {
        if (value)
        {
          data |= (1 << (15 - pin));
        }
        else
        {
          data &= ~(1 << (15 - pin));
        }
        return m_bus->write(&data, 1, port_01_polarity_inversion_register);
      }
    }
    return false;
  }

public:
  Tca9535(const std::string& name = "TCA9535", Object* parent = nullptr) {}

  virtual bool open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin)
  {
    m_bus = new virtual_class::VIIC(name() + "_iic", this);
    return m_bus->open(TCA9535_ADDR << 1, scl_port, scl_pin, sda_port, sda_pin, 1, 2);
  }

  virtual bool open(virtual_class::VIIC* bus)
  {
    m_bus = bus;
    return true;
  }

  bool set_input(const uint8_t pin, const bool polarity_inversion = false)
  {
    bool ret  = true;
    ret      &= write_configuration(pin, true);
    ret      &= write_polarity_inversion_register(pin, polarity_inversion);
    return ret;
  }

  bool set_output(const uint8_t pin)
  {
    return write_configuration(pin, false);
  }

  bool read(const uint8_t pin, bool& value)
  {
    return read_input_register(pin, value);
  }

  bool read(const uint8_t pin)
  {
    bool value = false;
    read_input_register(pin, value);
    return value;
  }

  bool read_output(const uint8_t pin)
  {
    bool value = false;
    read_output_register(pin, value);
    return value;
  }

  bool write(const uint8_t pin, const bool value)
  {
    return write_output_register(pin, value);
  }

  bool toggle(const uint8_t pin)
  {
    bool value = false;
    if (read_output_register(pin, value))
    {
      return write_output_register(pin, !value);
    }
    return false;
  }

  bool set_polarity_inversion(const uint8_t pin, const bool value)
  {
    return write_polarity_inversion_register(pin, value);
  }

  bool get_polarity_inversion(const uint8_t pin, bool& value)
  {
    return read_polarity_inversion_register(pin, value);
  }

  bool is_input(const uint8_t pin)
  {
    bool value = false;
    return read_configuration(pin, value) && value;
  }

  bool is_output(const uint8_t pin)
  {
    bool value = false;
    return read_configuration(pin, value) && !value;
  }

  virtual bool close()
  {
    if (m_bus->parent() == this)
    {
      delete m_bus;
      m_bus = nullptr;
    }
    return true;
  }

  virtual ~Tca9535()
  {
    close();
  }
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __TCA9535_HPP__ */
