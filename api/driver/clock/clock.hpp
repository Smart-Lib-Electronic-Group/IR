#ifndef __CLOCK_HPP__
#define __CLOCK_HPP__

#include "virtual_timer.hpp"

namespace OwO
{
namespace driver
{
class Clock : public virtual_class::VTimer
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Clock)
  NO_MOVE(Clock)

protected:
  virtual void timer_task() override {}

public:
  explicit Clock(const std::string& name = "clock", Object* parent = nullptr) : virtual_class::VTimer(name, parent) {}
  virtual bool open(uint8_t timer_num, uint16_t psc, uint16_t arr, Timer::Count_Mode counter_mode = Timer::UP, uint8_t division = 1)
  {
    return VTimer::open(timer_num, psc, arr, counter_mode, division);
  }
  virtual bool open(const uint32_t period_ms, const bool auto_reload = true)
  {
    return VTimer::open(period_ms, auto_reload);
  }
  using VTimer::get_frequency;
  using VTimer::wait;

  operator float() const
  {
    return get_frequency();
  }

  operator bool()
  {
    return VTimer::wait();
  }

  virtual ~Clock() = default;
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __CLOCK_HPP__ */
