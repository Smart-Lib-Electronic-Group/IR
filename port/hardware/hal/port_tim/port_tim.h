/**
 * @file      port_tim.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL TIM port Driver (定时器——驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_TIM_H__
#define __PORT_TIM_H__

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "error_handle.h"

/// @brief TIM 定时器总数
#define TIMER_COUNT 14

  /// @brief port TIM 工作模式
  typedef enum PORT_TIMER_TYPE_E
  {
    PORT_TIMER_NORMAL, /* port TIM 普通模式 */
    PORT_TIMER_IC,     /* port TIM 输入捕获模式 */
    PORT_TIMER_OC,     /* port TIM 输出比较模式 */
  } port_timer_type_e;

  /// @brief port TIM 计数模式
  typedef enum PORT_TIMER_COUNTER_TYPE_E
  {
    PORT_TIMER_UP,              /* port TIM 向上计数模式 */
    PORT_TIMER_DOWN,            /* port TIM 向下计数模式 */
    PORT_TIMER_CENTER_ALIGNED1, /* port TIM 中心对齐1模式 */
    PORT_TIMER_CENTER_ALIGNED2, /* port TIM 中心对齐2模式 */
    PORT_TIMER_CENTER_ALIGNED3, /* port TIM 中心对齐3模式 */
  } port_timer_counter_mode_e;

  /// @brief port TIM 输出比较工作模式
  typedef enum PORT_TIMER_OC_MODE_E
  {
    PORT_TIMER_TIMING,          /* port TIM 时间模式 */
    PORT_TIMER_ACTIVE,          /* port TIM 激活模式 */
    PORT_TIMER_INACTIVE,        /* port TIM 非激活模式 */
    PORT_TIMER_TOGGLE,          /* port TIM 翻转触发模式 */
    PORT_TIMER_PWM1,            /* port TIM PWM1模式 */
    PORT_TIMER_PWM2,            /* port TIM PWM2模式 */
    PORT_TIMER_FORCED_ACTIVE,   /* port TIM 强制激活模式 */
    PORT_TIMER_FORCED_INACTIVE, /* port TIM 强制非激活模式 */
  } port_timer_oc_mode_e;

  /// @brief port TIM 回调函数结构体
  typedef struct PORT_TIMER_CALLBACK_T
  {
    void (*function)(void*); /* port TIM 回调函数 */
    void* arg;               /* port TIM 回调函数参数 */
  } port_timer_callback_t;

  extern uint32_t     ul_port_timer_get_gpio_af(const uint8_t timer_num);
  extern error_code_e e_port_timer_start(const uint8_t timer_num);
  extern error_code_e e_port_timer_stop(const uint8_t timer_num);
  extern bool         b_port_timer_wait_semaphore(const uint8_t timer_num, uint32_t waiting_time);
  extern error_code_e e_port_timer_analyze(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr);
  extern error_code_e e_port_timer_set_duty_cycle(const uint8_t timer_num, const uint16_t timer_arr, const float timer_duty_cycle);
  extern float        f_port_timer_get_duty_cycle(const uint8_t timer_num);
  extern float        f_port_timer_get_frequency(const uint8_t timer_num);
  extern error_code_e e_port_timer_normal_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const port_timer_callback_t* timer_callback);
  extern error_code_e e_port_timer_ic_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const uint8_t timer_channel_num, const uint8_t timer_ic_prescaler, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload);
  extern error_code_e e_port_timer_oc_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const uint8_t timer_channel_num, const port_timer_oc_mode_e timer_oc_mode, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload);
  extern error_code_e e_port_timer_deinit(const uint8_t timer_num);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_TIM_H__ */
