#include "input_compare.hpp"
#include "thread.hpp"

using namespace OwO;
using namespace driver;
using namespace virtual_class;
using namespace system;
using namespace kernel;

O_METAOBJECT(Input_Compare, VTimer)

bool Input_Compare::open(Gpio::Port port, uint8_t pin, uint8_t timer_num, uint8_t channel, uint16_t psc, uint16_t arr, Gpio::Level level, uint8_t prescaler)
{
  if (false == VTimer::open(timer_num, psc, arr, channel, prescaler))
    return false;

  if (false == m_gpio->open(port, pin, get_gpio_af_channel(), level, Gpio::AF_OD))
  {
    close();
    return false;
  }

  start();
  return true;
}

bool Input_Compare::close()
{
  stop();

  bool ret  = true;
  ret      &= VTimer::close();
  ret      &= m_gpio->close();
  return ret;
}

bool Input_Compare::analyze(uint32_t delay_time, uint32_t waiting_time, bool is_stop)
{
  if (wait(waiting_time))
  {
    if (is_stop)
      stop();

    VTimer::ic_analyze();
    Thread::msleep(delay_time);

    if (is_stop)
      start();

    return true;
  }
  else
  {
    Thread::msleep(delay_time);
    return false;
  }
}

bool Input_Compare::analyze()
{
  if (wait(0))
  {
    VTimer::ic_analyze();
    return true;
  }
  else
  {
    return false;
  }
}

float Input_Compare::get_duty_cycle(float& duty_cycle) const
{
  if (Gpio::LEVEL_HIGH == m_gpio->level())
    duty_cycle = VTimer::get_duty_cycle();
  else
    duty_cycle = 100.0f - VTimer::get_duty_cycle();

  return duty_cycle;
}

float Input_Compare::get_duty_cycle() const
{
  if (Gpio::LEVEL_HIGH == m_gpio->level())
    return VTimer::get_duty_cycle();
  else
    return 100.0f - VTimer::get_duty_cycle();
}

float Input_Compare::get_frequency(float& frequency) const
{
  frequency = VTimer::get_frequency();
  return frequency;
}

float Input_Compare::get_frequency() const
{
  return VTimer::get_frequency();
}
