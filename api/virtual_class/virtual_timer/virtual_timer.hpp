#ifndef __VIRTUAL_TIMER_HPP__
#define __VIRTUAL_TIMER_HPP__

#include "port_tim.h"
#include "port_os.h"
#include "object.hpp"
#include "signal.hpp"
#include "semaphore.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 定时器
namespace Timer
{
/// @brief 枚举 Timer 计数模式
enum Count_Mode
{
  UP,              /* 向上计数模式 */
  DOWN,            /* 向下计数模式 */
  CENTER_ALIGNED1, /* 中心对齐1模式 */
  CENTER_ALIGNED2, /* 中心对齐2模式 */
  CENTER_ALIGNED3, /* 中心对齐3模式 */
};

/// @brief 枚举 Timer 输出比较工作模式
enum Output_Compare_Mode
{
  TIMING,          /* 时间模式 */
  ACTIVE,          /* 激活模式 */
  INACTIVE,        /* 非激活模式 */
  TOGGLE,          /* 翻转触发模式 */
  PWM1,            /* PWM1模式 */
  PWM2,            /* PWM2模式 */
  FORCED_ACTIVE,   /* 强制激活模式 */
  FORCED_INACTIVE, /* 强制非激活模式 */
};
} /* namespace Timer */

/// @brief 名称空间 纯虚类
namespace virtual_class
{
/// @brief 纯虚类 定时器
class VTimer : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VTimer)
  NO_MOVE(VTimer)

private:
  /// @brief VTimer 定时器编号
  uint8_t                    m_timer_num;
  /// @brief VTimer 预分频系数
  uint16_t                   m_timer_psc;
  /// @brief VTimer 自动重装载值
  uint16_t                   m_timer_arr;
  /// @brief VTimer 软件定时器时间间隔
  uint32_t                   m_timer_period_ms;
  /// @brief VTimer 软件定时器句柄
  port_os_timer_t            m_timer_handle;
  /// @brief VTimer 软件定时器信号量
  system::kernel::Semaphore* m_timer_semaphore;

  static void timer_task_entry(void* arg);

protected:
  virtual void timer_task() = 0;

  uint32_t get_gpio_af_channel() const;
  bool     ic_analyze();
  bool     wait(uint32_t waiting_time = WAIT_FOREVER);
  float    get_frequency() const;
  float    get_duty_cycle() const;
  bool     set_duty_cycle(float duty_cycle);

  virtual bool open(uint8_t timer_num, uint16_t psc, uint16_t arr, Timer::Count_Mode counter_mode = Timer::UP, uint8_t division = 1);
  virtual bool open(uint8_t timer_num, uint16_t psc, uint16_t arr, uint8_t ic_channel_num, uint8_t ic_prescaler = 1, Timer::Count_Mode counter_mode = Timer::UP, uint8_t division = 1, bool auto_reload = false);
  virtual bool open(uint8_t timer_num, uint16_t psc, uint16_t arr, uint8_t oc_channel_num, Timer::Output_Compare_Mode oc_mode, Timer::Count_Mode counter_mode = Timer::UP, uint8_t division = 1, bool auto_reload = true);
  virtual bool open(const uint32_t period_ms, const bool auto_reload = true);

signals:
  system::Signal<system::Object*> signal_timer_alarm;

public:
  explicit VTimer(const std::string& name, Object* parent = nullptr) : system::Object(name, parent)
  {
    m_timer_num       = 0;
    m_timer_psc       = 0;
    m_timer_arr       = 0;
    m_timer_period_ms = 0;
    m_timer_handle    = nullptr;
    m_timer_semaphore = nullptr;
  }

  virtual bool start();
  virtual bool stop();
  virtual bool close();

  virtual ~VTimer();
};
} /* namespace virtual_class */
} /* namespace OwO */

#endif /* __VIRTUAL_TIMER_HPP__ */
