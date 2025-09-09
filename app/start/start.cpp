#include "start.h"
#include "port_net_init.h"

#include "led.hpp"
#include "clock.hpp"

#include "main_app.hpp"

using namespace OwO;
using namespace device;
using namespace system;
using namespace kernel;

class led_clock : public driver::Clock
{
  O_MEMORY
  O_OBJECT

  friend void v_port_net_link_up_callback();
  friend void v_port_net_link_down_callback();

private:
  BLed*        m_led;
  bool         m_state;
  bool         m_mode;
  float        m_brightness;
  virtual void timer_task() override
  {
    m_brightness += 2.0f;
    if (m_brightness > 100.0f)
    {
      m_brightness = 0.0f;
      m_state      = !m_state;
      m_led->set_level(m_state);
    }

    if (m_mode)
    {
      m_led->set_lightness(m_brightness);
    }
    else
    {
      m_led->set_lightness(0.0f);
    }
  }

public:
  explicit led_clock(const std::string& name, BLed* led) : Clock(name, led)
  {
    m_led  = led;
    m_mode = false;
  }

  virtual ~led_clock() {}
};

O_METAOBJECT(led_clock, driver::Clock)

led_clock* net_state_check = nullptr;

void v_port_net_link_up_callback()
{
  if (net_state_check != nullptr)
  {
    net_state_check->m_mode = true;
  }
}

void v_port_net_link_down_callback()
{
  if (net_state_check != nullptr)
  {
    net_state_check->m_mode = false;
  }
}

BLed*           led = nullptr;
main::main_app* app = nullptr;

void start_app(void*)
{
  led             = new BLed("led", nullptr);
  net_state_check = new led_clock("led_clock", led);
  app             = new main::main_app("main_app", nullptr);
  led->open(Gpio::PB, 1, 3, 4);
  net_state_check->open(10);
  net_state_check->start();
  app->start();
  v_port_os_thread_delete(nullptr);
}