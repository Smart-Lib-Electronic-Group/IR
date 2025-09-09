/**
 * @file      led.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for LED device (LED与呼吸灯)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __LED_HPP__
#define __LED_HPP__

#include "pwm.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 类 Led - LED
class Led : public virtual_class::VGpio
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Led)
  NO_MOVE(Led)

  /// @brief LED 状态
  bool m_state = false;

private:
  /// @brief 处理中断回调函数 - 不实现
  virtual void irq_callback() {}

public:
  /**
   * @brief LED 构造函数
   *
   * @param name     名称
   * @param parent   父对象指针
   */
  explicit Led(const std::string& name = "Led", Object* parent = nullptr) : VGpio(name, parent) {}

  /**
   * @brief  LED 使能(初始化)
   *
   * @param  port   GPIO 端口
   * @param  pin    GPIO 引脚
   * @param  level  有效电平
   * @return true   成功
   * @return false  失败
   */
  virtual bool open(Gpio::Port port, uint8_t pin, Gpio::Level level = Gpio::LEVEL_HIGH)
  {
    return VGpio::open(port, pin, Gpio::OUT_PP, level);
  }

  /**
   * @brief  LED 开启
   *
   * @return Led& LED 对象自身
   */
  Led& on()
  {
    if (level() == Gpio::LEVEL_LOW)
      low();
    else if (level() == Gpio::LEVEL_HIGH)
      high();

    m_state = true;
    return *this;
  }

  /**
   * @brief  LED 关闭
   *
   * @return Led& LED 对象自身
   */
  Led& off()
  {
    if (level() == Gpio::LEVEL_LOW)
      high();
    else if (level() == Gpio::LEVEL_HIGH)
      low();

    m_state = false;
    return *this;
  }

  /**
   * @brief  LED 翻转
   *
   * @return Led& LED 对象自身
   */
  Led& toggle()
  {
    VGpio::toggle();
    m_state = !m_state;
    return *this;
  }

  /**
   * @brief  LED 状态
   *
   * @return true   开启
   * @return false  关闭
   */
  bool state() const
  {
    return m_state;
  }

  /**
   * @brief  LED 赋值运算符
   *
   * @param  value 值
   * @return bool  值
   */
  const bool& operator=(const bool& value)
  {
    if (value)
      on();
    else
      off();
    return value;
  }

  /**
   * @brief  LED 转换运算符
   *
   * @return bool  LED 状态
   */
  operator bool() const
  {
    return state();
  }

  /**
   * @brief LED 析构函数
   *
   */
  virtual ~Led() {}
};

/// @brief 类 BLed - 呼吸灯
class BLed : public driver::Pwm
{
  O_MEMORY
  O_OBJECT
  NO_COPY(BLed)
  NO_MOVE(BLed)

protected:
  // 禁用接口
  using Pwm::get_duty_cycle;
  using Pwm::set_duty_cycle;

public:
  /**
   * @brief 呼吸灯 构造函数
   *
   * @param name   名称
   * @param parent 父对象指针
   */
  BLed(const std::string& name = "BLed", Object* parent = nullptr) : Pwm(name, parent) {}

  /**
   * @brief  呼吸灯 设置亮度
   *
   * @param  lightness 亮度(0.0-100.0)
   * @return true      成功
   * @return false     失败
   */
  bool set_lightness(float lightness)
  {
    return set_duty_cycle(lightness);
  }

  /**
   * @brief  呼吸灯 获取亮度
   *
   * @return float 亮度(0.0-100.0)
   */
  float get_lightness() const
  {
    return get_duty_cycle();
  }

  /**
   * @brief  呼吸灯 赋值运算符
   * 
   * @param  value 亮度(0.0-100.0)
   * @return float 亮度(0.0-100.0)
   */
  float operator=(float value)
  {
    set_lightness(value);
    return value;
  }

  /**
   * @brief  呼吸灯 析构函数
   *
   */
  virtual ~BLed() {}
};
} /* namespace device */
} /* namespace OwO */

#endif /* __LED_HPP__ */