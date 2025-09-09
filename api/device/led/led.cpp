/**
 * @file      led.cpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for LED device (LED与呼吸灯)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "led.hpp"

using namespace OwO;
using namespace virtual_class;
using namespace device;

O_METAOBJECT(Led, VGpio)
O_METAOBJECT(BLed, VTimer)
