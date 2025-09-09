#ifndef __TCA9548A_HPP__
#define __TCA9548A_HPP__

#include "thread.hpp"
#include "virtual_iic.hpp"
#include "virtual_gpio.hpp"

/// @brief TCA9548A 默认通讯地址
#define TCA9548A_DEF_ADDRESS 0x70

namespace OwO
{
namespace driver
{
class Tca9548a : public system::Object
{
  O_OBJECT
  O_MEMORY
  NO_COPY(Tca9548a)
  NO_MOVE(Tca9548a)

private:
  /// @brief TCA9548A 错误计数
  uint32_t             m_error_count;
  /// @brief TCA9548A 总线地址
  uint8_t              m_address;
  /// @brief TCA9548A 通讯缓存
  uint8_t              m_tmp;
  /// @brief TCA9548A 总线
  virtual_class::VIIC* m_bus;
  /// @brief TCA9548A 复位引脚
  Class::io*           m_reset_pin;

protected:
  void m_iic_setup()
  {
    m_bus->set_address(m_address << 1);
    m_bus->set_address_type(0);
    m_bus->set_clk_time(2);
  }

public:
  Tca9548a(const std::string& name = "TCA9548A", Object* parent = nullptr) : Object(name, parent)
  {
    m_error_count = 0;
    m_address     = TCA9548A_DEF_ADDRESS;
    m_tmp         = 0;
    m_bus         = nullptr;
    m_reset_pin   = new Class::io("TCA9548A_RESET", this);
  }

  bool open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin, Gpio::Port reset_port, uint8_t reset_pin, uint8_t address = TCA9548A_DEF_ADDRESS)
  {
    if (m_is_open)
      return false;

    m_address  = address;

    m_bus      = new virtual_class::VIIC(name() + "_iic", this);
    bool ret   = true;
    ret       &= m_bus->open(m_address, scl_port, scl_pin, sda_port, sda_pin);
    ret       &= m_reset_pin->open(reset_port, reset_pin);

    if (ret)
    {
      reset();
      m_is_open = true;
    }

    return ret;
  }

  bool open(virtual_class::VIIC* bus, Gpio::Port reset_port, uint8_t reset_pin, uint8_t address = TCA9548A_DEF_ADDRESS)
  {
    if (m_is_open)
      return false;

    m_bus     = bus;
    m_address = address;

    if (m_reset_pin->open(reset_port, reset_pin))
    {
      reset();
      m_is_open = true;
      return true;
    }
    else
      return false;
  }

  bool    set_channel(uint8_t channel);
  uint8_t get_channel();

  void reset()
  {
    m_reset_pin->low();
    system::kernel::Thread::msleep(5);
    m_reset_pin->high();
    system::kernel::Thread::msleep(5);
  }

  bool close()
  {
    bool ret = true;
    if (m_bus->parent() == this)
      ret &= m_bus->close();
    ret &= m_reset_pin->close();

    if (ret)
    {
      m_is_open = false;
      m_bus     = nullptr;
    }
    return ret;
  }

  uint8_t operator=(uint8_t channel)
  {
    set_channel(channel);
    return get_channel();
  }

  virtual_class::VIIC* bus()
  {
    return m_bus;
  }

  virtual ~Tca9548a()
  {
    close();
  }
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __TCA9548A_HPP__ */
