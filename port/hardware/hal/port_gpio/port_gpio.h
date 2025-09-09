/**
 * @file      port_gpio.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL GPIO port Driver (GPIO驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_GPIO_H__
#define __PORT_GPIO_H__

#if __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

  /// @brief port GPIO 工作模式
  typedef enum PORT_GPIO_MODE_E
  {
    PORT_GPIO_IN,        /* port GPIO 普通输入模式 */
    PORT_GPIO_IN_IT,     /* port GPIO 中断输入模式 */
    PORT_GPIO_IN_EVENT,  /* port GPIO 事件输入模式 */
    PORT_GPIO_IN_ANALOG, /* port GPIO 模拟输入模式 */
    PORT_GPIO_OUT_PP,    /* port GPIO 推挽输出模式 */
    PORT_GPIO_OUT_OD,    /* port GPIO 开漏输出模式 */
    PORT_GPIO_AF_PP,     /* port GPIO 推挽复用模式 */
    PORT_GPIO_AF_OD,     /* port GPIO 开漏复用模式 */
  } port_gpio_mode_e;

  /// @brief port GPIO 默认电压
  typedef enum PORT_GPIO_PULL_E
  {
    PORT_GPIO_PULL_NONE, /* port GPIO 引脚浮空 */
    PORT_GPIO_PULL_UP,   /* port GPIO 引脚上拉 */
    PORT_GPIO_PULL_DOWN, /* port GPIO 引脚下拉 */
  } port_gpio_pull_e;

  /// @brief port GPIO 工作速度
  typedef enum PORT_GPIO_SPEED_E
  {
    PORT_GPIO_SPEED_LOW       = 0, /* port GPIO 低速 */
    PORT_GPIO_SPEED_MED       = 1, /* port GPIO 中速 */
    PORT_GPIO_SPEED_HIGH      = 2, /* port GPIO 高速 */
    PORT_GPIO_SPEED_VERY_HIGH = 3, /* port GPIO 极高速 */
  } port_gpio_speed_e;

  /// @brief port GPIO 事件检测模式
  typedef enum PORT_GPIO_EVENT_E
  {
    PORT_GPIO_EVENT_RISING = 1, /* port GPIO 检测上升沿 */
    PORT_GPIO_EVENT_FALLING,    /* port GPIO 检测下降沿 */
    PORT_GPIO_EVENT_BOTH,       /* port GPIO 检测上升沿与下降沿 */
  } port_gpio_event_e;

  /// @brief port GPIO 回调函数结构体
  typedef struct PORT_GPIO_CALLBACK_T
  {
    void (*function)(void*); /* port GPIO 回调函数 */
    void* arg;               /* port GPIO 回调函数参数 */
  } port_gpio_callback_t;

  extern void v_port_gpio_nvic_enable(const uint8_t gpio_pin);
  extern void v_port_gpio_nvic_disable(const uint8_t gpio_pin);
  extern void v_port_gpio_nvic_set_pority(const uint8_t gpio_pin, uint8_t gpio_nvic_pority);
  extern void v_port_gpio_init(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_event_e gpio_event, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const port_gpio_callback_t* gpio_callback);
  extern void v_port_gpio_af_init(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const uint32_t gpio_af_channel);
  extern void v_port_gpio_change_mode(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_event_e gpio_event, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const port_gpio_callback_t* gpio_callback);
  extern void v_port_gpio_write(const uint8_t gpio_port, const uint8_t gpio_pin, const bool gpio_value);
  extern bool b_port_gpio_read(const uint8_t gpio_port, const uint8_t gpio_pin);
  extern void v_port_gpio_toggle(const uint8_t gpio_port, const uint8_t gpio_pin);
  extern void v_port_gpio_deinit(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_GPIO_H__ */
