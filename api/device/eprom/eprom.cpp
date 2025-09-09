/**
 * @file      eprom.cpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for EPROM device
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "eprom.hpp"

using namespace OwO::device;
using namespace OwO::system;

O_METAOBJECT(EPROM, ORom)

bool EPROM::check(bool auto_reload)
{
  uint32_t position = 8 * 16 * m_type - 1;
  char     flag     = 0;
  ORom::read(position, &flag, 1);

  if (flag == EPROM_DEF_CHECK_FLAG)
    return false;
  else
  {
    if (auto_reload)
    {
      flag = EPROM_DEF_CHECK_FLAG;
      ORom::erase(0, 8 * 16 * m_type, EPROM_DEF_ERASE_BIT);
      ORom::write(position, &flag, 1);
      ORom::read(position, &flag, 1);
      return flag == EPROM_DEF_CHECK_FLAG;
    }

    return true;
  }
}
