/**
 * @file      port_gpio.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL GPIO port Driver (GPIO驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_gpio.h"
#include "port_include.h"

/// @brief GPIO 中断默认优先级
#define GPIO_NVIC_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY

/**
 * @brief (静态常量) port GPIO 引脚映射表
 *
 */
static const uint32_t sc_aul_port_gpio_pin_map[]              = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15 };

/**
 * @brief (静态变量) port GPIO 端口映射表
 *
 */
static GPIO_TypeDef* s_apt_port_gpio_port_map[]               = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIOJ };

/**
 * @brief (静态常量) port GPIO 中断向量映射表
 *
 */
static const IRQn_Type sc_ae_port_gpio_irqn_map[]             = { EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn };

static port_gpio_callback_t s_at_port_gpio_callback_array[16] = { 0 };

/**
 * @brief (静态内联) port GPIO 获取 HAL库 GPIO 引脚编码
 *
 * @param  gpio_pin GPIO 引脚编号
 * @return uint32_t GPIO 引脚HAL库编码
 */
static inline uint32_t sl_ul_port_gpio_get_pin(const uint8_t gpio_pin)
{
  if (gpio_pin >= sizeof(sc_aul_port_gpio_pin_map) / sizeof(sc_aul_port_gpio_pin_map[0]))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port gpio get pin error!\n");
    return 0;
  }

  return sc_aul_port_gpio_pin_map[gpio_pin];
}

/**
 * @brief (静态内联) port GPIO 获取 HAL库 GPIO 端口句柄
 *
 * @param  gpio_port     GPIO 端口编号
 * @return GPIO_TypeDef* GPIO 端口HAL库句柄
 */
static inline GPIO_TypeDef* sl_pt_port_gpio_get_port(const uint8_t gpio_port)
{
  if (gpio_port < 'A' || gpio_port >= (sizeof(s_apt_port_gpio_port_map) / sizeof(s_apt_port_gpio_port_map[0]) + 'A'))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port gpio get port error!\n");
    return 0;
  }

  return s_apt_port_gpio_port_map[(gpio_port - 'A')];
}

/**
 * @brief (静态) port GPIO 获取 HAL库 GPIO 引脚工作模式
 *
 * @param  gpio_mode  GPIO 引脚模式
 * @param  gpio_event GPIO 事件模式
 * @return uint32_t   GPIO 引脚工作模式HAL库编码
 */
static uint32_t s_ul_port_gpio_get_mode(const port_gpio_mode_e gpio_mode, const port_gpio_event_e gpio_event)
{
  switch (gpio_mode)
  {
    case PORT_GPIO_IN :
      return GPIO_MODE_INPUT;
    case PORT_GPIO_IN_IT :
      {
        switch (gpio_event)
        {
          case PORT_GPIO_EVENT_RISING :
            return GPIO_MODE_IT_RISING;
          case PORT_GPIO_EVENT_FALLING :
            return GPIO_MODE_IT_FALLING;
          case PORT_GPIO_EVENT_BOTH :
            return GPIO_MODE_IT_RISING_FALLING;
          default :
            goto gpio_get_mode_err;
        }
      }
    case PORT_GPIO_IN_EVENT :
      {
        switch (gpio_event)
        {
          case PORT_GPIO_EVENT_RISING :
            return GPIO_MODE_EVT_RISING;
          case PORT_GPIO_EVENT_FALLING :
            return GPIO_MODE_EVT_FALLING;
          case PORT_GPIO_EVENT_BOTH :
            return GPIO_MODE_EVT_RISING_FALLING;
          default :
            goto gpio_get_mode_err;
        }
      }
    case PORT_GPIO_IN_ANALOG :
      return GPIO_MODE_ANALOG;
    case PORT_GPIO_OUT_PP :
      return GPIO_MODE_OUTPUT_PP;
    case PORT_GPIO_OUT_OD :
      return GPIO_MODE_OUTPUT_OD;
    case PORT_GPIO_AF_PP :
      return GPIO_MODE_AF_PP;
    case PORT_GPIO_AF_OD :
      return GPIO_MODE_AF_OD;

    default :
      goto gpio_get_mode_err;
  }

gpio_get_mode_err:
  g_e_error_code = UNDEFINED_ERROR;
  ERROR_HANDLE("port gpio get mode error!\n");
  return 0;
}

/**
 * @brief (静态内联) port GPIO 获取 HAL库 GPIO 中断向量
 *
 * @param  gpio_pin  GPIO 引脚编号
 * @return IRQn_Type GPIO HAL库中断向量
 */
static inline IRQn_Type sl_e_port_gpio_get_irqn(const uint8_t gpio_pin)
{
  if (gpio_pin >= sizeof(sc_ae_port_gpio_irqn_map) / sizeof(sc_ae_port_gpio_irqn_map[0]))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port gpio get irq error!\n");
    return (IRQn_Type)IRQN_ERROR;
  }

  return sc_ae_port_gpio_irqn_map[gpio_pin];
}

/**
 * @brief (静态) port GPIO HAL库 GPIO 时钟使能
 *
 * @param gpio_port GPIO 端口编号
 */
static void s_v_port_gpio_clock_enable(const uint8_t gpio_port)
{
  if (gpio_port < 'A' || gpio_port >= (sizeof(s_apt_port_gpio_port_map) / sizeof(s_apt_port_gpio_port_map[0]) + 'A'))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port gpio clock enable error!\n");
    return;
  }

  if ('A' == gpio_port)
    __HAL_RCC_GPIOA_CLK_ENABLE();
  else if ('B' == gpio_port)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if ('C' == gpio_port)
    __HAL_RCC_GPIOC_CLK_ENABLE();
  else if ('D' == gpio_port)
    __HAL_RCC_GPIOD_CLK_ENABLE();
  else if ('E' == gpio_port)
    __HAL_RCC_GPIOE_CLK_ENABLE();
  else if ('F' == gpio_port)
    __HAL_RCC_GPIOF_CLK_ENABLE();
  else if ('G' == gpio_port)
    __HAL_RCC_GPIOG_CLK_ENABLE();
  else if ('H' == gpio_port)
    __HAL_RCC_GPIOH_CLK_ENABLE();
  else if ('I' == gpio_port)
    __HAL_RCC_GPIOI_CLK_ENABLE();
  else if ('J' == gpio_port)
    __HAL_RCC_GPIOJ_CLK_ENABLE();
}

/**
 * @brief port GPIO NVIC 中断 使能
 *
 * @param gpio_pin  GPIO 引脚编号
 */
void v_port_gpio_nvic_enable(const uint8_t gpio_pin)
{
  IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);

  if ((IRQn_Type)IRQN_ERROR != gpio_irqn)
    HAL_NVIC_EnableIRQ(gpio_irqn);
}

/**
 * @brief port GPIO NVIC 中断 失能
 *
 * @param gpio_pin  GPIO 引脚编号
 */
void v_port_gpio_nvic_disable(const uint8_t gpio_pin)
{
  IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);

  if ((IRQn_Type)IRQN_ERROR != gpio_irqn)
    HAL_NVIC_DisableIRQ(gpio_irqn);
}

/**
 * @brief port GPIO NVIC 中断 优先级设置
 *
 * @param gpio_pin    GPIO 引脚编号
 * @param nvic_pority NVIC 中断 优先级
 */
void v_port_gpio_nvic_set_pority(const uint8_t gpio_pin, uint8_t gpio_nvic_pority)
{
  IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);

  if ((IRQn_Type)IRQN_ERROR != gpio_irqn)
  {
    HAL_NVIC_DisableIRQ(gpio_irqn);
    HAL_NVIC_SetPriority(gpio_irqn, gpio_nvic_pority, 0);
    HAL_NVIC_EnableIRQ(gpio_irqn);
  }
}

/**
 * @brief port GPIO 初始化
 *
 * @param gpio_port     GPIO 端口编号
 * @param gpio_pin      GPIO 引脚编号
 * @param gpio_mode     GPIO 工作模式
 * @param gpio_event    GPIO 事件模式
 * @param gpio_pull     GPIO 默认电压
 * @param gpio_speed    GPIO 工作速度
 * @param gpio_callback GPIO 回调函数
 */
void v_port_gpio_init(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_event_e gpio_event, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const port_gpio_callback_t* gpio_callback)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  /* GPIO 时钟使能 */
  s_v_port_gpio_clock_enable(gpio_port);
  /* GPIO 初始化结构体填充 */
  GPIO_InitStruct.Pin   = sl_ul_port_gpio_get_pin(gpio_pin);
  GPIO_InitStruct.Mode  = s_ul_port_gpio_get_mode(gpio_mode, gpio_event);
  GPIO_InitStruct.Pull  = (uint32_t)gpio_pull;
  GPIO_InitStruct.Speed = (uint32_t)gpio_speed;

  /* GPIO 解除初始化 */
  HAL_GPIO_DeInit(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin));
  /* GPIO 初始化 */
  HAL_GPIO_Init(sl_pt_port_gpio_get_port(gpio_port), &GPIO_InitStruct);

  if (PORT_GPIO_IN_IT == gpio_mode || PORT_GPIO_IN_EVENT == gpio_mode)
  {
    /* GPIO 中断使能 */
    IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);
    if (IRQN_ERROR != gpio_irqn)
    {
      HAL_NVIC_DisableIRQ(gpio_irqn);
      HAL_NVIC_ClearPendingIRQ(gpio_irqn);
      HAL_NVIC_SetPriority(gpio_irqn, GPIO_NVIC_DEF_PRIORITY, 0);
      if (NULL != gpio_callback)
        memcpy(&s_at_port_gpio_callback_array[gpio_pin], gpio_callback, sizeof(port_gpio_callback_t));
      else
        memset(&s_at_port_gpio_callback_array[gpio_pin], 0, sizeof(port_gpio_callback_t));
      HAL_NVIC_EnableIRQ(gpio_irqn);
    }
  }
}

/**
 * @brief port GPIO 复用初始化
 *
 * @param gpio_port         GPIO 端口编号
 * @param gpio_pin          GPIO 引脚编号
 * @param gpio_mode         GPIO 工作模式
 * @param gpio_pull         GPIO 默认电压
 * @param gpio_speed        GPIO 工作速度
 * @param gpio_af_channels  GPIO 复用通道
 */
void v_port_gpio_af_init(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const uint32_t gpio_af_channel)
{
  if (PORT_GPIO_AF_OD == gpio_mode || PORT_GPIO_AF_PP == gpio_mode)
  {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO 时钟使能 */
    s_v_port_gpio_clock_enable(gpio_port);
    /* GPIO 初始化结构体填充 */
    GPIO_InitStruct.Pin       = sl_ul_port_gpio_get_pin(gpio_pin);
    GPIO_InitStruct.Mode      = s_ul_port_gpio_get_mode(gpio_mode, PORT_GPIO_EVENT_BOTH);
    GPIO_InitStruct.Pull      = (uint32_t)gpio_pull;
    GPIO_InitStruct.Speed     = (uint32_t)gpio_speed;
    GPIO_InitStruct.Alternate = gpio_af_channel;

    /* GPIO 解除初始化 */
    HAL_GPIO_DeInit(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin));
    /* GPIO 初始化 */
    HAL_GPIO_Init(sl_pt_port_gpio_get_port(gpio_port), &GPIO_InitStruct);
  }
}

/**
 * @brief port GPIO 更改工作模式(保留中断设置)
 *
 * @param gpio_port     GPIO 端口编号
 * @param gpio_pin      GPIO 引脚编号
 * @param gpio_mode     GPIO 工作模式
 * @param gpio_event    GPIO 事件模式
 * @param gpio_pull     GPIO 默认电压
 * @param gpio_speed    GPIO 工作速度
 * @param gpio_callback GPIO 回调函数
 */
void v_port_gpio_change_mode(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode, const port_gpio_event_e gpio_event, const port_gpio_pull_e gpio_pull, const port_gpio_speed_e gpio_speed, const port_gpio_callback_t* gpio_callback)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  /* GPIO 时钟使能 */
  s_v_port_gpio_clock_enable(gpio_port);
  /* GPIO 初始化结构体填充 */
  GPIO_InitStruct.Pin   = sl_ul_port_gpio_get_pin(gpio_pin);
  GPIO_InitStruct.Mode  = s_ul_port_gpio_get_mode(gpio_mode, gpio_event);
  GPIO_InitStruct.Pull  = (uint32_t)gpio_pull;
  GPIO_InitStruct.Speed = (uint32_t)gpio_speed;

  /* GPIO 初始化 */
  HAL_GPIO_Init(sl_pt_port_gpio_get_port(gpio_port), &GPIO_InitStruct);

  if (PORT_GPIO_IN_IT == gpio_mode || PORT_GPIO_IN_EVENT == gpio_mode)
  {
    /* GPIO 中断使能 */
    IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);
    if (IRQN_ERROR != gpio_irqn)
    {
      HAL_NVIC_DisableIRQ(gpio_irqn);
      HAL_NVIC_ClearPendingIRQ(gpio_irqn);
      HAL_NVIC_SetPriority(gpio_irqn, GPIO_NVIC_DEF_PRIORITY, 0);
      if (NULL != gpio_callback)
        memcpy(&s_at_port_gpio_callback_array[gpio_pin], gpio_callback, sizeof(port_gpio_callback_t));
      else
        memset(&s_at_port_gpio_callback_array[gpio_pin], 0, sizeof(port_gpio_callback_t));
      HAL_NVIC_EnableIRQ(gpio_irqn);
    }
  }
}

/**
 * @brief port GPIO 设置引脚电平
 *
 * @param gpio_port  GPIO 端口编号
 * @param gpio_pin   GPIO 引脚编号
 * @param gpio_value GPIO 引脚电平
 */
void v_port_gpio_write(const uint8_t gpio_port, const uint8_t gpio_pin, const bool gpio_value)
{
  HAL_GPIO_WritePin(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin), gpio_value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief port GPIO 读取引脚电平
 *
 * @param gpio_port  GPIO 端口编号
 * @param gpio_pin   GPIO 引脚编号
 * @return true      高电平
 * @return false     低电平
 */
bool b_port_gpio_read(const uint8_t gpio_port, const uint8_t gpio_pin)
{
  return (bool)HAL_GPIO_ReadPin(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin));
}

/**
 * @brief port GPIO 翻转引脚电平
 *
 * @param gpio_port  GPIO 端口编号
 * @param gpio_pin   GPIO 引脚编号
 */
void v_port_gpio_toggle(const uint8_t gpio_port, const uint8_t gpio_pin)
{
  HAL_GPIO_TogglePin(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin));
}

/**
 * @brief port GPIO 解除初始化
 *
 * @param gpio_port  GPIO 端口编号
 * @param gpio_pin   GPIO 引脚编号
 * @param gpio_mode  GPIO 工作模式
 */
void v_port_gpio_deinit(const uint8_t gpio_port, const uint8_t gpio_pin, const port_gpio_mode_e gpio_mode)
{
  if (PORT_GPIO_IN_IT == gpio_mode || PORT_GPIO_IN_EVENT == gpio_mode)
  {
    /* GPIO 中断失能 */
    IRQn_Type gpio_irqn = sl_e_port_gpio_get_irqn(gpio_pin);
    if ((IRQn_Type)IRQN_ERROR != gpio_irqn)
    {
      HAL_NVIC_DisableIRQ(gpio_irqn);
      HAL_NVIC_ClearPendingIRQ(gpio_irqn);
    }
  }

  /* GPIO 解除初始化 */
  HAL_GPIO_DeInit(sl_pt_port_gpio_get_port(gpio_port), sl_ul_port_gpio_get_pin(gpio_pin));
}

/**
 * @brief port GPIO HAL库 中断事件回调函数(事件分发与上浮)
 *
 * @param GPIO_Pin GPIO 引脚HAL库编码
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint8_t i = 0;

  /* GPIO 事件分发与上浮 */
  for (i = 0; i < sizeof(sc_aul_port_gpio_pin_map) / sizeof(sc_aul_port_gpio_pin_map[0]); i++)
  {
    if (GPIO_Pin == sc_aul_port_gpio_pin_map[i])
    {
      if (NULL != s_at_port_gpio_callback_array[i].function)
        s_at_port_gpio_callback_array[i].function(s_at_port_gpio_callback_array[i].arg);
      return;
    }
  }
  g_e_error_code = UNDEFINED_ERROR;
  ERROR_HANDLE("port gpio callback error!\n");
}
