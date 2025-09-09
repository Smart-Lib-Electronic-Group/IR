#ifndef __VIRTUAL_GPIO_HPP__
#define __VIRTUAL_GPIO_HPP__

#include "object.hpp"
#include "port_gpio.h"

/// @brief 名称空间 库名
namespace OwO
{
namespace device
{
class Key_State_Machine;
} /* namespace device */

namespace Gpio
{
/// @brief 枚举 GPIO 端口
enum Port
{
  PA = 'A', /* GPIOA */
  PB,       /* GPIOB */
  PC,       /* GPIOC */
  PD,       /* GPIOD */
  PE,       /* GPIOE */
  PF,       /* GPIOF */
  PG,       /* GPIOG */
  PH,       /* GPIOH */
  PI,       /* GPIOI */
  PJ,       /* GPIOJ */
};

/// @brief 枚举 GPIO 工作模式
enum Mode
{
  IN,        /* GPIO 普通输入模式 */
  IN_IT,     /* GPIO 中断输入模式 */
  IN_EVENT,  /* GPIO 事件输入模式 */
  IN_ANALOG, /* GPIO 模拟输入模式 */
  OUT_PP,    /* GPIO 推挽输出模式 */
  OUT_OD,    /* GPIO 开漏输出模式 */
  AF_PP,     /* GPIO 推挽复用模式 */
  AF_OD,     /* GPIO 开漏复用模式 */
};

/// @brief 枚举 GPIO 电平
enum Level
{
  LEVEL_LOW,  /* GPIO 低电平有效 */
  LEVEL_HIGH, /* GPIO 高电平有效 */
};

/// @brief 枚举 GPIO 默认电压
enum Pull
{
  PULL_NONE = 0, /* GPIO 引脚浮空 */
  PULL_UP,       /* GPIO 引脚上拉 */
  PULL_DOWN,     /* GPIO 引脚下拉 */
};

/// @brief 枚举 GPIO 速度
enum Speed
{
  SPEED_LOW       = 0, /* GPIO 低速 */
  SPEED_MEDIUM    = 1, /* GPIO 中速 */
  SPEED_HIGH      = 2, /* GPIO 高速 */
  SPEED_VERY_HIGH = 3, /* GPIO 超高速 */
};

/// @brief 枚举 GPIO 事件检测模式
enum Event
{
  EVENT_RISING = 1, /* GPIO 检测上升沿 */
  EVENT_FALLING,    /* GPIO 检测下降沿 */
  EVENT_BOTH,       /* GPIO 检测上升沿与下降沿 */
};
}   // namespace Gpio

/// @brief 名称空间 纯虚类
namespace virtual_class
{
class VGpio : public OwO::system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VGpio)
  NO_MOVE(VGpio)

private:
  uint8_t     m_port;
  uint8_t     m_pin;
  Gpio::Mode  m_mode;
  Gpio::Level m_level;
  Gpio::Pull  m_pull;
  Gpio::Speed m_speed;
  Gpio::Event m_event;
  bool        m_irq_enabled;

  static void m_irq(void* arg)
  {
    static_cast<VGpio*>(arg)->irq_callback();
  };

  void m_init();

protected:
  friend device::Key_State_Machine;

  virtual void irq_callback() = 0;

  void enable_irq()
  {
    m_irq_enabled = true;
    v_port_gpio_nvic_enable(m_pin);
  }

  void disable_irq()
  {
    m_irq_enabled = false;
    v_port_gpio_nvic_disable(m_pin);
  }

  void high()
  {
    v_port_gpio_write(m_port, m_pin, true);
  }

  void low()
  {
    v_port_gpio_write(m_port, m_pin, false);
  }

  void toggle()
  {
    v_port_gpio_toggle(m_port, m_pin);
  }

  bool write(bool val)
  {
    v_port_gpio_write(m_port, m_pin, val);
    return val;
  }

  void read(bool& val)
  {
    val = b_port_gpio_read(m_port, m_pin);
  }

  bool read() const
  {
    return b_port_gpio_read(m_port, m_pin);
  }

  Gpio::Level level() const
  {
    return m_level;
  }

  Gpio::Event event() const
  {
    return m_event;
  }

  Gpio::Mode  set_mode(Gpio::Mode mode);
  Gpio::Level set_level(Gpio::Level level);
  Gpio::Pull  set_pull(Gpio::Pull pull);
  Gpio::Speed set_speed(Gpio::Speed speed);
  Gpio::Event set_event(Gpio::Event event);
  Gpio::Mode  change_mode(Gpio::Mode mode);

  bool open(Gpio::Port port, uint8_t pin, uint32_t af_channel, Gpio::Level level = Gpio::LEVEL_HIGH, Gpio::Mode mode = Gpio::AF_PP, Gpio::Pull pull = Gpio::PULL_UP, Gpio::Speed speed = Gpio::SPEED_VERY_HIGH);

public:
  explicit VGpio(const std::string& name, Object* parent) : OwO::system::Object(name, parent)
  {
    m_port        = 0;
    m_pin         = 0;
    m_mode        = Gpio::Mode::IN;
    m_level       = Gpio::Level::LEVEL_HIGH;
    m_pull        = Gpio::Pull::PULL_NONE;
    m_speed       = Gpio::Speed::SPEED_MEDIUM;
    m_event       = Gpio::Event::EVENT_BOTH;
    m_irq_enabled = false;
  }
  virtual bool open(Gpio::Port port, uint8_t pin, Gpio::Mode mode = Gpio::OUT_PP, Gpio::Level level = Gpio::LEVEL_HIGH, Gpio::Event event = Gpio::EVENT_BOTH, Gpio::Pull pull = Gpio::PULL_UP, Gpio::Speed speed = Gpio::SPEED_VERY_HIGH);
  virtual bool close();
  virtual ~VGpio();
};

class VSoft_IIC;
class VSpi;
} /* namespace virtual_class */

namespace driver
{
class Pwm;
class RS485;
class Input_Compare;
class Tca9548a;
} /* namespace driver */

namespace device
{
class IR;
} /* namespace device */

namespace Class
{
class io : public virtual_class::VGpio
{
  O_MEMORY
  O_OBJECT
  NO_COPY(io)
  NO_MOVE(io)

  friend class device::IR;
  friend class driver::Pwm;
  friend class driver::RS485;
  friend class driver::Input_Compare;
  friend class driver::Tca9548a;
  friend class virtual_class::VSoft_IIC;
  friend class virtual_class::VSpi;

protected:
  virtual void irq_callback() override {}

  Gpio::Level level()
  {
    return VGpio::level();
  }

  void high()
  {
    VGpio::high();
  }

  void low()
  {
    VGpio::low();
  }

  void write(bool val)
  {
    VGpio::write(val);
  }

  bool read()
  {
    return VGpio::read();
  }

  Gpio::Mode set_mode(Gpio::Mode mode)
  {
    return VGpio::set_mode(mode);
  }

  bool open(Gpio::Port port, uint8_t pin, uint32_t af_channel, Gpio::Level level = Gpio::LEVEL_HIGH, Gpio::Mode mode = Gpio::AF_PP, Gpio::Pull pull = Gpio::PULL_UP, Gpio::Speed speed = Gpio::SPEED_VERY_HIGH)
  {
    return VGpio::open(port, pin, af_channel, level, mode, pull, speed);
  }

  virtual bool open(Gpio::Port port, uint8_t pin, Gpio::Mode mode = Gpio::OUT_PP, Gpio::Level level = Gpio::LEVEL_HIGH, Gpio::Event event = Gpio::EVENT_BOTH, Gpio::Pull pull = Gpio::PULL_UP, Gpio::Speed speed = Gpio::SPEED_HIGH)
  {
    return VGpio::open(port, pin, mode, level, event, pull, speed);
  }

public:
  explicit io(const std::string& name, Object* parent) : virtual_class::VGpio(name, parent) {}
  virtual ~io() {}
};
} /* namespace Class */
} /* namespace OwO */

#endif /* __VIRTUAL_GPIO_HPP__ */
