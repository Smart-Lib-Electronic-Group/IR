#include "virtual_gpio.hpp"

using namespace OwO;
using namespace virtual_class;
using namespace Gpio;
using namespace system;
using namespace Class;

O_METAOBJECT(VGpio, Object)
O_METAOBJECT(io, VGpio)

void VGpio::m_init()
{
  if (true == m_irq_enabled)
  {
    port_gpio_callback_t cb_t;
    cb_t.function = m_irq;
    cb_t.arg      = static_cast<void*>(this);
    v_port_gpio_init(m_port, m_pin, static_cast<port_gpio_mode_e>(m_mode), static_cast<port_gpio_event_e>(m_event), static_cast<port_gpio_pull_e>(m_pull), static_cast<port_gpio_speed_e>(m_speed), &cb_t);
  }
  else
  {
    v_port_gpio_init(m_port, m_pin, static_cast<port_gpio_mode_e>(m_mode), static_cast<port_gpio_event_e>(m_event), static_cast<port_gpio_pull_e>(m_pull), static_cast<port_gpio_speed_e>(m_speed), NULL);
  }
}

Mode VGpio::set_mode(Mode mode)
{
  if (m_mode != mode)
  {
    if (true == m_irq_enabled)
      disable_irq();

    m_mode = mode;

    if (IN_IT == m_mode || IN_EVENT == m_mode)
      m_irq_enabled = true;

    m_init();
  }
  return m_mode;
}

Level VGpio::set_level(Level level)
{
  m_level = level;
  return m_level;
}

Pull VGpio::set_pull(Pull pull)
{
  if (m_pull != pull)
  {
    m_pull = pull;

    m_init();
  }

  return m_pull;
}

Speed VGpio::set_speed(Speed speed)
{
  if (m_speed != speed)
  {
    m_speed = speed;

    m_init();
  }

  return m_speed;
}

Event VGpio::set_event(Event event)
{
  if (m_event != event)
  {
    m_event = event;

    m_init();
  }

  return m_event;
}

Mode VGpio::change_mode(Mode mode)
{
  if (m_mode != mode)
  {
    m_mode = mode;

    if (IN_IT == m_mode || IN_EVENT == m_mode)
      m_irq_enabled = true;

    m_init();
  }

  return m_mode;
}

bool VGpio::open(Gpio::Port port, uint8_t pin, uint32_t af_channel, Gpio::Level level, Gpio::Mode mode, Gpio::Pull pull, Gpio::Speed speed)
{
  if (is_open())
    return false;

  m_port        = port;
  m_pin         = pin;
  m_mode        = mode;
  m_level       = level;
  m_pull        = pull;
  m_speed       = speed;
  m_irq_enabled = false;

  v_port_gpio_af_init(m_port, m_pin, static_cast<port_gpio_mode_e>(m_mode), static_cast<port_gpio_pull_e>(m_pull), static_cast<port_gpio_speed_e>(m_speed), af_channel);

  return true;
}

bool VGpio::open(Gpio::Port port, uint8_t pin, Gpio::Mode mode, Gpio::Level level, Gpio::Event event, Gpio::Pull pull, Gpio::Speed speed)
{
  if (is_open())
    return false;

  m_port        = port;
  m_pin         = pin;
  m_mode        = mode;
  m_level       = level;
  m_event       = event;
  m_pull        = pull;
  m_speed       = speed;
  m_irq_enabled = false;

  if (IN_IT == m_mode || IN_EVENT == m_mode)
    m_irq_enabled = true;

  m_init();

  return true;
}

bool VGpio::close()
{
  if (!is_open())
    return true;

  if (true == m_irq_enabled)
    disable_irq();

  v_port_gpio_deinit(m_port, m_pin, static_cast<port_gpio_mode_e>(m_mode));

  return true;
}

VGpio::~VGpio()
{
  close();
}
