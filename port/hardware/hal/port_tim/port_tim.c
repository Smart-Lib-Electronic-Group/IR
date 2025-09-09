/**
 * @file      port_tim.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL TIM port Driver (定时器——驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_tim.h"
#include "port_include.h"

/// @brief TIM 中断默认优先级
#define TIMER_NVIC_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY - 1

/**
 * @brief (静态变量) port TIM 定时器映射表
 *
 */
static TIM_TypeDef* s_apt_port_timer_instance_map[]                  = { TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8, TIM9, TIM10, TIM11, TIM12, TIM13, TIM14 };

/**
 * @brief (静态常量) port TIM 定时器中断映射表
 *
 */
static const IRQn_Type sc_ae_port_timer_irqn_map[]                   = { TIM1_UP_TIM10_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_DAC_IRQn, TIM7_IRQn, TIM8_UP_TIM13_IRQn, TIM1_BRK_TIM9_IRQn, TIM1_UP_TIM10_IRQn, TIM1_TRG_COM_TIM11_IRQn, TIM8_BRK_TIM12_IRQn, TIM8_UP_TIM13_IRQn, TIM8_TRG_COM_TIM14_IRQn };

/**
 * @brief (静态常量) port TIM 通道映射表
 *
 */
static const uint32_t sc_aul_port_timeer_channel_map[]               = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

/**
 * @brief (静态常量) port TIM 激活的通道映射表
 *
 */
static const HAL_TIM_ActiveChannel sc_ae_port_timer_active_channel[] = { HAL_TIM_ACTIVE_CHANNEL_1, HAL_TIM_ACTIVE_CHANNEL_2, HAL_TIM_ACTIVE_CHANNEL_3, HAL_TIM_ACTIVE_CHANNEL_4 };

/**
 * @brief (全局变量) port TIM HAL 句柄指针数组
 *
 */
TIM_HandleTypeDef* g_apt_port_timer_handle[TIMER_COUNT]              = { 0 };

/**
 * @brief port TIM 输入捕获信息结构体
 *
 */
typedef struct PORT_TIMER_IC_DATA_T
{
  uint32_t ccr_1st_cnt;  /* port TIM 输入捕获 发生第一次下升沿捕获时CCR寄存器的值 */
  uint32_t ccr_2nd_cnt;  /* port TIM 输入捕获 发生第二次上升沿捕获时CCR寄存器的值 */
  uint32_t over_1st_cnt; /* port TIM 输入捕获 发生计数器溢出事件的次数--计数1 */
  uint32_t over_2nd_cnt; /* port TIM 输入捕获 发生计数器溢出事件的次数--计数2 */
  uint32_t over_tmp_cnt; /* port TIM 输入捕获 发生计数器溢出事件的次数--临时量 */
  uint32_t ic_flag;      /* port TIM 输入捕获 输入捕获状态机标志位 */
  bool     is_finished;  /* port TIM 输入捕获 输入捕获是否结束标志位 */
} port_timer_ic_data_t;

/**
 * @brief port TIM 信息结构体
 *
 */
typedef struct PORT_TIMER_INFO_T
{
  port_timer_ic_data_t* ic_data;     /* port TIM 输入捕获信息结构体指针 */
  port_os_semaphore_t   binary;      /* port TIM 二值信号量 */
  uint8_t               channel_num; /* port TIM 工作通道 */
  port_timer_type_e     type;        /* port TIM 工作模式 */
  bool                  is_running;  /* port TIM 工作标志位 */
  float                 frequency;   /* port TIM 频率 */
  float                 duty_cycle;  /* port TIM 占空比 */
  port_timer_callback_t callback;    /* port TIM 回调函数 */
} port_timer_info_t;

/**
 * @brief (静态局部变量) port TIM 信息结构体指针数组
 *
 */
static port_timer_info_t* s_apt_port_timer_info[TIMER_COUNT] = { 0 };

/**
 * @brief (静态内联) port TIM 获取 HAL库 TIM 通道句柄
 *
 * @param  timer_num    TIM 通道编号
 * @return TIM_TypeDef* TIM 通道HAL库句柄
 */
static inline TIM_TypeDef* sl_pt_port_timer_get_instance(const uint8_t timer_num)
{
  if (timer_num < 1 || timer_num > sizeof(s_apt_port_timer_instance_map) / sizeof(s_apt_port_timer_instance_map[0]))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer get instance error!\n");
    return 0;
  }

  return s_apt_port_timer_instance_map[timer_num - 1];
}

/**
 * @brief (静态) port TIM 获取 HAL库 TIM 计数模式
 *
 * @param timer_counter_mode  TIM 计数模式
 * @return uint32_t           TIM 计数模式HAL库编码
 */
static uint32_t s_ul_port_timer_get_counter_mode(const port_timer_counter_mode_e timer_counter_mode)
{
  switch (timer_counter_mode)
  {
    case PORT_TIMER_UP :
      return TIM_COUNTERMODE_UP;
    case PORT_TIMER_DOWN :
      return TIM_COUNTERMODE_DOWN;
    case PORT_TIMER_CENTER_ALIGNED1 :
      return TIM_COUNTERMODE_CENTERALIGNED1;
    case PORT_TIMER_CENTER_ALIGNED2 :
      return TIM_COUNTERMODE_CENTERALIGNED2;
    case PORT_TIMER_CENTER_ALIGNED3 :
      return TIM_COUNTERMODE_CENTERALIGNED3;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer get counter mode error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port TIM 获取 HAL库 TIM 时钟分频系数
 *
 * @param timer_division  TIM 时钟分频系数
 * @return uint32_t       TIM 时钟分频系数HAL库编码
 */
static uint32_t s_ul_port_timer_get_division(const uint8_t timer_division)
{
  switch (timer_division)
  {
    case 1 :
      return TIM_CLOCKDIVISION_DIV1;
    case 2 :
      return TIM_CLOCKDIVISION_DIV2;
    case 4 :
      return TIM_CLOCKDIVISION_DIV4;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer get division error!\n");
      return 0;
  }
}

/**
 * @brief (静态内联) port TIM 获取 HAL库 TIM 自动重载标志位
 *
 * @param timer_auto_reload TIM 自动重载标志位
 * @return uint32_t         TIM 自动重载标志位HAL库编码
 */
inline static uint32_t sl_ul_port_timer_get_auto_reload(const bool timer_auto_reload)
{
  return timer_auto_reload ? TIM_AUTORELOAD_PRELOAD_ENABLE : TIM_AUTORELOAD_PRELOAD_DISABLE;
}

/**
 * @brief (静态内联) port TIM 获取 HAL库 TIM 中断向量
 *
 * @param  timer_num    TIM 中断向量
 * @return IRQn_Type    TIM 中断向量HAL库编码
 */
static inline IRQn_Type sl_e_port_timer_get_irqn(const uint8_t timer_num)
{
  if (timer_num < 1 || timer_num > sizeof(sc_ae_port_timer_irqn_map) / sizeof(sc_ae_port_timer_irqn_map[0]))
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer get irqn error!\n");
    return (IRQn_Type)IRQN_ERROR;
  }

  return sc_ae_port_timer_irqn_map[timer_num - 1];
}

/**
 * @brief (静态内联) port TIM 获取 HAL库 TIM 通道编码
 *
 * @param  timer_channel_num  TIM 通道编码
 * @return uint32_t           TIM 通道编码HAL库编码
 */
static inline uint32_t sl_ul_port_timer_get_channel(const uint8_t timer_channel_num)
{
  if (timer_channel_num > 4)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer get channel error!\n");
    return 0;
  }

  return sc_aul_port_timeer_channel_map[timer_channel_num - 1];
}

/**
 * @brief (静态内联) port TIM 获取 HAL库 TIM 激活的通道编码
 *
 * @param  timer_channel_num      TIM 激活的通道编码
 * @return HAL_TIM_ActiveChannel  TIM 激活的通道编码HAL库编码
 */
static inline HAL_TIM_ActiveChannel sl_e_port_timer_get_active_channel(const uint8_t timer_channel_num)
{
  if (timer_channel_num > 4)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer get active channel error!\n");
    return (HAL_TIM_ActiveChannel)0;
  }

  return sc_ae_port_timer_active_channel[timer_channel_num - 1];
}

/**
 * @brief (静态) port TIM 获取 HAL库 TIM 输出比较工作模式
 *
 * @param timer_oc_mode   TIM 输出比较工作模式
 * @return uint32_t       TIM 输出比较工作模式HAL库编码
 */
static uint32_t s_ul_port_timer_get_oc_mode(const port_timer_oc_mode_e timer_oc_mode)
{
  switch (timer_oc_mode)
  {
    case PORT_TIMER_TIMING :
      return TIM_OCMODE_TIMING;
    case PORT_TIMER_ACTIVE :
      return TIM_OCMODE_ACTIVE;
    case PORT_TIMER_INACTIVE :
      return TIM_OCMODE_INACTIVE;
    case PORT_TIMER_TOGGLE :
      return TIM_OCMODE_TOGGLE;
    case PORT_TIMER_PWM1 :
      return TIM_OCMODE_PWM1;
    case PORT_TIMER_PWM2 :
      return TIM_OCMODE_PWM2;
    case PORT_TIMER_FORCED_ACTIVE :
      return TIM_OCMODE_FORCED_ACTIVE;
    case PORT_TIMER_FORCED_INACTIVE :
      return TIM_OCMODE_FORCED_INACTIVE;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer get oc mode error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port TIM 获取 HAL库 TIM 输入捕获分频系数
 *
 * @param timer_ic_prescaler  TIM 输入捕获分频系数
 * @return uint32_t           TIM 输入捕获分频系数HAL库编码
 */
static uint32_t s_ul_port_timer_get_ic_prescaler(const uint8_t timer_ic_prescaler)
{
  switch (timer_ic_prescaler)
  {
    case 1 :
      return TIM_ICPSC_DIV1;
    case 2 :
      return TIM_ICPSC_DIV2;
    case 4 :
      return TIM_ICPSC_DIV4;
    case 8 :
      return TIM_ICPSC_DIV8;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer get ic prescaler error!\n");
      return 0;
  }
}

/**
 * @brief port TIM 获取 HAL库 TIM GPIO复用编码
 *
 * @param timer_num TIM 通道编号
 * @return uint32_t GPIO复用HAL库编码
 */
uint32_t ul_port_timer_get_gpio_af(const uint8_t timer_num)
{
  switch (timer_num)
  {
    case 1 :
      return GPIO_AF1_TIM1;
    case 2 :
      return GPIO_AF1_TIM2;
    case 3 :
      return GPIO_AF2_TIM3;
    case 4 :
      return GPIO_AF2_TIM4;
    case 5 :
      return GPIO_AF2_TIM5;
    case 8 :
      return GPIO_AF3_TIM8;
    case 9 :
      return GPIO_AF3_TIM9;
    case 10 :
      return GPIO_AF3_TIM10;
    case 11 :
      return GPIO_AF3_TIM11;
    case 12 :
      return GPIO_AF9_TIM12;
    case 13 :
      return GPIO_AF9_TIM13;
    case 14 :
      return GPIO_AF9_TIM14;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer get gpio af error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port TIM HAL库 TIM 时钟使能
 *
 * @param timer_num TIM 通道编号
 */
static void s_v_port_timer_clock_enable(const uint8_t timer_num)
{
  switch (timer_num)
  {
    case 1 :
      __HAL_RCC_TIM1_CLK_ENABLE();
      break;
    case 2 :
      __HAL_RCC_TIM2_CLK_ENABLE();
      break;
    case 3 :
      __HAL_RCC_TIM3_CLK_ENABLE();
      break;
    case 4 :
      __HAL_RCC_TIM4_CLK_ENABLE();
      break;
    case 5 :
      __HAL_RCC_TIM5_CLK_ENABLE();
      break;
    case 6 :
      __HAL_RCC_TIM6_CLK_ENABLE();
      break;
    case 7 :
      __HAL_RCC_TIM7_CLK_ENABLE();
      break;
    case 8 :
      __HAL_RCC_TIM8_CLK_ENABLE();
      break;
    case 9 :
      __HAL_RCC_TIM9_CLK_ENABLE();
      break;
    case 10 :
      __HAL_RCC_TIM10_CLK_ENABLE();
      break;
    case 11 :
      __HAL_RCC_TIM11_CLK_ENABLE();
      break;
    case 12 :
      __HAL_RCC_TIM12_CLK_ENABLE();
      break;
    case 13 :
      __HAL_RCC_TIM13_CLK_ENABLE();
      break;
    case 14 :
      __HAL_RCC_TIM14_CLK_ENABLE();
      break;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer clock enable error!\n");
      break;
  }
}

/**
 * @brief (静态) port TIM HAL库 TIM 时钟失能
 *
 * @param timer_num TIM 通道编号
 */
static void s_v_port_timer_clock_disable(const uint8_t timer_num)
{
  switch (timer_num)
  {
    case 1 :
      __HAL_RCC_TIM1_CLK_DISABLE();
      break;
    case 2 :
      __HAL_RCC_TIM2_CLK_DISABLE();
      break;
    case 3 :
      __HAL_RCC_TIM3_CLK_DISABLE();
      break;
    case 4 :
      __HAL_RCC_TIM4_CLK_DISABLE();
      break;
    case 5 :
      __HAL_RCC_TIM5_CLK_DISABLE();
      break;
    case 6 :
      __HAL_RCC_TIM6_CLK_DISABLE();
      break;
    case 7 :
      __HAL_RCC_TIM7_CLK_DISABLE();
      break;
    case 8 :
      __HAL_RCC_TIM8_CLK_DISABLE();
      break;
    case 9 :
      __HAL_RCC_TIM9_CLK_DISABLE();
      break;
    case 10 :
      __HAL_RCC_TIM10_CLK_DISABLE();
      break;
    case 11 :
      __HAL_RCC_TIM11_CLK_DISABLE();
      break;
    case 12 :
      __HAL_RCC_TIM12_CLK_DISABLE();
      break;
    case 13 :
      __HAL_RCC_TIM13_CLK_DISABLE();
      break;
    case 14 :
      __HAL_RCC_TIM14_CLK_DISABLE();
      break;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer clock disable error!\n");
      break;
  }
}

/**
 * @brief (静态) port TIM 基础模式 端口 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @return error_code_e         错误代码
 */
static error_code_e s_e_port_timer_base_normal_init(const uint8_t timer_num, const uint32_t timer_psc, const uint32_t timer_arr, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division)
{
  /* TIM 句柄结构体动态内存申请 */
  TIM_HandleTypeDef* timer_handle = (TIM_HandleTypeDef*)Malloc(sizeof(TIM_HandleTypeDef));
  if (NULL == timer_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer base normal handle Malloc failed!\n");
    return g_e_error_code;
  }

  IRQn_Type               timer_irqn          = sl_e_port_timer_get_irqn(timer_num);
  TIM_ClockConfigTypeDef  timer_clock_config  = { 0 };
  TIM_MasterConfigTypeDef timer_master_config = { 0 };
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));

  /* TIM 时钟使能 */
  s_v_port_timer_clock_enable(timer_num);

  /* TIM 初始化结构体填充 */
  timer_handle->Instance           = sl_pt_port_timer_get_instance(timer_num);
  timer_handle->Init.Prescaler     = timer_psc;
  timer_handle->Init.CounterMode   = s_ul_port_timer_get_counter_mode(timer_counter_mode);
  timer_handle->Init.Period        = timer_arr;
  timer_handle->Init.ClockDivision = s_ul_port_timer_get_division(timer_division);

  /* TIM 中断失能 */
  HAL_NVIC_DisableIRQ(timer_irqn);
  HAL_NVIC_ClearPendingIRQ(timer_irqn);

  /* TIM 解除初始化 */
  if (HAL_OK != HAL_TIM_Base_DeInit(timer_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port timer base normal deinit failed!\n");
    goto timer_base_normal_init_err;
  }

  /* TIM 时钟源配置 */
  timer_clock_config.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_OK != HAL_TIM_ConfigClockSource(timer_handle, &timer_clock_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base normal clock source config failed!\n");
    goto timer_base_normal_init_err;
  }

  /* TIM 初始化 */
  if (HAL_OK != HAL_TIM_Base_Init(timer_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base normal init failed!\n");
    goto timer_base_normal_init_err;
  }

  /* TIM 主从模式配置 */
  timer_master_config.MasterOutputTrigger = TIM_TRGO_RESET;
  timer_master_config.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_OK != HAL_TIMEx_MasterConfigSynchronization(timer_handle, &timer_master_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base normal master config synchronization failed!\n");
    goto timer_base_normal_init_err;
  }

  /* TIM 中断使能 */
  HAL_NVIC_SetPriority(timer_irqn, TIMER_NVIC_DEF_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(timer_irqn);

  g_apt_port_timer_handle[timer_num - 1] = timer_handle;
  return SUCESS;

/* TIM 基础模式端口初始化故障处理 */
timer_base_normal_init_err:
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));
  Free(timer_handle);
  return g_e_error_code;
}

/**
 * @brief (静态) port TIM 输入捕获模式 端口 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_channel_num    TIM 通道编码
 * @param  timer_ic_prescaler   TIM 输入捕获分频系数
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @param  timer_auto_reload    TIM 自动重载标志位
 * @return error_code_e         错误代码
 */
static error_code_e s_e_port_timer_base_ic_init(const uint8_t timer_num, const uint32_t timer_psc, const uint32_t timer_arr, const uint8_t timer_channel_num, const uint8_t timer_ic_prescaler, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload)
{
  /* TIM 句柄结构体动态内存申请 */
  TIM_HandleTypeDef* timer_handle = (TIM_HandleTypeDef*)Malloc(sizeof(TIM_HandleTypeDef));
  if (NULL == timer_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer base ic handle Malloc failed!\n");
    return g_e_error_code;
  }

  IRQn_Type               timer_irqn          = sl_e_port_timer_get_irqn(timer_num);
  TIM_ClockConfigTypeDef  timer_clock_config  = { 0 };
  TIM_MasterConfigTypeDef timer_master_config = { 0 };
  TIM_IC_InitTypeDef      timer_ic_config     = { 0 };
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));

  /* TIM 时钟使能 */
  s_v_port_timer_clock_enable(timer_num);

  /* TIM 初始化结构体填充 */
  timer_handle->Instance               = sl_pt_port_timer_get_instance(timer_num);
  timer_handle->Init.Prescaler         = timer_psc;
  timer_handle->Init.CounterMode       = s_ul_port_timer_get_counter_mode(timer_counter_mode);
  timer_handle->Init.Period            = timer_arr;
  timer_handle->Init.ClockDivision     = s_ul_port_timer_get_division(timer_division);
  timer_handle->Init.AutoReloadPreload = sl_ul_port_timer_get_auto_reload(timer_auto_reload);

  /* TIM 中断失能 */
  HAL_NVIC_DisableIRQ(timer_irqn);
  HAL_NVIC_ClearPendingIRQ(timer_irqn);

  /* TIM 解除初始化 */
  if (HAL_OK != HAL_TIM_IC_DeInit(timer_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port timer base ic deinit failed!\n");
    goto timer_base_ic_init_err;
  }

  /* TIM 时钟源配置 */
  timer_clock_config.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_OK != HAL_TIM_ConfigClockSource(timer_handle, &timer_clock_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base ic clock source config failed!\n");
    goto timer_base_ic_init_err;
  }

  /* TIM 初始化 */
  if (HAL_OK != HAL_TIM_IC_Init(timer_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base ic init failed!\n");
    goto timer_base_ic_init_err;
  }

  /* TIM 主从模式配置 */
  timer_master_config.MasterOutputTrigger = TIM_TRGO_RESET;
  timer_master_config.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_OK != HAL_TIMEx_MasterConfigSynchronization(timer_handle, &timer_master_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base ic master config synchronization failed!\n");
    goto timer_base_ic_init_err;
  }

  /* TIM 输入捕获配置 */
  timer_ic_config.ICPolarity  = TIM_ICPOLARITY_BOTHEDGE;
  timer_ic_config.ICSelection = TIM_ICSELECTION_DIRECTTI;
  timer_ic_config.ICPrescaler = s_ul_port_timer_get_ic_prescaler(timer_ic_prescaler);
  timer_ic_config.ICFilter    = 0;
  if (HAL_OK != HAL_TIM_IC_ConfigChannel(timer_handle, &timer_ic_config, sl_ul_port_timer_get_channel(timer_channel_num)))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base ic config channel failed!\n");
    goto timer_base_ic_init_err;
  }

  /* TIM 使能 */
  HAL_NVIC_SetPriority(timer_irqn, TIMER_NVIC_DEF_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(timer_irqn);

  g_apt_port_timer_handle[timer_num - 1] = timer_handle;
  return SUCESS;

/* TIM 输入捕获模式端口初始化故障处理 */
timer_base_ic_init_err:
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));
  Free(timer_handle);
  return g_e_error_code;
}

/**
 * @brief (静态) port TIM 输出比较模式 端口 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_channel_num    TIM 通道编码
 * @param  timer_oc_mode        TIM 输出比较工作模式
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @param  timer_auto_reload    TIM 自动重载标志位
 * @return error_code_e         错误代码
 */
static error_code_e s_e_port_timer_base_oc_init(const uint8_t timer_num, const uint32_t timer_psc, const uint32_t timer_arr, const uint8_t timer_channel_num, const port_timer_oc_mode_e timer_oc_mode, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload)
{
  /* TIM 句柄结构体动态内存申请 */
  TIM_HandleTypeDef* timer_handle = (TIM_HandleTypeDef*)Malloc(sizeof(TIM_HandleTypeDef));
  if (NULL == timer_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer base oc handle Malloc failed!\n");
    return g_e_error_code;
  }

  TIM_ClockConfigTypeDef  timer_clock_config  = { 0 };
  TIM_MasterConfigTypeDef timer_master_config = { 0 };
  TIM_OC_InitTypeDef      timer_oc_config     = { 0 };
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));

  /* TIM 时钟使能 */
  s_v_port_timer_clock_enable(timer_num);

  /* TIM 初始化结构体填充 */
  timer_handle->Instance               = sl_pt_port_timer_get_instance(timer_num);
  timer_handle->Init.Prescaler         = timer_psc;
  timer_handle->Init.CounterMode       = s_ul_port_timer_get_counter_mode(timer_counter_mode);
  timer_handle->Init.Period            = timer_arr;
  timer_handle->Init.ClockDivision     = s_ul_port_timer_get_division(timer_division);
  timer_handle->Init.AutoReloadPreload = sl_ul_port_timer_get_auto_reload(timer_auto_reload);

  /* TIM 解除初始化 */
  if (HAL_OK != HAL_TIM_OC_DeInit(timer_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port timer base oc deinit failed!\n");
    goto timer_base_oc_init_err;
  }

  /* TIM 时钟源配置 */
  timer_clock_config.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_OK != HAL_TIM_ConfigClockSource(timer_handle, &timer_clock_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base oc config clock source failed!\n");
    goto timer_base_oc_init_err;
  }

  /* TIM 初始化 */
  if (HAL_OK != HAL_TIM_OC_Init(timer_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base oc init failed!\n");
    goto timer_base_oc_init_err;
  }

  /* TIM 主从模式配置 */
  timer_master_config.MasterOutputTrigger = TIM_TRGO_RESET;
  timer_master_config.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_OK != HAL_TIMEx_MasterConfigSynchronization(timer_handle, &timer_master_config))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base oc config master config synchronization failed!\n");
    goto timer_base_oc_init_err;
  }

  /* TIM 输出比较配置 */
  timer_oc_config.OCMode     = s_ul_port_timer_get_oc_mode(timer_oc_mode);
  timer_oc_config.Pulse      = timer_arr / 2;
  timer_oc_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  timer_oc_config.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_OK != HAL_TIM_OC_ConfigChannel(timer_handle, &timer_oc_config, sl_ul_port_timer_get_channel(timer_channel_num)))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer base oc config channel failed!\n");
    goto timer_base_oc_init_err;
  }

  g_apt_port_timer_handle[timer_num - 1] = timer_handle;
  return SUCESS;

/* TIM 输出比较模式端口初始化故障处理 */
timer_base_oc_init_err:
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));
  Free(timer_handle);
  return g_e_error_code;
}

/**
 * @brief (静态) port TIM 端口 解除初始化
 *
 * @param timer_num     TIM 通道编号
 * @return error_code_e 错误代码
 */
static error_code_e s_e_port_timer_base_deinit(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle = g_apt_port_timer_handle[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_handle)
    return SUCESS;

  IRQn_Type timer_irqn = sl_e_port_timer_get_irqn(timer_num);
  switch (s_apt_port_timer_info[timer_num - 1]->type)
  {
    case PORT_TIMER_NORMAL :
      /* TIM 中断失能 */
      HAL_NVIC_DisableIRQ(timer_irqn);
      HAL_NVIC_ClearPendingIRQ(timer_irqn);

      /* TIM 解除初始化 */
      if (HAL_OK != HAL_TIM_Base_DeInit(timer_handle))
      {
        g_e_error_code = DEINIT_ERROR;
        ERROR_HANDLE("port timer normal base deinit failed!\n");
      }
      break;
    case PORT_TIMER_IC :
      /* TIM 中断失能 */
      HAL_NVIC_DisableIRQ(timer_irqn);
      HAL_NVIC_ClearPendingIRQ(timer_irqn);

      /* TIM 解除初始化 */
      if (HAL_OK != HAL_TIM_IC_DeInit(timer_handle))
      {
        g_e_error_code = DEINIT_ERROR;
        ERROR_HANDLE("port timer ic base deinit failed!\n");
      }
      break;
    case PORT_TIMER_OC :
      /* TIM 解除初始化 */
      if (HAL_OK != HAL_TIM_OC_DeInit(timer_handle))
      {
        g_e_error_code = DEINIT_ERROR;
        ERROR_HANDLE("port timer base oc deinit failed!\n");
      }
      break;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port timer type undefined!\n");
      break;
  }

  /* TIM 时钟失能 */
  s_v_port_timer_clock_disable(timer_num);

  /* TIM 句柄结构体内存清理与释放 */
  memset(timer_handle, 0, sizeof(TIM_HandleTypeDef));
  Free(timer_handle);
  g_apt_port_timer_handle[timer_num - 1] = NULL;

  return g_e_error_code;
}

/**
 * @brief (静态) port TIM 中断计数溢出事件回调函数
 *
 * @param timer_num TIM 通道编号
 */
static void s_v_port_timer_period_elapsed_callback(const uint8_t timer_num)
{
  /* TIM 获取指针 */
  port_timer_info_t* timer_info = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info)
    return;

  if (false == timer_info->is_running)
    return;

  if (PORT_TIMER_IC == timer_info->type)
  {
    if (false == timer_info->ic_data->is_finished)
      /* TIM 溢出次数自加 */
      timer_info->ic_data->over_tmp_cnt++;

    return;
  }

  if (PORT_TIMER_NORMAL == timer_info->type)
  {
    /* TIM 普通模式回调函数 */
    if (timer_info->callback.function)
      timer_info->callback.function(timer_info->callback.arg);
    /* TIM 释放信号量 */
    b_port_os_semaphore_give(timer_info->binary);
    return;
  }
}

/**
 * @brief (静态) port TIM 输入捕获事件回调函数
 *
 * @param timer_num TIM 通道编号
 */
static void s_v_port_timer_capture_callback(const uint8_t timer_num)
{
  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle      = g_apt_port_timer_handle[timer_num - 1];
  port_timer_info_t* timer_info        = s_apt_port_timer_info[timer_num - 1];
  uint32_t           timer_channel_num = sl_ul_port_timer_get_channel(timer_info->channel_num);

  /* TIM 空指针判断 */
  if (NULL == timer_handle || NULL == timer_info)
    return;

  /* TIM 完成标志位检测 */
  if (false == timer_info->is_running || true == timer_info->ic_data->is_finished)
    return;

  /* TIM 通道检测 */
  if (timer_handle->Channel != sl_e_port_timer_get_active_channel(timer_info->channel_num))
    return;

  switch (timer_info->ic_data->ic_flag)
  {
    case 0 :
      {
        /* TIM 参数清空 */
        memset(timer_info->ic_data, 0, sizeof(port_timer_ic_data_t));
        /* TIM 清空定时器的计数器 */
        __HAL_TIM_SET_COUNTER(timer_handle, 0);
        /* TIM 设置成下降沿捕获 */
        __HAL_TIM_SET_CAPTUREPOLARITY(timer_handle, timer_channel_num, TIM_INPUTCHANNELPOLARITY_FALLING);
        /* TIM 设置捕获状态为第二个阶段 */
        timer_info->ic_data->ic_flag = 1;
        break;
      }
    case 1 :
      {
        /* TIM 获取存放在CCR寄存器的值 */
        timer_info->ic_data->ccr_1st_cnt  = __HAL_TIM_GET_COMPARE(timer_handle, timer_channel_num);
        /* TIM 获取计时器溢出次数 */
        timer_info->ic_data->over_1st_cnt = timer_info->ic_data->over_tmp_cnt;
        /* TIM 设置成上升沿捕获 */
        __HAL_TIM_SET_CAPTUREPOLARITY(timer_handle, timer_channel_num, TIM_INPUTCHANNELPOLARITY_RISING);
        /* TIM 设置捕获状态为第三个阶段 */
        timer_info->ic_data->ic_flag = 2;
        break;
      }
    case 2 :
      {
        /* TIM 完成一次捕获,设置标志位,锁定数据 */
        timer_info->ic_data->is_finished  = true;
        /* TIM 取存放在CCR寄存器的值 */
        timer_info->ic_data->ccr_2nd_cnt  = __HAL_TIM_GET_COMPARE(timer_handle, timer_channel_num);
        /* TIM 获取计时器溢出次数 */
        timer_info->ic_data->over_2nd_cnt = timer_info->ic_data->over_tmp_cnt;
        /* TIM 设置捕获状态为第一个阶段 */
        timer_info->ic_data->ic_flag      = 0;
        /* TIM 释放信号量 */
        b_port_os_semaphore_give(timer_info->binary);
        break;
      }
  }
}

/**
 * @brief port TIM 使能(开启)
 *
 * @param  timer_num      TIM 通道编号
 * @return error_code_e   错误代码
 */
error_code_e e_port_timer_start(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle = g_apt_port_timer_handle[timer_num - 1];
  port_timer_info_t* timer_info   = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_handle || NULL == timer_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port timer handle or info is null!\n");
    return g_e_error_code;
  }

  if (true == timer_info->is_running)
    return SUCESS;

  switch (timer_info->type)
  {
    case PORT_TIMER_NORMAL :
      {
        /* TIM 获取信号量 */
        b_port_os_semaphore_take(timer_info->binary, 1);
        /* TIM 使能(开启定时器) */
        if (HAL_OK != HAL_TIM_Base_Start_IT(timer_handle))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer normal start failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = true;
        break;
      }
    case PORT_TIMER_IC :
      {
        timer_info->ic_data->ic_flag = 0;
        /* TIM 获取信号量 */
        b_port_os_semaphore_take(timer_info->binary, 1);
        /* TIM 使能(开始输入捕获) */
        if (HAL_OK != HAL_TIM_IC_Start_IT(timer_handle, sl_ul_port_timer_get_channel(timer_info->channel_num)))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer ic start failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = true;
        break;
      }
    case PORT_TIMER_OC :
      {
        /* TIM 使能(开始输出比较) */
        if (HAL_OK != HAL_TIM_OC_Start_IT(timer_handle, sl_ul_port_timer_get_channel(timer_info->channel_num)))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer oc start failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = true;
        break;
      }
    default :
      {
        g_e_error_code = UNDEFINED_ERROR;
        ERROR_HANDLE("port timer type not support!\n");
        return g_e_error_code;
      }
  }

  return SUCESS;
}

/**
 * @brief port TIM 失能(关闭)
 *
 * @param  timer_num      TIM 通道编号
 * @return error_code_e   错误代码
 */
error_code_e e_port_timer_stop(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle = g_apt_port_timer_handle[timer_num - 1];
  port_timer_info_t* timer_info   = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_handle || NULL == timer_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port timer handle or info is null!\n");
    return g_e_error_code;
  }

  if (false == timer_info->is_running)
    return SUCESS;

  switch (timer_info->type)
  {
    case PORT_TIMER_NORMAL :
      {
        /* TIM 失能(关闭定时器) */
        if (HAL_OK != HAL_TIM_Base_Stop_IT(timer_handle))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer normal stop failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = false;
        break;
      }
    case PORT_TIMER_IC :
      {
        /* TIM 失能(结束输入捕获) */
        if (HAL_OK != HAL_TIM_IC_Stop_IT(timer_handle, sl_ul_port_timer_get_channel(timer_info->channel_num)))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer ic stop failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = false;
        break;
      }
    case PORT_TIMER_OC :
      {
        /* TIM 失能(结束输出比较) */
        if (HAL_OK != HAL_TIM_OC_Stop_IT(timer_handle, sl_ul_port_timer_get_channel(timer_info->channel_num)))
        {
          g_e_error_code = SETUP_ERROR;
          ERROR_HANDLE("port timer oc stop failed!\n");
          return g_e_error_code;
        }
        timer_info->is_running = false;
        break;
      }
    default :
      {
        g_e_error_code = UNDEFINED_ERROR;
        ERROR_HANDLE("port timer type not support!\n");
        return g_e_error_code;
      }
  }

  return SUCESS;
}

/**
 * @brief port TIM 等待信号量
 *
 * @param  timer_num      TIM 通道编号
 * @param  waiting_time   阻塞时长(ms)
 * @return true           事件发生
 * @return false          事件超时
 */
bool b_port_timer_wait_semaphore(const uint8_t timer_num, uint32_t waiting_time)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return false;
  }

  /* TIM 获取指针 */
  port_timer_info_t* timer_info = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port timer this timer is not init!\n");
    return false;
  }

  if (PORT_TIMER_NORMAL == timer_info->type || PORT_TIMER_IC == timer_info->type)
  {
    /* TIM 等待信号量 */
    return b_port_os_semaphore_take(timer_info->binary, waiting_time);
  }

  return false;
}

/**
 * @brief port TIM 计算数据(输入捕获数据分析)
 *
 * @param  timer_num      TIM 通道编号
 * @param  timer_psc      TIM 预分频系数
 * @param  timer_arr      TIM 自动重装载值
 * @return error_code_e   错误代码
 */
error_code_e e_port_timer_analyze(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  port_timer_info_t* timer_info = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port timer this timer is not init!\n");
    return g_e_error_code;
  }

  if (PORT_TIMER_IC == timer_info->type)
  {
    /*
      ------------------------------------------占空比计算公式------------------------------------------
      (period_cnt1 * (arr + 1) + ccr1_cnt + 1) / (period_cnt2 * (arr + 1) + ccr2_cnt + 1)   *  100
                      高电平时间                                  一个周期时间                 百分比转换
    */
    timer_info->duty_cycle = ((float)(timer_info->ic_data->over_1st_cnt * (timer_arr + 1) + timer_info->ic_data->ccr_1st_cnt + 1)) * 100.0f / ((float)(timer_info->ic_data->over_2nd_cnt * (timer_arr + 1) + timer_info->ic_data->ccr_2nd_cnt + 1));

    /*
      -------------------------------频率计算公式-------------------------------
         clk_frequency  /  psc  /  (period_cnt2 * (arr + 1) + ccr2_cnt + 1)
          总线时钟频率    预分频系数                  一个周期时间
    */
    if (1 == timer_num)
      /* TIM1在APB2上,故获取PCLK2时钟频率 */
      timer_info->frequency = ((float)HAL_RCC_GetPCLK2Freq() / (float)timer_psc) / ((float)(timer_info->ic_data->over_2nd_cnt * (timer_arr + 1) + timer_info->ic_data->ccr_2nd_cnt + 1));
    else
      /* 其余定时器在APB1上,故获取PCLK1时钟频率乘以2 (如果APB1预分频系数等于1,则频率不变,否则频率乘以2) */
      timer_info->frequency = ((float)HAL_RCC_GetPCLK1Freq() * 2.0f / (float)timer_psc) / ((float)(timer_info->ic_data->over_2nd_cnt * (timer_arr + 1) + timer_info->ic_data->ccr_2nd_cnt + 1));

    /* 清空标志位以进入下次采集 */
    timer_info->ic_data->ic_flag     = 0;
    timer_info->ic_data->is_finished = false;
  }

  return SUCESS;
}

/**
 * @brief port TIM 占空比设置(输出比较参数设置)
 *
 * @param  timer_num          TIM 通道编号
 * @param  timer_arr          TIM 自动重装载值
 * @param  timer_duty_cycle   TIM 占空比(0.0~100.0)
 * @return error_code_e       错误代码
 */
error_code_e e_port_timer_set_duty_cycle(const uint8_t timer_num, const uint16_t timer_arr, float timer_duty_cycle)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle = g_apt_port_timer_handle[timer_num - 1];
  port_timer_info_t* timer_info   = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info || NULL == timer_handle)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port timer this timer is not init!\n");
    return g_e_error_code;
  }

  if (PORT_TIMER_OC == timer_info->type)
  {
    /* 占空比参数检查 */
    if (0.0f > timer_duty_cycle)
      timer_duty_cycle = 0.0f;
    else if (100.0f < timer_duty_cycle)
      timer_duty_cycle = 100.0f;

    /* 占空比参数保存 */
    timer_info->duty_cycle = timer_duty_cycle;

    /* 占空比 = (arr + 1) * duty_cycle / 100 */
    __HAL_TIM_SET_COMPARE(timer_handle, sl_ul_port_timer_get_channel(timer_info->channel_num), (uint32_t)((timer_arr + 1) * timer_duty_cycle / 100.0f));
  }

  return SUCESS;
}

/**
 * @brief port TIM 获取 占空比
 *
 * @param  timer_num  TIM 通道编号
 * @return float      TIM 占空比
 */
float f_port_timer_get_duty_cycle(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return 0.0f;
  }

  /* TIM 获取指针 */
  port_timer_info_t* timer_info = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port timer this timer is not init!\n");
    return 0.0f;
  }
  else
    return timer_info->duty_cycle;
}

/**
 * @brief port TIM 获取 频率
 *
 * @param  timer_num  TIM 通道编号
 * @return float      TIM 频率
 */
float f_port_timer_get_frequency(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return 0.0f;
  }

  /* TIM 获取指针 */
  port_timer_info_t* timer_info = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port timer this timer is not init!\n");
    return 0.0f;
  }
  else
    return timer_info->frequency;
}

/**
 * @brief port TIM 基础模式 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @param  timer_callback       TIM 回调函数
 * @return error_code_e         错误代码
 */
error_code_e e_port_timer_normal_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const port_timer_callback_t* timer_callback)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 是否已经初始化判断 */
  if (NULL != s_apt_port_timer_info[timer_num - 1])
  {
    g_e_error_code = ALREADY_INIT_ERROR;
    ERROR_HANDLE("port timer already initialized!\n");
    return g_e_error_code;
  }

  /* TIM 信息结构体动态内存申请 */
  port_timer_info_t* timer_info = (port_timer_info_t*)Malloc(sizeof(port_timer_info_t));
  if (NULL == timer_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer normal info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(timer_info, 0, sizeof(port_timer_info_t));

  /* TIM 设置模式 */
  timer_info->type = PORT_TIMER_NORMAL;
  /*
    ---------------------频率计算公式---------------------
       clk_frequency   /   (arr + 1)   *   (psc + 1)
        总线时钟频率        自动重装载值       预分频系数
  */
  if (1 == timer_num)
    /* TIM1在APB2上,故获取PCLK2时钟频率 */
    timer_info->frequency = (float)HAL_RCC_GetPCLK2Freq() / (float)((timer_arr + 1) * (timer_psc + 1));
  else
    /* 其余定时器在APB1上,故获取PCLK1时钟频率乘以2 (如果APB1预分频系数等于1,则频率不变,否则频率乘以2) */
    timer_info->frequency = (float)HAL_RCC_GetPCLK1Freq() * 2.0f / (float)((timer_arr + 1) * (timer_psc + 1));

  /* TIM 无用参数置零 */
  timer_info->ic_data     = NULL;
  timer_info->channel_num = 0;
  timer_info->duty_cycle  = 0.0f;

  timer_info->is_running  = false;

  /* TIM 基础模式端口初始化 */
  if (SUCESS != s_e_port_timer_base_normal_init(timer_num, timer_psc, timer_arr, timer_counter_mode, timer_division))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer normal base init failed!\n");
    goto timer_normal_init_err;
  }

  /* TIM 信号量创建 */
  timer_info->binary = pt_port_os_semaphore_create(1, 1);
  if (NULL == timer_info->binary)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer normal semaphore create failed!\n");
    goto timer_normal_init_err;
  }

  if (NULL != timer_callback)
    memcpy(&timer_info->callback, timer_callback, sizeof(port_timer_callback_t));
  else
    memset(&timer_info->callback, 0, sizeof(port_timer_callback_t));

  /* TIM 通道信息保存 */
  s_apt_port_timer_info[timer_num - 1] = timer_info;
  return SUCESS;

/* TIM 基础模式初始化故障处理 */
timer_normal_init_err:
  /* TIM 信息结构体内存清理与释放 */
  if (timer_info)
  {
    /* TIM 清除信号量 */
    if (NULL != timer_info->binary)
      v_port_os_semaphore_delete(timer_info->binary);
    memset(timer_info, 0, sizeof(port_timer_info_t));
  }
  Free(timer_info);
  return g_e_error_code;
}

/**
 * @brief port TIM 输入捕获模式 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_channel_num    TIM 通道编码
 * @param  timer_ic_prescaler   TIM 输入捕获分频系数
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @param  timer_auto_reload    TIM 自动重载标志位
 * @return error_code_e         错误代码
 */
error_code_e e_port_timer_ic_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const uint8_t timer_channel_num, const uint8_t timer_ic_prescaler, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 是否已经初始化判断 */
  if (NULL != s_apt_port_timer_info[timer_num - 1])
  {
    g_e_error_code = ALREADY_INIT_ERROR;
    ERROR_HANDLE("port timer already initialized!\n");
    return g_e_error_code;
  }

  /* TIM 信息结构体动态内存申请 */
  port_timer_info_t* timer_info = (port_timer_info_t*)Malloc(sizeof(port_timer_info_t));
  if (NULL == timer_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer ic info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(timer_info, 0, sizeof(port_timer_info_t));

  /* TIM 输入捕获信息结构体动态内存申请 */
  timer_info->ic_data = (port_timer_ic_data_t*)Malloc(sizeof(port_timer_ic_data_t));
  if (NULL == timer_info->ic_data)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer ic data Malloc failed!\n");
    goto timer_ic_init_err;
  }

  memset(timer_info->ic_data, 0, sizeof(port_timer_ic_data_t));

  /* TIM 设置模式 */
  timer_info->type        = PORT_TIMER_IC;
  timer_info->channel_num = timer_channel_num;
  /* TIM 参数置零 */
  timer_info->duty_cycle  = 0.0f;
  timer_info->frequency   = 0.0f;

  timer_info->is_running  = false;

  /* TIM 输入捕获模式端口初始化 */
  if (SUCESS != s_e_port_timer_base_ic_init(timer_num, timer_psc, timer_arr, timer_channel_num, timer_ic_prescaler, timer_counter_mode, timer_division, timer_auto_reload))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer ic base init failed!\n");
    goto timer_ic_init_err;
  }

  /* TIM 信号量创建 */
  timer_info->binary = pt_port_os_semaphore_create(1, 1);
  if (NULL == timer_info->binary)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer normal semaphore create failed!\n");
    goto timer_ic_init_err;
  }

  /* TIM 通道信息保存 */
  s_apt_port_timer_info[timer_num - 1] = timer_info;
  return SUCESS;

/* TIM 输入捕获模式初始化故障处理 */
timer_ic_init_err:
  /* TIM 信息结构体内存清理与释放 */
  if (NULL != timer_info)
  {
    /* TIM 清除信号量 */
    if (NULL != timer_info->binary)
      v_port_os_semaphore_delete(timer_info->binary);
    /* TIM 输入捕获信息结构体内存清理与释放 */
    if (NULL != timer_info->ic_data)
      memset(timer_info->ic_data, 0, sizeof(port_timer_ic_data_t));
    Free(timer_info->ic_data);
    memset(timer_info, 0, sizeof(port_timer_info_t));
  }
  Free(timer_info);
  return g_e_error_code;
}

/**
 * @brief port TIM 输出比较模式 初始化
 *
 * @param  timer_num            TIM 通道编号
 * @param  timer_psc            TIM 预分频系数
 * @param  timer_arr            TIM 自动重装载值
 * @param  timer_channel_num    TIM 通道编码
 * @param  timer_oc_mode        TIM 输出比较工作模式
 * @param  timer_counter_mode   TIM 计数模式
 * @param  timer_division       TIM 时钟分频系数
 * @param  timer_auto_reload    TIM 自动重载标志位
 * @return error_code_e         错误代码
 */
error_code_e e_port_timer_oc_init(const uint8_t timer_num, const uint16_t timer_psc, const uint16_t timer_arr, const uint8_t timer_channel_num, const port_timer_oc_mode_e timer_oc_mode, const port_timer_counter_mode_e timer_counter_mode, const uint8_t timer_division, const bool timer_auto_reload)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 是否已经初始化判断 */
  if (NULL != s_apt_port_timer_info[timer_num - 1])
  {
    g_e_error_code = ALREADY_INIT_ERROR;
    ERROR_HANDLE("port timer already initialized!\n");
    return g_e_error_code;
  }

  /* TIM 信息结构体动态内存申请 */
  port_timer_info_t* timer_info = (port_timer_info_t*)Malloc(sizeof(port_timer_info_t));
  if (NULL == timer_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port timer oc info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(timer_info, 0, sizeof(port_timer_info_t));

  /* TIM 设置模式 */
  timer_info->type        = PORT_TIMER_OC;
  timer_info->channel_num = timer_channel_num;
  /*
    ---------------------频率计算公式---------------------
       clk_frequency   /   (arr + 1)   *   (psc + 1)
        总线时钟频率        自动重装载值       预分频系数
  */
  if (1 == timer_num)
    /* TIM1在APB2上,故获取PCLK2时钟频率 */
    timer_info->frequency = (float)HAL_RCC_GetPCLK2Freq() / (float)((timer_arr + 1) * (timer_psc + 1));
  else
    /* 其余定时器在APB1上,故获取PCLK1时钟频率乘以2 (如果APB1预分频系数等于1,则频率不变,否则频率乘以2) */
    timer_info->frequency = (float)HAL_RCC_GetPCLK1Freq() * 2.0f / (float)((timer_arr + 1) * (timer_psc + 1));

  /* 无用参数置零 */
  timer_info->duty_cycle = 0.0f;
  timer_info->ic_data    = NULL;

  timer_info->is_running = false;

  /* TIM 输出比较模式端口初始化 */
  if (SUCESS != s_e_port_timer_base_oc_init(timer_num, timer_psc, timer_arr, timer_channel_num, timer_oc_mode, timer_counter_mode, timer_division, timer_auto_reload))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port timer oc base init failed!\n");
    goto timer_oc_init_err;
  }

  /* TIM 通道信息保存 */
  s_apt_port_timer_info[timer_num - 1] = timer_info;
  return SUCESS;

/* TIM 输出比较模式初始化故障处理 */
timer_oc_init_err:
  /* TIM 信息结构体内存清理与释放 */
  if (NULL != timer_info)
    memset(timer_info, 0, sizeof(port_timer_info_t));
  Free(timer_info);
  return g_e_error_code;
}

/**
 * @brief port TIM 解除初始化
 *
 * @param  timer_num      TIM 通道编号
 * @return error_code_e   错误代码
 */
error_code_e e_port_timer_deinit(const uint8_t timer_num)
{
  /* TIM 输入参数合法性检查 */
  if (timer_num < 1 || timer_num > TIMER_COUNT)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port timer invalid number!\n");
    return g_e_error_code;
  }

  /* TIM 获取指针 */
  TIM_HandleTypeDef* timer_handle = g_apt_port_timer_handle[timer_num - 1];
  port_timer_info_t* timer_info   = s_apt_port_timer_info[timer_num - 1];

  /* TIM 空指针判断 */
  if (NULL == timer_info || NULL == timer_handle)
    return g_e_error_code;

  /* TIM 停止工作 */
  if (SUCESS != e_port_timer_stop(timer_num))
  {
    g_e_error_code = SETUP_ERROR;
    ERROR_HANDLE("port timer stop failed!\n");
  }

  /* TIM 端口解除初始化 */
  if (SUCESS != s_e_port_timer_base_deinit(timer_num))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port timer base deinit failed!\n");
  }

  /* TIM 输入捕获信息结构体内存清理与释放 */
  if (PORT_TIMER_IC == timer_info->type)
  {
    memset(timer_info->ic_data, 0, sizeof(port_timer_ic_data_t));
    Free(timer_info->ic_data);
  }

  /* TIM 清除信号量 */
  if (NULL != timer_info->binary)
    v_port_os_semaphore_delete(timer_info->binary);

  /* TIM 信息结构体内存清理与释放 */
  memset(timer_info, 0, sizeof(port_timer_info_t));
  Free(timer_info);
  s_apt_port_timer_info[timer_num - 1] = NULL;

  return g_e_error_code;
}

/**
 * @brief port TIM HAL库 计数溢出事件回调函数
 *
 * @param htim TIM HAL库句柄结构体指针
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  if (htim->Instance == TIM1)
    s_v_port_timer_period_elapsed_callback(1);
  else if (htim->Instance == TIM2)
    s_v_port_timer_period_elapsed_callback(2);
  else if (htim->Instance == TIM3)
    s_v_port_timer_period_elapsed_callback(3);
  else if (htim->Instance == TIM4)
    s_v_port_timer_period_elapsed_callback(4);
  else if (htim->Instance == TIM5)
    s_v_port_timer_period_elapsed_callback(5);
  else if (htim->Instance == TIM6)
    s_v_port_timer_period_elapsed_callback(6);
  else if (htim->Instance == TIM7)
    s_v_port_timer_period_elapsed_callback(7);
  else if (htim->Instance == TIM8)
    s_v_port_timer_period_elapsed_callback(8);
  else if (htim->Instance == TIM9)
    s_v_port_timer_period_elapsed_callback(9);
  else if (htim->Instance == TIM10)
    s_v_port_timer_period_elapsed_callback(10);
  else if (htim->Instance == TIM11)
    s_v_port_timer_period_elapsed_callback(11);
  else if (htim->Instance == TIM12)
    s_v_port_timer_period_elapsed_callback(12);
  else if (htim->Instance == TIM13)
    s_v_port_timer_period_elapsed_callback(13);
  else if (htim->Instance == TIM14)
    s_v_port_timer_period_elapsed_callback(14);
}

/**
 * @brief port TIM HAL库 输入捕获事件回调函数
 *
 * @param htim TIM HAL库句柄结构体指针
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
  if (htim->Instance == TIM1)
    s_v_port_timer_capture_callback(1);
  else if (htim->Instance == TIM2)
    s_v_port_timer_capture_callback(2);
  else if (htim->Instance == TIM3)
    s_v_port_timer_capture_callback(3);
  else if (htim->Instance == TIM4)
    s_v_port_timer_capture_callback(4);
  else if (htim->Instance == TIM5)
    s_v_port_timer_capture_callback(5);
  else if (htim->Instance == TIM6)
    s_v_port_timer_capture_callback(6);
  else if (htim->Instance == TIM7)
    s_v_port_timer_capture_callback(7);
  else if (htim->Instance == TIM8)
    s_v_port_timer_capture_callback(8);
  else if (htim->Instance == TIM9)
    s_v_port_timer_capture_callback(9);
  else if (htim->Instance == TIM10)
    s_v_port_timer_capture_callback(10);
  else if (htim->Instance == TIM11)
    s_v_port_timer_capture_callback(11);
  else if (htim->Instance == TIM12)
    s_v_port_timer_capture_callback(12);
  else if (htim->Instance == TIM13)
    s_v_port_timer_capture_callback(13);
  else if (htim->Instance == TIM14)
    s_v_port_timer_capture_callback(14);
}
