#include "pwm.hpp"

using namespace OwO;
using namespace driver;
using namespace virtual_class;

O_METAOBJECT(Pwm, VTimer)

bool Pwm::open(Gpio::Port port, uint8_t pin, uint8_t timer_num, uint8_t channel, uint16_t psc, uint16_t arr, Gpio::Level level)
{
  if (false == VTimer::open(timer_num, psc, arr, channel, Timer::PWM1))
    return false;

  if (false == m_gpio->open(port, pin, get_gpio_af_channel(), level))
  {
    close();
    return false;
  }

  start();
  return true;
}

bool Pwm::close()
{
  stop();

  bool ret  = true;
  ret      &= VTimer::close();
  ret      &= m_gpio->close();
  return ret;
}

bool Pwm::set_duty_cycle(float duty_cycle)
{
  if (Gpio::LEVEL_HIGH == m_gpio->level())
    return VTimer::set_duty_cycle(duty_cycle);
  else if (Gpio::LEVEL_LOW == m_gpio->level())
    return VTimer::set_duty_cycle(100.0f - duty_cycle);
  else
    return false;
}

float Pwm::get_duty_cycle() const
{
  if (Gpio::LEVEL_HIGH == m_gpio->level())
    return VTimer::get_duty_cycle();
  else if (Gpio::LEVEL_LOW == m_gpio->level())
    return 100.0 - VTimer::get_duty_cycle();
  else
    return 0.0;
}
