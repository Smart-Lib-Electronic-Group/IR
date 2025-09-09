/**
 * @file      key.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for Key device (按键)
 * @version   v1.0.0
 * @date      2025-06-25
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __KEY_HPP__
#define __KEY_HPP__

#include "virtual_gpio.hpp"
#include "signal.hpp"
#include "clock.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
class Key;

/// @brief 按键类型
enum class Key_Type
{
  Rising = 1, /* 上升沿触发 */
  Falling,    /* 下降沿触发 */
  Both,       /* 双边沿触发 */
  Press,      /* 按下触发 */
  Long_Press, /* 长按触发 */
};

/// @brief 按键状态机
class Key_State_Machine : public driver::Clock
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Key_State_Machine)
  NO_MOVE(Key_State_Machine)

  friend class Key;

private:
  /// @brief 按键状态机 互斥锁
  mutable system::kernel::Mutex    m_mutex;
  /// @brief 按键列表
  std::list<virtual_class::VGpio*> m_keys;

  /**
   * @brief 按键状态机 定时器任务
   *
   */
  virtual void timer_task() override
  {
    system::kernel::Mutex_Guard lock(m_mutex);
    if (!m_keys.empty())
    {
      for (auto key : m_keys)
      {
        key->irq_callback();
      }
    }
  }

protected:
  /**
   * @brief 构造函数
   *
   * @param name 名称
   */
  Key_State_Machine(const std::string& name = "Key_State_Machine") : Clock(name)
  {
    Clock::open(10);
  }

  /**
   * @brief  获取按键状态机实例
   *
   * @return Key_State_Machine& 按键状态机实例
   */
  static Key_State_Machine& instance()
  {
    static Key_State_Machine instance;
    return instance;
  }

  /**
   * @brief 增加按键
   *
   * @param key 按键实例指针
   */
  void add_key(virtual_class::VGpio* key)
  {
    system::kernel::Mutex_Guard lock(m_mutex);
    m_keys.push_back(key);

    if (m_keys.size() == 1)
      Clock::start();
  }

  /**
   * @brief 移除按键
   *
   * @param key 按键实例指针
   */
  void remove_key(virtual_class::VGpio* key)
  {
    system::kernel::Mutex_Guard lock(m_mutex);
    m_keys.remove(key);

    if (m_keys.empty())
      Clock::stop();
  }

  /**
   * @brief 析构函数
   *
   */
  virtual ~Key_State_Machine()
  {
    Clock::stop();
    for (auto key : m_keys)
      remove_key(key);
  }
};

/// @brief 类 按键
class Key : public virtual_class::VGpio
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Key)
  NO_MOVE(Key)

  friend class Key_State_Machine;

private:
  enum class key_state_e : uint8_t
  {
    INIT = 0,
    IDLE,
    PRESS_DELAY,
    PRESS_HOLD,
    RELEASE_DELAY,
  };

  Key_Type    m_type;
  key_state_e m_state;
  uint32_t    m_long_press_time;
  uint32_t    m_long_press_start_time;
  bool        m_last_gpio_state;
  bool        m_new_gpio_state;
  bool        m_flag;

protected:
  void rising_type_callback()
  {
    if (key_state_e::INIT == m_state)
    {
      m_last_gpio_state = m_new_gpio_state = VGpio::read();
      m_state                              = key_state_e::IDLE;
    }
    else if (key_state_e::IDLE == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
        m_flag = !m_flag;
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
        m_flag = !m_flag;

      m_last_gpio_state = m_new_gpio_state;
    }
  }

  void falling_type_callback()
  {
    if (key_state_e::INIT == m_state)
    {
      m_last_gpio_state = m_new_gpio_state = VGpio::read();
      m_state                              = key_state_e::IDLE;
    }
    else if (key_state_e::IDLE == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 1 && m_new_gpio_state == 0)
        m_flag = !m_flag;
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 0 && m_new_gpio_state == 1)
        m_flag = !m_flag;

      m_last_gpio_state = m_new_gpio_state;
    }
  }

  void both_type_callback()
  {
    if (level() == Gpio::LEVEL_HIGH)
      m_flag = VGpio::read();
    else if (level() == Gpio::LEVEL_LOW)
      m_flag = !VGpio::read();
  }

  void press_type_callback()
  {
    if (key_state_e::INIT == m_state)
    {
      m_last_gpio_state = m_new_gpio_state = VGpio::read();
      m_state                              = key_state_e::IDLE;
    }
    else if (key_state_e::IDLE == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
        m_state = key_state_e::PRESS_DELAY;
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
        m_state = key_state_e::PRESS_DELAY;
    }
    else if (key_state_e::PRESS_DELAY == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
        m_state = key_state_e::PRESS_HOLD;
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
        m_state = key_state_e::PRESS_HOLD;
      else
        m_state = key_state_e::IDLE;
    }
    else if (key_state_e::PRESS_HOLD == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
      {
        m_flag            = true;
        m_state           = key_state_e::RELEASE_DELAY;
        m_last_gpio_state = m_new_gpio_state;
      }
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
      {
        m_flag            = true;
        m_state           = key_state_e::RELEASE_DELAY;
        m_last_gpio_state = m_new_gpio_state;
      }
      else
        m_state = key_state_e::IDLE;
    }
    else if (key_state_e::RELEASE_DELAY == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 1 && m_new_gpio_state == 0)
      {
        m_flag            = false;
        m_state           = key_state_e::IDLE;
        m_last_gpio_state = m_new_gpio_state;
      }
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 0 && m_new_gpio_state == 1)
      {
        m_flag            = false;
        m_state           = key_state_e::IDLE;
        m_last_gpio_state = m_new_gpio_state;
      }
    }
  }

  void long_press_type_callback()
  {
    if (key_state_e::INIT == m_state)
    {
      m_last_gpio_state = m_new_gpio_state = VGpio::read();
      m_state                              = key_state_e::IDLE;
    }
    else if (key_state_e::IDLE == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
        m_state = key_state_e::PRESS_DELAY;
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
        m_state = key_state_e::PRESS_DELAY;
    }
    else if (key_state_e::PRESS_DELAY == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
      {
        m_long_press_start_time = ul_port_os_get_tick_count();
        m_state                 = key_state_e::PRESS_HOLD;
      }
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
      {
        m_long_press_start_time = ul_port_os_get_tick_count();
        m_state                 = key_state_e::PRESS_HOLD;
      }
      else
        m_state = key_state_e::IDLE;
    }
    else if (key_state_e::PRESS_HOLD == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 0 && m_new_gpio_state == 1)
      {
        if (ul_port_os_get_tick_count() - m_long_press_start_time > m_long_press_time)
        {
          m_flag            = true;
          m_state           = key_state_e::RELEASE_DELAY;
          m_last_gpio_state = m_new_gpio_state;
        }
      }
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 1 && m_new_gpio_state == 0)
      {
        if (ul_port_os_get_tick_count() - m_long_press_start_time > m_long_press_time)
        {
          m_flag            = true;
          m_state           = key_state_e::RELEASE_DELAY;
          m_last_gpio_state = m_new_gpio_state;
        }
      }
      else
        m_state = key_state_e::IDLE;
    }
    else if (key_state_e::RELEASE_DELAY == m_state)
    {
      m_new_gpio_state = VGpio::read();

      if (level() == Gpio::LEVEL_HIGH && m_last_gpio_state == 1 && m_new_gpio_state == 0)
      {
        m_flag            = false;
        m_state           = key_state_e::IDLE;
        m_last_gpio_state = m_new_gpio_state;
      }
      else if (level() == Gpio::LEVEL_LOW && m_last_gpio_state == 0 && m_new_gpio_state == 1)
      {
        m_flag            = false;
        m_state           = key_state_e::IDLE;
        m_last_gpio_state = m_new_gpio_state;
      }
    }
  }

  void irq_callback() override
  {
    switch (m_type)
    {
      case Key_Type::Rising :
        rising_type_callback();
        break;
      case Key_Type::Falling :
        falling_type_callback();
        break;
      case Key_Type::Both :
        both_type_callback();
        break;
      case Key_Type::Press :
        press_type_callback();
        break;
      case Key_Type::Long_Press :
        long_press_type_callback();
        break;
      default :
        break;
    }
  }

public:
  Key(const std::string& name = "Key", Object* parent = nullptr) : VGpio(name, parent)
  {
    m_type                  = Key_Type::Falling;
    m_state                 = key_state_e::INIT;
    m_long_press_time       = 1000;
    m_long_press_start_time = 0;
    m_last_gpio_state       = false;
    m_new_gpio_state        = false;
    m_flag                  = false;
  }

  virtual bool open(Gpio::Port port, uint8_t pin, Key_Type type = Key_Type::Rising, Gpio::Level level = Gpio::LEVEL_HIGH)
  {
    Gpio::Event event = Gpio::Event::EVENT_BOTH;

    if (type == Key_Type::Rising)
      event = Gpio::Event::EVENT_RISING;
    else if (type == Key_Type::Falling)
      event = Gpio::Event::EVENT_FALLING;

    if (VGpio::open(port, pin, Gpio::Mode::IN, level, event, (Gpio::LEVEL_HIGH == level) ? Gpio::PULL_DOWN : Gpio::PULL_UP))
    {
      m_flag  = false;
      m_type  = type;
      m_state = key_state_e::INIT;
      Key_State_Machine::instance().add_key(this);
      return true;
    }
    return false;
  }

  virtual bool open(Gpio::Port port, uint8_t pin, Key_Type type, uint32_t long_press_time, Gpio::Level level = Gpio::LEVEL_HIGH)
  {
    if (VGpio::open(port, pin, Gpio::Mode::IN, level, Gpio::Event::EVENT_BOTH, (Gpio::LEVEL_HIGH == level) ? Gpio::PULL_DOWN : Gpio::PULL_UP))
    {
      m_flag            = false;
      m_type            = type;
      m_state           = key_state_e::INIT;
      m_long_press_time = long_press_time;
      Key_State_Machine::instance().add_key(this);
      return true;
    }
    return false;
  }

  bool read() const
  {
    return m_flag;
  }

  operator bool() const
  {
    return m_flag;
  }

  bool operator==(bool flag) const
  {
    return m_flag == flag;
  }

  void clear()
  {
    m_flag = false;
  }

  void set_type(Key_Type type, uint32_t long_press_time = 1000)
  {
    Gpio::Event event = Gpio::Event::EVENT_BOTH;

    if (type == Key_Type::Rising)
      event = Gpio::Event::EVENT_RISING;
    else if (type == Key_Type::Falling)
      event = Gpio::Event::EVENT_FALLING;

    Key_State_Machine::instance().remove_key(this);

    if (type == Key_Type::Long_Press)
      m_long_press_time = long_press_time;

    m_flag  = false;
    m_type  = type;
    m_state = key_state_e::INIT;

    set_event(event);

    Key_State_Machine::instance().add_key(this);
  }

  bool close() override
  {
    Key_State_Machine::instance().remove_key(this);
    return VGpio::close();
  }

  virtual ~Key()
  {
    close();
  }
};
} /* namespace device */
} /* namespace OwO */

#endif /* __KEY_HPP__ */