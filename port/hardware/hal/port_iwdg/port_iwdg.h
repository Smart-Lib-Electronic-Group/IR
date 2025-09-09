/**
 * @file      port_iwdg.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL IWDG port Driver (看门狗驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_IWDG_H__
#define __PORT_IWDG_H__

#if __cplusplus
extern "C"
{
#endif

#include "error_handle.h"

  /// @brief port IWDG 预分频值
  typedef enum PORT_IWDG_PRESCALER_E
  {
    PORT_IWDG_4,   /* port IWDG   4分频 */
    PORT_IWDG_8,   /* port IWDG   8分频 */
    PORT_IWDG_16,  /* port IWDG  16分频 */
    PORT_IWDG_32,  /* port IWDG  32分频 */
    PORT_IWDG_64,  /* port IWDG  64分频 */
    PORT_IWDG_128, /* port IWDG 128分频 */
    PORT_IWDG_256, /* port IWDG 256分频 */
  } port_iwdg_prescaler_e;

  extern error_code_e e_port_iwdg_init(const port_iwdg_prescaler_e prescaler, const uint32_t reload);
  extern error_code_e e_port_iwdg_feed(void);

#if __cplusplus
}
#endif

#endif /* __PORT_IWDG_H__ */
