/**
 * @file      port_dma.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL DMA port Driver (DMA驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_dma.h"
#include "port_include.h"

/// @brief DMA 中断默认优先级
#define DMA_NVIC_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY - 3

/**
 * @brief (静态变量) port DMA 数据流映射表
 *
 */
static DMA_Stream_TypeDef* s_apt_port_dma_stream_map[2][8] = {
  { DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, DMA1_Stream3, DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7 },
  { DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, DMA2_Stream3, DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7 }
};

/**
 * @brief (静态常量) port DMA 中断向量映射表
 *
 */
static const IRQn_Type sc_ae_port_dma_irqn_map[2][8] = {
  { DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, DMA1_Stream7_IRQn },
  { DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn }
};

/**
 * @brief (静态常量) port DMA 通道映射表
 *
 */
static uint32_t sc_ul_port_dma_channel_map[8]  = { DMA_CHANNEL_0, DMA_CHANNEL_1, DMA_CHANNEL_2, DMA_CHANNEL_3, DMA_CHANNEL_4, DMA_CHANNEL_5, DMA_CHANNEL_6, DMA_CHANNEL_7 };

/**
 * @brief (全局变量) port DMA HAL 句柄指针数组
 *
 */
DMA_HandleTypeDef* g_apt_port_dma_handle[2][8] = { 0 };

/**
 * @brief (静态内联) port DMA 获取 HAL库 DMA 通道句柄
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 * @return DMA_Channel_TypeDef* DMA HAL库源句柄
 */
static inline DMA_Stream_TypeDef* sl_pt_port_dma_get_instance(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  if (dma_num < 1 || dma_num > 2 || dma_stream_num > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma get instance error!\n");
    return NULL;
  }

  return s_apt_port_dma_stream_map[dma_num - 1][dma_stream_num];
}

/**
 * @brief (静态内联) port DMA 获取 HAL库 DMA 中断向量
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 * @return IRQn_Type            DMA HAL库中断向量
 */
static inline IRQn_Type sl_e_port_dma_get_irqn(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  if (dma_num < 1 || dma_num > 2 || dma_stream_num > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma get irqn error!\n");
    return (IRQn_Type)IRQN_ERROR;
  }

  return sc_ae_port_dma_irqn_map[dma_num - 1][dma_stream_num];
}

/**
 * @brief (静态内联) port DMA 获取 DMA 通道编码
 *
 * @param  dma_channel DMA 通道编号 (0-7)
 * @return uint32_t   DMA 通道HAL库编码
 */
static inline uint32_t sl_ul_port_dma_get_channel(const uint8_t dma_channel)
{
  if (dma_channel > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma get channel error!\n");
    return 0;
  }

  return sc_ul_port_dma_channel_map[dma_channel];
}

/**
 * @brief (静态) port DMA 获取 HAL库 DMA 数据流方向
 *
 * @param  dma_direction DMA 数据流方向
 * @return uint32_t     DMA 数据流方向HAL库编码
 */
static uint32_t s_ul_port_dma_get_direction(const port_dma_direction_e dma_direction)
{
  switch (dma_direction)
  {
    case PORT_DMA_PERIPH_TO_MEMORY :
      return DMA_PERIPH_TO_MEMORY;
    case PORT_DMA_MEMORY_TO_PERIPH :
      return DMA_MEMORY_TO_PERIPH;
    case PORT_DMA_MEMORY_TO_MEMORY :
      return DMA_MEMORY_TO_MEMORY;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port dma get direction error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port DMA 获取 HAL库 DMA 工作模式
 *
 * @param  dma_mode  DMA 工作模式
 * @return uint32_t DMA 工作模式HAL库编码
 */
static uint32_t s_ul_port_dma_get_mode(const port_dma_mode_e dma_mode)
{
  switch (dma_mode)
  {
    case PORT_DMA_NORMAL :
      return DMA_NORMAL;
    case PORT_DMA_CIRCULAR :
      return DMA_CIRCULAR;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port dma get mode error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port DMA 获取 HAL库 DMA 工作优先级
 *
 * @param  dma_priority  DMA 工作优先级
 * @return uint32_t     DMA 工作优先级HAL库编码
 */
static uint32_t s_ul_port_dma_get_priority(const port_dma_priority_e dma_priority)
{
  switch (dma_priority)
  {
    case PORT_DMA_LOW :
      return DMA_PRIORITY_LOW;
    case PORT_DMA_MEDIUM :
      return DMA_PRIORITY_MEDIUM;
    case PORT_DMA_HIGH :
      return DMA_PRIORITY_HIGH;
    case PORT_DMA_VERY_HIGH :
      return DMA_PRIORITY_VERY_HIGH;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port dma get priority error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port DMA HAL库 DMA 时钟使能
 *
 * @param dma_num DMA 编号 (1-2)
 */
static void s_v_port_dma_clock_enable(const uint8_t dma_num)
{
  if (1 == dma_num)
    __HAL_RCC_DMA1_CLK_ENABLE();
  else if (2 == dma_num)
    __HAL_RCC_DMA2_CLK_ENABLE();
}

/**
 * @brief port DMA 获取句柄指针
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 * @return DMA_HandleTypeDef* DMA HAL库句柄指针
 */
DMA_HandleTypeDef* pt_port_dma_get_handle(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  if (dma_num < 1 || dma_num > 2 || dma_stream_num > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma get handle error!\n");
    return NULL;
  }

  return g_apt_port_dma_handle[dma_num - 1][dma_stream_num];
}

/**
 * @brief port DMA NVIC 中断 使能
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 */
void v_port_dma_nvic_enable(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  IRQn_Type dma_irqn = sl_e_port_dma_get_irqn(dma_num, dma_stream_num);
  if ((IRQn_Type)INIT_ERROR != dma_irqn)
    HAL_NVIC_EnableIRQ(dma_irqn);
}

/**
 * @brief port DMA NVIC 中断 失能
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 */
void v_port_dma_nvic_disable(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  IRQn_Type dma_irqn = sl_e_port_dma_get_irqn(dma_num, dma_stream_num);
  if ((IRQn_Type)INIT_ERROR != dma_irqn)
    HAL_NVIC_DisableIRQ(dma_irqn);
}

/**
 * @brief port DMA NVIC 中断 优先级设置
 *
 * @param  dma_num              DMA 编号 (1-2)
 * @param  dma_stream_num       DMA 流编号 (0-7)
 * @param  dma_nvic_priority    NVIC 中断 优先级
 */
void v_port_dma_nvic_set_priority(const uint8_t dma_num, const uint8_t dma_stream_num, uint8_t dma_nvic_priority)
{
  IRQn_Type dma_irqn = sl_e_port_dma_get_irqn(dma_num, dma_stream_num);

  if ((IRQn_Type)INIT_ERROR != dma_irqn)
  {
    HAL_NVIC_DisableIRQ(dma_irqn);
    HAL_NVIC_SetPriority(dma_irqn, dma_nvic_priority, 0);
    HAL_NVIC_EnableIRQ(dma_irqn);
  }
}

/**
 * @brief port DMA 初始化
 *
 * @param  dma_num           DMA 编号 (1-2)
 * @param  dma_stream_num    DMA 流编号 (0-7)
 * @param  dma_channel       DMA 通道编号 (0-7)
 * @param  dma_direction     DMA 数据流方向
 * @param  dma_mode          DMA 工作模式
 * @param  dma_priority      DMA 工作优先级
 * @return error_code_e      错误编码
 */
error_code_e e_port_dma_init(const uint8_t dma_num, const uint8_t dma_stream_num, const uint8_t dma_channel, const port_dma_direction_e dma_direction, const port_dma_mode_e dma_mode, const port_dma_priority_e dma_priority)
{
  /* DMA 参数合法性判断 */
  if (dma_num < 1 || dma_num > 2 || dma_stream_num > 7 || dma_channel > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma invalid infomation!\n");
    return g_e_error_code;
  }

  /* DMA 是否已经初始化判断 */
  if (NULL != pt_port_dma_get_handle(dma_num, dma_stream_num))
  {
    ERROR_HANDLE("port dma channel already initialized!\n");
    g_e_error_code = ALREADY_INIT_ERROR;
    return g_e_error_code;
  }

  /* DMA 句柄结构体动态内存申请 */
  DMA_HandleTypeDef* dma_handle = (DMA_HandleTypeDef*)Malloc(sizeof(DMA_HandleTypeDef));
  if (NULL == dma_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port dma handle Malloc failed!\n");
    goto dma_err;
  }

  memset(dma_handle, 0, sizeof(DMA_HandleTypeDef));
  /* DMA 时钟使能 */
  s_v_port_dma_clock_enable(dma_num);
  /* DMA 初始化结构体填充 */
  dma_handle->Instance                 = sl_pt_port_dma_get_instance(dma_num, dma_stream_num);
  dma_handle->Init.Channel             = sl_ul_port_dma_get_channel(dma_channel);
  dma_handle->Init.Direction           = s_ul_port_dma_get_direction(dma_direction);
  dma_handle->Init.PeriphInc           = DMA_PINC_DISABLE;
  dma_handle->Init.MemInc              = DMA_MINC_ENABLE;
  dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  dma_handle->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  dma_handle->Init.Mode                = s_ul_port_dma_get_mode(dma_mode);
  dma_handle->Init.Priority            = s_ul_port_dma_get_priority(dma_priority);

  IRQn_Type dma_irqn                   = sl_e_port_dma_get_irqn(dma_num, dma_stream_num);
  /* DMA 中断失能 */
  HAL_NVIC_DisableIRQ(dma_irqn);
  HAL_NVIC_ClearPendingIRQ(dma_irqn);
  /* DMA 解除初始化 */
  if (HAL_OK != HAL_DMA_DeInit(dma_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port dma deinit failed!\n");
    goto dma_err;
  }

  /* DMA 初始化 */
  if (HAL_OK != HAL_DMA_Init(dma_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port dma init failed!\n");
    goto dma_err;
  }
  /* DMA 中断使能 */
  HAL_NVIC_EnableIRQ(dma_irqn);
  HAL_NVIC_SetPriority(dma_irqn, DMA_NVIC_DEF_PRIORITY, 0);

  g_apt_port_dma_handle[dma_num - 1][dma_stream_num] = dma_handle;
  return SUCESS;

/* DMA 初始化故障处理 */
dma_err:
  memset(dma_handle, 0, sizeof(DMA_HandleTypeDef));
  Free(dma_handle);
  return g_e_error_code;
}

/**
 * @brief port DMA 解除初始化
 *
 * @param  dma_num           DMA 编号 (1-2)
 * @param  dma_stream_num    DMA 流编号 (0-7)
 * @return error_code_e   错误编码
 */
error_code_e e_port_dma_deinit(const uint8_t dma_num, const uint8_t dma_stream_num)
{
  /* DMA 参数合法性判断 */
  if (dma_num < 1 || dma_num > 2 || dma_stream_num > 7)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port dma invalid infomation!\n");
    return g_e_error_code;
  }

  /* DMA 空指针判断 */
  DMA_HandleTypeDef* dma_handle = pt_port_dma_get_handle(dma_num, dma_stream_num);
  if (NULL == dma_handle)
    return g_e_error_code;

  IRQn_Type dma_irqnn = sl_e_port_dma_get_irqn(dma_num, dma_stream_num);
  /* DMA 中断失能 */
  HAL_NVIC_DisableIRQ(dma_irqnn);
  HAL_NVIC_ClearPendingIRQ(dma_irqnn);

  /* DMA 解除初始化 */
  if (HAL_DMA_DeInit(dma_handle) != HAL_OK)
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("dma deinit failed!\n");
  }

  /* DMA 句柄结构体内存清理与释放 */
  memset(dma_handle, 0, sizeof(DMA_HandleTypeDef));
  Free(dma_handle);
  g_apt_port_dma_handle[dma_num - 1][dma_stream_num] = NULL;

  return g_e_error_code;
}
