#ifndef __INPUT_COMPARE_HPP__
#define __INPUT_COMPARE_HPP__

#include "virtual_timer.hpp"
#include "virtual_gpio.hpp"

namespace OwO
{
namespace driver
{
class Input_Compare : public virtual_class::VTimer
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Input_Compare)
  NO_MOVE(Input_Compare)
private:
  /// @brief 输入比较 输入引脚
  Class::io* m_gpio;

protected:
  virtual void timer_task() override {}

  using virtual_class::VTimer::open;

public:
  explicit Input_Compare(const std::string& name = "input_compare", Object* parent = nullptr) : virtual_class::VTimer(name, parent)
  {
    m_gpio = new Class::io(name + "_gpio", this);
  }

  virtual bool open(Gpio::Port port, uint8_t pin, uint8_t timer_num, uint8_t channel, uint16_t psc = 90 - 1, uint16_t arr = 0xffff, Gpio::Level level = Gpio::LEVEL_HIGH, uint8_t prescaler = 1);

  virtual bool close();

  bool  analyze(uint32_t delay_time, uint32_t waiting_time = WAIT_FOREVER, bool is_stop = false);
  bool  analyze();
  float get_duty_cycle(float& duty_cycle) const;
  float get_duty_cycle() const;
  float get_frequency(float& frequency) const;
  float get_frequency() const;

  ~Input_Compare()
  {
    delete m_gpio;
  }
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __INPUT_COMPARE_HPP__ */
