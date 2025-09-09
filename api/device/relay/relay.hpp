#ifndef __RELAY_HPP__
#define __RELAY_HPP__

#include "virtual_gpio.hpp"

/// @brief 名称空间 库名
namespace OwO
{
namespace device
{
class Relay : public virtual_class::VGpio
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Relay)
  NO_MOVE(Relay)

  bool m_state = false;

private:
  virtual void irq_callback() {}

  using virtual_class::VGpio::open;

public:
  Relay(const std::string& name = "Relay", Object* parent = nullptr) : VGpio(name, parent) {}

  virtual bool open(Gpio::Port port, uint8_t pin, Gpio::Level level = Gpio::LEVEL_HIGH)
  {
    return VGpio::open(port, pin, Gpio::OUT_PP, level);
  }

  Relay& on()
  {
    if (level() == Gpio::LEVEL_LOW)
      low();
    else if (level() == Gpio::LEVEL_HIGH)
      high();

    m_state = true;
    return *this;
  }

  Relay& off()
  {
    if (level() == Gpio::LEVEL_LOW)
      high();
    else if (level() == Gpio::LEVEL_HIGH)
      low();

    m_state = false;
    return *this;
  }

  bool state() const
  {
    return m_state;
  }

  const bool& operator=(const bool& value)
  {
    if (value)
      on();
    else
      off();

    m_state = value;
    return value;
  }

  operator bool() const
  {
    return state();
  }

  virtual ~Relay() {}
};
} /* namespace device */
} /* namespace OwO */

#endif /* __RELAY_HPP__ */