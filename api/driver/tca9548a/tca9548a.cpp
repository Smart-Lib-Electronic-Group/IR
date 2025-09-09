#include "tca9548a.hpp"

using namespace OwO::driver;
using namespace OwO::system;

O_METAOBJECT(Tca9548a, Object)

bool Tca9548a::set_channel(uint8_t channel)
{
  if (!m_is_open)
    return false;

  m_iic_setup();

  m_error_count = 0;
  m_tmp         = 0;

  if (channel < 8)
  {
    while (m_tmp != (1 << channel))
    {
      if (m_error_count > 10)
      {
        reset();
        return false;
      }

      m_tmp = 1 << channel;
      m_bus->send(&m_tmp, 1);
      m_bus->recv(&m_tmp, 1);
      m_error_count++;
    }

    if (m_error_count <= 10)
      return true;
  }

  return false;
}

uint8_t Tca9548a::get_channel()
{
  if (!m_is_open)
    return 0;

  m_iic_setup();

  m_tmp = 0;
  m_bus->recv(&m_tmp, 1);

  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if (m_tmp & (1 << i))
    {
      break;
    }
  }

  return i;
}
