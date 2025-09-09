#ifndef __PWM_HPP__
#define __PWM_HPP__

#include "virtual_timer.hpp"
#include "virtual_gpio.hpp"

namespace OwO
{
namespace driver
{
class Pwm : public virtual_class::VTimer
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Pwm)
  NO_MOVE(Pwm)

private:
  /// @brief PWM 输出引脚
  Class::io* m_gpio;
  using VTimer::signal_timer_alarm;

protected:
  virtual void timer_task() override {}

public:
  explicit Pwm(const std::string& name, Object* parent = nullptr) : virtual_class::VTimer(name, parent)
  {
    m_gpio = new Class::io(name + "_gpio", this);
  }
  virtual bool open(Gpio::Port port, uint8_t pin, uint8_t timer_num, uint8_t channel, uint16_t psc = 50 - 1, uint16_t arr = 90 - 1, Gpio::Level level = Gpio::LEVEL_HIGH);
  virtual bool close() override;
  bool         set_duty_cycle(float duty_cycle);
  float        get_duty_cycle() const;

  bool set_level(Gpio::Level level)
  {
    return m_gpio->set_level(level);
  }

  bool set_level(bool level)
  {
    return set_level(level ? Gpio::LEVEL_HIGH : Gpio::LEVEL_LOW);
  }

  float operator=(float duty_cycle)
  {
    return set_duty_cycle(duty_cycle) ? duty_cycle : 0.0f;
  }

  operator float() const
  {
    return get_duty_cycle();
  }

  virtual ~Pwm()
  {
    close();
    delete m_gpio;
  }
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __PWM_HPP__ */
