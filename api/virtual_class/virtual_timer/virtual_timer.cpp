#include "virtual_timer.hpp"

using namespace OwO;
using namespace system;
using namespace Timer;
using namespace virtual_class;

O_METAOBJECT(VTimer, Object)

void VTimer::timer_task_entry(void* arg)
{
  VTimer* timer = static_cast<VTimer*>(pv_port_os_timer_get_arg(arg));
  if (NULL != timer)
  {
    timer->m_timer_semaphore->release();
    timer->timer_task();
    timer->signal_timer_alarm(timer);
  }
}

uint32_t VTimer::get_gpio_af_channel() const
{
  return ul_port_timer_get_gpio_af(m_timer_num);
}

bool VTimer::ic_analyze()
{
  return SUCESS == e_port_timer_analyze(m_timer_num, m_timer_psc, m_timer_arr);
}

bool VTimer::wait(uint32_t waiting_time)
{
  if (!is_open())
    return false;

  if (m_timer_semaphore)
    return m_timer_semaphore->try_acquire(waiting_time);
  else
    return b_port_timer_wait_semaphore(m_timer_num, waiting_time);
}

float VTimer::get_frequency() const
{
  if (m_timer_handle)
    return 1000.0f / m_timer_period_ms;
  else
    return f_port_timer_get_frequency(m_timer_num);
}

float VTimer::get_duty_cycle() const
{
  if (!is_open())
    return 0.0;

  return f_port_timer_get_duty_cycle(m_timer_num);
}

bool VTimer::set_duty_cycle(float duty_cycle)
{
  if (!is_open())
    return false;

  return SUCESS == e_port_timer_set_duty_cycle(m_timer_num, m_timer_arr, duty_cycle);
}

bool VTimer::open(uint8_t timer_num, uint16_t psc, uint16_t arr, Count_Mode counter_mode, uint8_t division)
{
  if (is_open())
    return false;

  m_timer_num = timer_num;
  m_timer_psc = psc;
  m_timer_arr = arr;

  port_timer_callback_t cb_t;
  cb_t.function = timer_task_entry;
  cb_t.arg      = static_cast<void*>(this);

  if (SUCESS == e_port_timer_normal_init(m_timer_num, m_timer_psc, m_timer_arr, static_cast<port_timer_counter_mode_e>(counter_mode), division, &cb_t))
  {
    m_is_open = true;
    return true;
  }
  else
    return false;
}

bool VTimer::open(uint8_t timer_num, uint16_t psc, uint16_t arr, uint8_t ic_channel_num, uint8_t ic_prescaler, Count_Mode counter_mode, uint8_t division, bool auto_reload)
{
  if (is_open())
    return false;

  m_timer_num = timer_num;
  m_timer_psc = psc;
  m_timer_arr = arr;

  if (SUCESS == e_port_timer_ic_init(m_timer_num, m_timer_psc, m_timer_arr, ic_channel_num, ic_prescaler, static_cast<port_timer_counter_mode_e>(counter_mode), division, auto_reload))
  {
    m_is_open = true;
    return true;
  }
  else
    return false;
}

bool VTimer::open(uint8_t timer_num, uint16_t psc, uint16_t arr, uint8_t oc_channel_num, Output_Compare_Mode oc_mode, Count_Mode counter_mode, uint8_t division, bool auto_reload)
{
  if (is_open())
    return false;

  m_timer_num = timer_num;
  m_timer_psc = psc;
  m_timer_arr = arr;

  if (SUCESS == e_port_timer_oc_init(m_timer_num, m_timer_psc, m_timer_arr, oc_channel_num, static_cast<port_timer_oc_mode_e>(oc_mode), static_cast<port_timer_counter_mode_e>(counter_mode), division, auto_reload))
  {
    m_is_open = true;
    return true;
  }
  else
    return false;
}

bool VTimer::open(const uint32_t period_ms, const bool auto_reload)
{
  if (is_open())
    return false;

  m_timer_semaphore = new system::kernel::Semaphore(1, 1);
  m_timer_semaphore->try_acquire();

  m_timer_period_ms = period_ms;
  m_timer_handle    = pt_port_os_timer_create(name().c_str(), m_timer_period_ms, auto_reload, timer_task_entry, this);
  if (NULL != m_timer_handle)
  {
    m_is_open = true;
    return true;
  }
  else
    return false;
}

bool VTimer::start()
{
  if (!is_open())
    return false;

  if (m_timer_handle)
    return b_port_os_timer_start(m_timer_handle, m_timer_period_ms);
  else
    return SUCESS == e_port_timer_start(m_timer_num);
}

bool VTimer::stop()
{
  if (!is_open())
    return false;

  if (m_timer_semaphore)
    m_timer_semaphore->release();

  if (m_timer_handle)
    return b_port_os_timer_stop(m_timer_handle);
  else
    return SUCESS == e_port_timer_stop(m_timer_num);
}

bool VTimer::close()
{
  if (!is_open())
    return true;

  bool ret = true;

  if (m_timer_handle)
    ret &= b_port_os_timer_delete(m_timer_handle);
  else
    ret &= SUCESS == e_port_timer_deinit(m_timer_num);

  if (m_timer_semaphore)
    delete m_timer_semaphore;

  if (ret)
    m_is_open = false;

  return ret;
}

VTimer::~VTimer()
{
  close();
}
