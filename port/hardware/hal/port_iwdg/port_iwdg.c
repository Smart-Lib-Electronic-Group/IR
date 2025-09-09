/**
 * @file      port_iwdg.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL IWDG port Driver (看门狗驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_iwdg.h"
#include "port_include.h"

/**
 * @brief  port IWDG 预分频值编码映射表
 *
 */
static const uint32_t sc_aul_port_iwdg_prescaler_map[] = { [PORT_IWDG_4] = IWDG_PRESCALER_4, [PORT_IWDG_8] = IWDG_PRESCALER_8, [PORT_IWDG_16] = IWDG_PRESCALER_16, [PORT_IWDG_32] = IWDG_PRESCALER_32, [PORT_IWDG_64] = IWDG_PRESCALER_64, [PORT_IWDG_128] = IWDG_PRESCALER_128, [PORT_IWDG_256] = IWDG_PRESCALER_256 };

/// @brief (静态) port IWDG 看门狗句柄指针
static IWDG_HandleTypeDef s_t_port_iwdg_handle         = { 0 };

/**
 * @brief  port IWDG 获取 HAL库 看门狗 预分频值
 *
 * @param  prescaler IWDG 预分频值
 * @return uint32_t  IWDG 预分频值HAL库编码
 */
static uint32_t s_ul_port_iwdg_get_prescaler(const port_iwdg_prescaler_e prescaler)
{
  if (prescaler < PORT_IWDG_4 && prescaler > PORT_IWDG_256)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port iwdg get prescaler error!\n");
    return 0;
  }

  return sc_aul_port_iwdg_prescaler_map[prescaler];
}

/**
 * @brief  port IWDG 初始化
 *
 * @param  prescaler      IWDG 预分频值
 * @param  reload         IWDG 重装值
 * @return error_code_e   错误代码
 * @note   timeout(ms) = prescaler * reload / 32
 */
error_code_e e_port_iwdg_init(const port_iwdg_prescaler_e prescaler, const uint32_t reload)
{
  s_t_port_iwdg_handle.Instance       = IWDG;
  s_t_port_iwdg_handle.Init.Prescaler = s_ul_port_iwdg_get_prescaler(prescaler);
  s_t_port_iwdg_handle.Init.Reload    = reload;

  if (HAL_OK != HAL_IWDG_Init(&s_t_port_iwdg_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port iwdg init failed!\n");
  }

  return SUCESS;
}

/**
 * @brief  port IWDG 喂狗
 *
 * @return error_code_e   错误代码
 */
error_code_e e_port_iwdg_feed(void)
{
  HAL_IWDG_Refresh(&s_t_port_iwdg_handle);
  return SUCESS;
}