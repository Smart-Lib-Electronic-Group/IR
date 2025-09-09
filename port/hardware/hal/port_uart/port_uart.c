/**
 * @file      port_uart.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL UART port Driver (UART——驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_uart.h"
#include "port_gpio.h"
#include "port_dma.h"
#include "port_include.h"
#include "uart_dma_double.h"

/// @brief port UART 中断默认优先级
#define UART_NVIC_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY
/// @brief port UART 任务默认优先级
#define UART_TASK_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY - 1
/// @brief port UART 任务默认栈大小
#define UART_TASK_DEF_STACK    128

/// @brief UART 接收完成事件标志位
#define UART_RECI_CPLT_BIT 0x00000001
/// @brief UART 发送完成事件标志位
#define UART_SEND_CPLT_BIT 0x00000002

/**
 * @brief (全局变量) port UART HAL 句柄指针数组
 *
 */
UART_HandleTypeDef* g_apt_port_uart_handle[UART_COUNT]  = { 0 };

/**
 * @brief (静态局部变量) port UART 引脚信息数组
 *
 */
static uint8_t s_auc_port_uart_gpio_info[UART_COUNT][4] = {
  { 'A', 9,  'A', 10 }, /* USART1 PA-09,PA-10 */
  { 'D', 5,  'D', 6  }, /* USART2 PD-05,PD-06 */
  { 'D', 8,  'D', 9  }, /* USART3 PD-08,PD-09 */
  { 'C', 10, 'C', 11 }, /* UART4  PC-10,PC-11 */
  { 'C', 12, 'D', 2  }, /* UART5  PC-12,PD-02 */
  { 'C', 6,  'C', 7  }, /* USART6 PC-06,PC-07 */
  { 'A', 15, 'A', 8  }, /* UART7  PA-15,PA-08 */
  { 'E', 1,  'E', 0  }  /* UART8  PE-01,PE-00 */
};

/**
 * @brief (静态局部变量) port UART DMA信息数组
 *
 */
static uint8_t s_auc_port_uart_dma_info[UART_COUNT][6] = {
  { 2, 7, 4, 2, 5, 4 }, /* USART1  TX -> DMA2-Stream7-Channel4  RX -> DMA2-Stream5-Channel4 */
  { 1, 6, 4, 1, 5, 4 }, /* USART2  TX -> DMA1-Stream6-Channel4  RX -> DMA1-Stream5-Channel4 */
  { 1, 3, 4, 1, 1, 4 }, /* USART3  TX -> DMA1-Stream3-Channel4  RX -> DMA1-Stream1-Channel4 */
  { 1, 4, 4, 1, 2, 4 }, /* UART4   TX -> DMA1-Stream4-Channel4  RX -> DMA1-Stream2-Channel4 */
  { 1, 7, 4, 1, 0, 4 }, /* UART5   TX -> DMA1-Stream7-Channel4  RX -> DMA1-Stream0-Channel4 */
  { 2, 6, 5, 2, 2, 5 }, /* USART6  TX -> DMA2-Stream6-Channel5  RX -> DMA2-Stream2-Channel5 */
  { 1, 3, 5, 1, 1, 5 }, /* UART7   TX -> DMA1-Stream3-Channel5  RX -> DMA1-Stream1-Channel5 */
  { 1, 6, 5, 1, 0, 5 }  /* UART8   TX -> DMA1-Stream6-Channel5  RX -> DMA1-Stream0-Channel5 */
};

/**
 * @brief port UART 串口信息结构体
 *
 */
typedef struct PORT_UART_INFO_T
{
  port_uart_callback_t  uart_tx_cplt;      /* port UART 发送完成回调函数 */
  port_uart_callback_t  uart_rx_cplt;      /* port UART 接收完成回调函数 */
  port_os_thread_t      task;              /* port UART 接收任务句柄 */
  port_os_semaphore_t   binary;            /* port UART 接收信号 */
  port_os_stream_t      receive_buffer;    /* port UART 接收缓存区 */
  port_os_event_t       event_group;       /* port UART 事件标志组句柄 */
  port_uart_io_mode_e   io_mode;           /* port UART 模式 */
  port_uart_work_mode_e rx_work_mode;      /* port UART 接收端工作模式 */
  port_uart_work_mode_e tx_work_mode;      /* port UART 发送端工作模式 */
  uint32_t              receive_len;       /* port UART 接收计数 */
  uint64_t              receive_total_len; /* port UART 接收总数 */
  uint8_t*              receive_tmp0;      /* port UART 接收缓存区一  (中断与DMA使用) */
  uint8_t*              receive_tmp1;      /* port UART 接收缓存区二  (中断与DMA使用) */
  uint32_t              receive_tmp_len;   /* port UART 接收缓存长度  (中断使用) */
  uint32_t              receive_tmp_size;  /* port UART 接收缓存大小  (中断与DMA使用) */
  uint8_t*              send_buf;          /* port UART 发送缓存区指针 (DMA使用) */
  bool                  receive_tmp_num;   /* port UART 接收缓存区编号 */
  bool                  is_finished;       /* port UART 接收完成标志位 */
} port_uart_info_t;

/**
 * @brief (静态局部变量) port UART 串口信息结构体指针数组
 *
 */
static port_uart_info_t* s_apt_port_uart_info[UART_COUNT] = { 0 };

extern DMA_HandleTypeDef* pt_port_dma_get_handle(const uint8_t dma_num, const uint8_t dma_stream_num);

/**
 * @brief (静态) port UART 获取 HAL库 UART 通道句柄
 *
 * @param  uart_port      UART 通道编号
 * @return USART_TypeDef* UART 通道HAL库句柄
 */
static USART_TypeDef* s_pt_port_uart_get_instance(const uint8_t uart_port)
{
  switch (uart_port)
  {
    case 1 :
      return USART1;
    case 2 :
      return USART2;
    case 3 :
      return USART3;
    case 4 :
      return UART4;
    case 5 :
      return UART5;
    case 6 :
      return USART6;
    case 7 :
      return UART7;
    case 8 :
      return UART8;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get instance error!\n");
      return NULL;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART 字长
 *
 * @param  uart_word_length UART 字长
 * @return uint32_t         UART 字长HAL库编码
 */
static uint32_t s_ul_port_uart_get_word_length(const uint8_t uart_word_length)
{
  switch (uart_word_length)
  {
    case 8 :
      return UART_WORDLENGTH_8B;
    case 9 :
      return UART_WORDLENGTH_9B;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get word length error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART 停止位
 *
 * @param  uart_stop_bits UART 停止位
 * @return uint32_t       UART 停止位HAL库编码
 */
static uint32_t s_ul_port_uart_get_stop_bits(const uint8_t uart_stop_bits)
{
  switch (uart_stop_bits)
  {
    case 1 :
      return UART_STOPBITS_1;
    case 2 :
      return UART_STOPBITS_2;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get stop bits error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART 校验模式
 *
 * @param  uart_parity  UART 校验模式
 * @return uint32_t     UART 校验模式HAL库编码
 */
static uint32_t s_ul_port_uart_get_parity(const port_uart_parity_e uart_parity)
{
  switch (uart_parity)
  {
    case PORT_UART_NONE :
      return UART_PARITY_NONE;
    case PORT_UART_EVEN :
      return UART_PARITY_EVEN;
    case PORT_UART_ODD :
      return UART_PARITY_ODD;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get parity error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART 模式
 *
 * @param  uart_io_mode   UART 模式
 * @return uint32_t UART  模式HAL库编码
 */
static uint32_t s_ul_port_uart_get_mode(const port_uart_io_mode_e uart_io_mode)
{
  switch (uart_io_mode)
  {
    case PORT_UART_RX :
      return UART_MODE_RX;
    case PORT_UART_TX :
      return UART_MODE_TX;
    case PORT_UART_RX_TX :
      return UART_MODE_TX_RX;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get mode error!\n");
      return 0;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART 中断向量
 *
 * @param  uart_port  UART 中断向量
 * @return IRQn_Type  UART 中断向量HAL库编码
 */
static IRQn_Type s_e_port_uart_get_irqn(const uint8_t uart_port)
{
  switch (uart_port)
  {
    case 1 :
      return USART1_IRQn;
    case 2 :
      return USART2_IRQn;
    case 3 :
      return USART3_IRQn;
    case 4 :
      return UART4_IRQn;
    case 5 :
      return UART5_IRQn;
    case 6 :
      return USART6_IRQn;
    case 7 :
      return UART7_IRQn;
    case 8 :
      return UART8_IRQn;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get irqn error!\n");
      return (IRQn_Type)IRQN_ERROR;
  }
}

/**
 * @brief (静态) port UART HAL库 UART 时钟使能
 *
 * @param uart_port UART 通道编号
 */
static void s_v_port_uart_clock_enable(const uint8_t uart_port)
{
  switch (uart_port)
  {
    case 1 :
      __HAL_RCC_USART1_CLK_ENABLE();
      break;
    case 2 :
      __HAL_RCC_USART2_CLK_ENABLE();
      break;
    case 3 :
      __HAL_RCC_USART3_CLK_ENABLE();
      break;
    case 4 :
      __HAL_RCC_UART4_CLK_ENABLE();
      break;
    case 5 :
      __HAL_RCC_UART5_CLK_ENABLE();
      break;
    case 6 :
      __HAL_RCC_USART6_CLK_ENABLE();
      break;
    case 7 :
      __HAL_RCC_UART7_CLK_ENABLE();
      break;
    case 8 :
      __HAL_RCC_UART8_CLK_ENABLE();
      break;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart clock enable error!\n");
      break;
  }
}

/**
 * @brief (静态) port UART HAL库 UART 时钟失能
 *
 * @param uart_port UART 通道编号
 */
static void s_v_port_uart_clock_disable(const uint8_t uart_port)
{
  switch (uart_port)
  {
    case 1 :
      __HAL_RCC_USART1_CLK_DISABLE();
      break;
    case 2 :
      __HAL_RCC_USART2_CLK_DISABLE();
      break;
    case 3 :
      __HAL_RCC_USART3_CLK_DISABLE();
      break;
    case 4 :
      __HAL_RCC_UART4_CLK_DISABLE();
      break;
    case 5 :
      __HAL_RCC_UART5_CLK_DISABLE();
      break;
    case 6 :
      __HAL_RCC_USART6_CLK_DISABLE();
      break;
    case 7 :
      __HAL_RCC_UART7_CLK_DISABLE();
      break;
    case 8 :
      __HAL_RCC_UART8_CLK_DISABLE();
      break;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart clock disable error!\n");
      break;
  }
}

/**
 * @brief (静态) port UART 获取 HAL库 UART GPIO复用编码
 *
 * @param uart_port UART 通道编号
 * @return uint32_t UART HAL库GPIO复用编码
 */
static uint32_t s_ul_port_uart_get_gpio_af(const uint8_t uart_port)
{
  switch (uart_port)
  {
    case 1 :
      return GPIO_AF7_USART1;
    case 2 :
      return GPIO_AF7_USART2;
    case 3 :
      return GPIO_AF7_USART3;
    case 4 :
      return GPIO_AF8_UART4;
    case 5 :
      return GPIO_AF8_UART5;
    case 6 :
      return GPIO_AF8_USART6;
    case 7 :
      return GPIO_AF8_UART7;
    case 8 :
      return GPIO_AF8_UART8;

    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port uart get gpio af error!\n");
      return 0;
  }
}

/**
 * @brief (静态内联) port UART 清除中断标志位
 *
 * @param uart_port UART 通道编号
 */
inline static void sl_v_port_uart_clean_flag(const uint8_t uart_port)
{
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  if (NULL != uart_handle)
  {
    __HAL_UNLOCK(uart_handle);
    __HAL_UART_CLEAR_FLAG(uart_handle, UART_FLAG_RXNE);
    __HAL_UART_CLEAR_FLAG(uart_handle, UART_FLAG_IDLE);
    __HAL_UART_CLEAR_PEFLAG(uart_handle);
  }
}

/**
 * @brief (静态内联) port UART GPIO 初始化
 *
 * @param uart_port UART 通道编号
 */
inline static void sl_v_port_uart_gpio_init(const uint8_t uart_port)
{
  v_port_gpio_af_init(s_auc_port_uart_gpio_info[uart_port - 1][0], s_auc_port_uart_gpio_info[uart_port - 1][1], PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, s_ul_port_uart_get_gpio_af(uart_port));
  v_port_gpio_af_init(s_auc_port_uart_gpio_info[uart_port - 1][2], s_auc_port_uart_gpio_info[uart_port - 1][3], PORT_GPIO_AF_OD, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, s_ul_port_uart_get_gpio_af(uart_port));
}

/**
 * @brief (静态内联) port UART GPIO 解除初始化
 *
 * @param uart_port UART 通道编号
 */
inline static void sl_v_port_uart_gpio_deinit(const uint8_t uart_port)
{
  v_port_gpio_deinit(s_auc_port_uart_gpio_info[uart_port - 1][0], s_auc_port_uart_gpio_info[uart_port - 1][1], PORT_GPIO_AF_PP);
  v_port_gpio_deinit(s_auc_port_uart_gpio_info[uart_port - 1][2], s_auc_port_uart_gpio_info[uart_port - 1][3], PORT_GPIO_AF_OD);
}

/**
 * @brief (静态) port UART DMA 初始化
 *
 * @param  uart_port    UART 通道编号
 * @return error_code_e 错误代码
 */
static error_code_e s_e_port_uart_dma_init(const uint8_t uart_port)
{
  if (PORT_UART_TX == s_apt_port_uart_info[uart_port - 1]->io_mode || PORT_UART_RX_TX == s_apt_port_uart_info[uart_port - 1]->io_mode)
  {
    if (PORT_UART_DMA == s_apt_port_uart_info[uart_port - 1]->tx_work_mode || PORT_UART_DMA_DOUBLE == s_apt_port_uart_info[uart_port - 1]->tx_work_mode)
    {
      g_e_error_code = e_port_dma_init(s_auc_port_uart_dma_info[uart_port - 1][0], s_auc_port_uart_dma_info[uart_port - 1][1], s_auc_port_uart_dma_info[uart_port - 1][2], PORT_DMA_MEMORY_TO_PERIPH, PORT_DMA_NORMAL, PORT_DMA_MEDIUM);
      __HAL_LINKDMA(g_apt_port_uart_handle[uart_port - 1], hdmatx, *pt_port_dma_get_handle(s_auc_port_uart_dma_info[uart_port - 1][0], s_auc_port_uart_dma_info[uart_port - 1][1]));
    }
  }

  if (PORT_UART_RX == s_apt_port_uart_info[uart_port - 1]->io_mode || PORT_UART_RX_TX == s_apt_port_uart_info[uart_port - 1]->io_mode)
  {
    if (PORT_UART_DMA == s_apt_port_uart_info[uart_port - 1]->rx_work_mode)
    {
      g_e_error_code = e_port_dma_init(s_auc_port_uart_dma_info[uart_port - 1][3], s_auc_port_uart_dma_info[uart_port - 1][4], s_auc_port_uart_dma_info[uart_port - 1][5], PORT_DMA_PERIPH_TO_MEMORY, PORT_DMA_NORMAL, PORT_DMA_MEDIUM);
      __HAL_LINKDMA(g_apt_port_uart_handle[uart_port - 1], hdmarx, *pt_port_dma_get_handle(s_auc_port_uart_dma_info[uart_port - 1][3], s_auc_port_uart_dma_info[uart_port - 1][4]));
    }
    else if (PORT_UART_DMA_DOUBLE == s_apt_port_uart_info[uart_port - 1]->rx_work_mode)
    {
      g_e_error_code = e_port_dma_init(s_auc_port_uart_dma_info[uart_port - 1][3], s_auc_port_uart_dma_info[uart_port - 1][4], s_auc_port_uart_dma_info[uart_port - 1][5], PORT_DMA_PERIPH_TO_MEMORY, PORT_DMA_CIRCULAR, PORT_DMA_MEDIUM);
      __HAL_LINKDMA(g_apt_port_uart_handle[uart_port - 1], hdmarx, *pt_port_dma_get_handle(s_auc_port_uart_dma_info[uart_port - 1][3], s_auc_port_uart_dma_info[uart_port - 1][4]));
    }
  }

  return g_e_error_code;
}

/**
 * @brief (静态内联) port UART DMA 解除初始化
 *
 * @param  uart_port    UART 通道编号
 * @return error_code_e 错误代码
 */
inline static error_code_e sl_e_port_uart_dma_deinit(const uint8_t uart_port)
{
  g_e_error_code = e_port_dma_deinit(s_auc_port_uart_dma_info[uart_port - 1][0], s_auc_port_uart_dma_info[uart_port - 1][1]);
  g_e_error_code = e_port_dma_deinit(s_auc_port_uart_dma_info[uart_port - 1][3], s_auc_port_uart_dma_info[uart_port - 1][4]);

  return g_e_error_code;
}

/**
 * @brief (静态) port UART 端口 初始化
 *
 * @param  uart_port          UART 通道编号
 * @param  uart_baud_rate     UART 波特率
 * @param  uart_word_length   UART 字长
 * @param  uart_stop_bits     UART 停止位
 * @param  uart_parity        UART 校验模式
 * @param  uart_io_mode       UART 模式
 * @return error_code_e       错误代码
 */
static error_code_e s_e_port_uart_base_init(const uint8_t uart_port, const uint32_t uart_baud_rate, const uint8_t uart_word_length, const uint8_t uart_stop_bits, const port_uart_parity_e uart_parity, const port_uart_io_mode_e uart_io_mode)
{
  /* UART 句柄结构体动态内存申请 */
  UART_HandleTypeDef* uart_handle = (UART_HandleTypeDef*)Malloc(sizeof(UART_HandleTypeDef));
  if (NULL == uart_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port uart handle Malloc failed!\n");
    return g_e_error_code;
  }

  IRQn_Type uart_irqn = s_e_port_uart_get_irqn(uart_port);
  memset(uart_handle, 0, sizeof(UART_HandleTypeDef));
  /* UART 句柄指针保存 */
  g_apt_port_uart_handle[uart_port - 1] = uart_handle;

  /* UART 时钟使能 */
  s_v_port_uart_clock_enable(uart_port);
  /* UART 初始化结构体填充 */
  uart_handle->Instance          = s_pt_port_uart_get_instance(uart_port);
  uart_handle->Init.BaudRate     = uart_baud_rate;
  uart_handle->Init.WordLength   = s_ul_port_uart_get_word_length(uart_word_length);
  uart_handle->Init.StopBits     = s_ul_port_uart_get_stop_bits(uart_stop_bits);
  uart_handle->Init.Parity       = s_ul_port_uart_get_parity(uart_parity);
  uart_handle->Init.Mode         = s_ul_port_uart_get_mode(uart_io_mode);
  uart_handle->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uart_handle->Init.OverSampling = UART_OVERSAMPLING_16;

  /* UART 中断失能 */
  HAL_NVIC_DisableIRQ(uart_irqn);
  HAL_NVIC_ClearPendingIRQ(uart_irqn);

  /* UART GPIO解除初始化 */
  sl_v_port_uart_gpio_deinit(uart_port);

  /* UART 解除初始化 */
  if (HAL_OK != HAL_UART_DeInit(uart_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port uart base deinit failed!\n");
    goto uart_base_err;
  }

  /* UART GPIO初始化 */
  sl_v_port_uart_gpio_init(uart_port);

  /* UART 初始化 */
  if (HAL_OK != HAL_UART_Init(uart_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port uart base init failed!\n");
    goto uart_base_err;
  }

  /* UART 中断使能 */
  if (PORT_UART_RX == uart_io_mode || PORT_UART_RX_TX == uart_io_mode)
  {
    HAL_NVIC_SetPriority(uart_irqn, UART_NVIC_DEF_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(uart_irqn);
  }

  return SUCESS;
/* UART 端口初始化故障处理 */
uart_base_err:
  memset(uart_handle, 0, sizeof(UART_HandleTypeDef));
  Free(uart_handle);
  g_apt_port_uart_handle[uart_port - 1] = NULL;
  return g_e_error_code;
}

/**
 * @brief (静态) port UART 端口 解除初始化
 *
 * @param  uart_port    UART 通道编号
 * @return error_code_e 错误代码
 */
static error_code_e s_e_port_uart_base_deinit(const uint8_t uart_port)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle)
    return SUCESS;

  IRQn_Type uart_irqn = s_e_port_uart_get_irqn(uart_port);
  /* UART 中断失能 */
  HAL_NVIC_DisableIRQ(uart_irqn);
  HAL_NVIC_ClearPendingIRQ(uart_irqn);

  /* UART GPIO解除初始化 */
  sl_v_port_uart_gpio_deinit(uart_port);

  /* UART 解除初始化 */
  if (HAL_OK != HAL_UART_DeInit(uart_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port uart base deinit failed!\n");
  }

  /* UART 时钟失能 */
  s_v_port_uart_clock_disable(uart_port);

  /* UART 句柄结构体内存清理与释放 */
  memset(uart_handle, 0, sizeof(UART_HandleTypeDef));
  Free(uart_handle);
  g_apt_port_uart_handle[uart_port - 1] = NULL;

  return g_e_error_code;
}

/**
 * @brief port UART 接收 中断事件 处理函数
 *
 * @param uart_port UART 通道编号
 */
void v_port_uart_irq_function(const uint8_t uart_port)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
    return;

  /* UART 中断接收模式 */
  if (PORT_UART_IT == uart_info->rx_work_mode)
  {
    /* UART 接收中断 */
    if (SET == __HAL_UART_GET_FLAG(uart_handle, UART_FLAG_RXNE))
    {
      if (uart_info->receive_tmp_len < uart_info->receive_tmp_size)
      {
        /* UART 读取接收数据 */
        if (!uart_info->receive_tmp_num)
          HAL_UART_Receive(uart_handle, &uart_info->receive_tmp0[uart_info->receive_tmp_len], 1, 1000);
        else
          HAL_UART_Receive(uart_handle, &uart_info->receive_tmp1[uart_info->receive_tmp_len], 1, 1000);
        /* UART 接收数据长度加 1 */
        uart_info->receive_tmp_len++;
      }
      /* UART 清空中断标志位 */
      sl_v_port_uart_clean_flag(uart_port);
    }
    /* UART 空闲中断 */
    else if (SET == __HAL_UART_GET_FLAG(uart_handle, UART_FLAG_IDLE))
    {
      /* UART 接收统计 */
      uart_info->receive_len     = uart_info->receive_tmp_len;
      uart_info->receive_tmp_len = 0;
      uart_info->receive_tmp_num = !uart_info->receive_tmp_num;
      uart_info->is_finished     = true;
      /* UART 清空中断标志位 */
      sl_v_port_uart_clean_flag(uart_port);
      /* UART 发送信号量 */
      b_port_os_semaphore_give(uart_info->binary);
    }
  }
  /* UART DMA接收模式 */
  else if (PORT_UART_DMA == uart_info->rx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->rx_work_mode)
  {
    /* UART 空闲中断 */
    if (SET == __HAL_UART_GET_FLAG(uart_handle, UART_FLAG_IDLE))
    {
      /* UART 关闭DMA */
      HAL_UART_DMAStop(uart_handle);

      /* UART 接收统计 */
      uart_info->receive_len = uart_info->receive_tmp_size - __HAL_DMA_GET_COUNTER(uart_handle->hdmarx);
      uart_info->is_finished = true;

      /* UART 清空中断标志位 */
      sl_v_port_uart_clean_flag(uart_port);

      /* UART 重启DMA */
      if (PORT_UART_DMA == uart_info->rx_work_mode)
      {
        uart_info->receive_tmp_num = !uart_info->receive_tmp_num;
        if (!uart_info->receive_tmp_num)
          HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp_size);
        else
          HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp1, uart_info->receive_tmp_size);
      }
      else
      {
        if (RESET == (((DMA_Stream_TypeDef*)(uart_handle->hdmarx->Instance))->CR & DMA_SxCR_CT))
          uart_info->receive_tmp_num = 1;
        else
          uart_info->receive_tmp_num = 0;
        HAL_UART_Receive_DMA_Double(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp1, uart_info->receive_tmp_size);
      }

      /* UART 发送信号量 */
      b_port_os_semaphore_give(uart_info->binary);
    }
  }
}

/**
 * @brief (静态) port UART 发送 完成回调事件 处理函数
 *
 * @param uart_port UART 通道编号
 */
static void s_v_port_uart_cplt_callback(const uint8_t uart_port)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
    return;

  /* UART 清空中断标志位 */
  sl_v_port_uart_clean_flag(uart_port);

  if (PORT_UART_DMA == uart_info->rx_work_mode)
  {
    /* UART 重启DMA */
    if (!uart_info->receive_tmp_num)
      HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp_size);
    else
      HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp1, uart_info->receive_tmp_size);
  }
  else if (PORT_UART_DMA_DOUBLE == uart_info->rx_work_mode)
  {
    /* UART 重启DMA */
    HAL_UART_Receive_DMA_Double(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp1, uart_info->receive_tmp_size);
  }

  /* UART 执行回调函数 */
  if (NULL != uart_info->uart_tx_cplt.function)
    uart_info->uart_tx_cplt.function(uart_info->uart_tx_cplt.arg);

  /* UART 设置发送事件完成事件标志位 */
  if (PORT_UART_IT == uart_info->tx_work_mode)
    ul_port_os_event_set(uart_info->event_group, UART_SEND_CPLT_BIT);
  else if (PORT_UART_DMA == uart_info->tx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->tx_work_mode)
  {
    Free(uart_info->send_buf);
    uart_info->send_buf = NULL;
    ul_port_os_event_set(uart_info->event_group, UART_SEND_CPLT_BIT);
  }
}

/**
 * @brief (静态) port UART Double DMA 接收 中断事件 处理函数
 *
 * @param uart_port  UART 通道编号
 * @param memory_num UART 缓存区编号
 */
static void s_v_port_uart_dma_double_callback(const uint8_t uart_port, const uint8_t memory_num)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
    return;

  /* UART 接收统计 */
  uart_info->receive_len     = uart_info->receive_tmp_size;
  uart_info->receive_tmp_num = !uart_info->receive_tmp_num;
  /* UART 发送信号量 */
  b_port_os_semaphore_give(uart_info->binary);
}

/**
 * @brief port UART 接收 任务
 *
 * @param arg 传入参数
 */
void v_port_uart_rx_task(void* arg)
{
  /* UART 获取、初始化变量 */
  port_uart_info_t* uart_info = (port_uart_info_t*)arg;
  uint32_t          len       = 0;

  while (1)
  {
    /* UART 等待信号量 */
    if (true == b_port_os_semaphore_take(uart_info->binary, WAIT_FOREVER))
    {
      /* UART 接收长度统计 */
      len = uart_info->receive_len;
      if (uart_info->receive_total_len > INT64_MAX - len)
        uart_info->receive_total_len = 0;
      else
        uart_info->receive_total_len += len;

      /* UART 接收数据并发送到流缓存区 */
      if (uart_info->receive_tmp_num)
        ul_port_os_stream_send(uart_info->receive_buffer, uart_info->receive_tmp0, len);
      else
        ul_port_os_stream_send(uart_info->receive_buffer, uart_info->receive_tmp1, len);

      /* UART 设置接收事件完成事件标志位 */
      if (uart_info->is_finished)
      {
        if (NULL != uart_info->uart_rx_cplt.arg)
          uart_info->uart_rx_cplt.function(uart_info->uart_rx_cplt.arg);

        uart_info->is_finished = false;
        ul_port_os_event_set(uart_info->event_group, UART_RECI_CPLT_BIT);
      }
    }
  }
}

/**
 * @brief port UART 资源清空
 *
 * @param uart_port UART 通道编号
 */
void v_port_uart_clean(uint8_t uart_port)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
    return;

  if (PORT_UART_RX == uart_info->io_mode || PORT_UART_RX_TX == uart_info->io_mode)
  {
    IRQn_Type uart_irqn = s_e_port_uart_get_irqn(uart_port);
    /* UART 中断失能 */
    HAL_NVIC_DisableIRQ(uart_irqn);

    /* UART 关闭DMA */
    if (PORT_UART_DMA == uart_info->rx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->rx_work_mode)
      HAL_UART_DMAStop(uart_handle);

    /* UART 清空中断标志位 */
    sl_v_port_uart_clean_flag(uart_port);
    /* UART 清空事件标志组 */
    ul_port_os_event_clear(uart_info->event_group, UART_RECI_CPLT_BIT | UART_SEND_CPLT_BIT);
    /* UART 清空接收流缓存区 */
    b_port_os_stream_reset(uart_info->receive_buffer);
    uart_info->receive_total_len = 0;
    uart_info->receive_len       = 0;
    uart_info->receive_tmp_len   = 0;
    uart_info->is_finished       = false;

    /* UART 中断使能 */
    HAL_NVIC_EnableIRQ(uart_irqn);

    /* UART 重启DMA */
    if (PORT_UART_DMA == uart_info->rx_work_mode)
    {
      if (!uart_info->receive_tmp_num)
        HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp_size);
      else
        HAL_UART_Receive_DMA(uart_handle, uart_info->receive_tmp1, uart_info->receive_tmp_size);
    }
    else if (PORT_UART_DMA_DOUBLE == uart_info->rx_work_mode)
      HAL_UART_Receive_DMA_Double(uart_handle, uart_info->receive_tmp0, uart_info->receive_tmp1, uart_info->receive_tmp_size);
  }
}

/**
 * @brief   port UART 重新初始化
 * @warning 用于修改参数,无时钟使能,无GPIO初始化
 *
 * @param  uart_port        UART 通道编号
 * @param  uart_baud_rate   UART 波特率
 * @param  uart_word_length UART 字长
 * @param  uart_stop_bits   UART 停止位
 * @param  uart_parity      UART 校验模式
 * @return error_code_e     错误代码
 */
error_code_e e_port_uart_reinit(uint8_t uart_port, uint32_t uart_baud_rate, uint8_t uart_word_length, uint8_t uart_stop_bits, port_uart_parity_e uart_parity)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port uart handle is not init!\n");
    return g_e_error_code;
  }

  IRQn_Type uart_irqn          = s_e_port_uart_get_irqn(uart_port);

  /* UART 初始化结构体填充 */
  uart_handle->Init.BaudRate   = uart_baud_rate;
  uart_handle->Init.WordLength = s_ul_port_uart_get_word_length(uart_word_length);
  uart_handle->Init.StopBits   = s_ul_port_uart_get_stop_bits(uart_stop_bits);
  uart_handle->Init.Parity     = s_ul_port_uart_get_parity(uart_parity);

  /* UART 中断失能 */
  HAL_NVIC_DisableIRQ(uart_irqn);
  HAL_NVIC_ClearPendingIRQ(uart_irqn);

  /* UART 解除初始化 */
  if (HAL_OK != HAL_UART_DeInit(uart_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port uart re-deinit failed!\n");
    goto uart_reinit_err;
  }

  /* UART 重新初始化 */
  if (HAL_OK != HAL_UART_Init(uart_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port uart re-init failed!\n");
    goto uart_reinit_err;
  }

  /* UART 中断使能 */
  if (UART_MODE_RX == uart_handle->Init.Mode || UART_MODE_TX_RX == uart_handle->Init.Mode)
  {
    HAL_NVIC_SetPriority(uart_irqn, UART_NVIC_DEF_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(uart_irqn);
  }

  return SUCESS;
/* UART 重新初始化故障处理 */
uart_reinit_err:
  g_apt_port_uart_handle[uart_port - 1] = NULL;
  memset(uart_handle, 0, sizeof(UART_HandleTypeDef));
  Free(uart_handle);
  return g_e_error_code;
}

/**
 * @brief  port UART 初始化
 *
 * @param  uart_port              UART 通道编号
 * @param  uart_baud_rate         UART 波特率
 * @param  uart_word_length       UART 字长
 * @param  uart_stop_bits         UART 停止位
 * @param  uart_parity            UART 校验模式
 * @param  uart_io_mode           UART 模式
 * @param  uart_rx_work_mode      UART 接收工作模式
 * @param  uart_tx_work_mode      UART 发送工作模式
 * @param  uart_receive_buf       UART 接收缓存区
 * @param  uart_receive_buf_size  UART 接收缓存区大小 (tmp大小)
 * @param  uart_rx_cplt           UART 接收完成回调函数
 * @param  uart_tx_cplt           UART 发送完成回调函数
 * @return error_code_e           错误代码
 */
error_code_e e_port_uart_init(const uint8_t uart_port, const uint32_t uart_baud_rate, const uint8_t uart_word_length, const uint8_t uart_stop_bits, const port_uart_parity_e uart_parity, const port_uart_io_mode_e uart_io_mode, const port_uart_work_mode_e uart_rx_work_mode, const port_uart_work_mode_e uart_tx_work_mode, port_os_stream_t uart_receive_buf, const uint32_t uart_receive_buf_size, const port_uart_callback_t* uart_rx_cplt, const port_uart_callback_t* uart_tx_cplt)
{
  /* UART 是否已经初始化判断 */
  if (NULL != s_apt_port_uart_info[uart_port - 1])
  {
    g_e_error_code = ALREADY_INIT_ERROR;
    ERROR_HANDLE("port uart already initialized!\n");
    return g_e_error_code;
  }

  /* UART 信息结构体动态内存申请 */
  port_uart_info_t* uart_info = (port_uart_info_t*)Malloc(sizeof(port_uart_info_t));
  if (NULL == uart_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port uart info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(uart_info, 0, sizeof(port_uart_info_t));
  /* UART 通道信息保存 */
  s_apt_port_uart_info[uart_port - 1] = uart_info;

  /* UART 事件标志组申请 */
  uart_info->event_group              = pt_port_os_event_create();
  if (NULL == uart_info->event_group)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port uart event_group init failed!\n");
    goto uart_init_err;
  }

  /* UART 模式赋值 */
  uart_info->io_mode      = uart_io_mode;
  uart_info->rx_work_mode = uart_rx_work_mode;
  uart_info->tx_work_mode = uart_tx_work_mode;

  /* UART 端口初始化 */
  if (SUCESS != s_e_port_uart_base_init(uart_port, uart_baud_rate, uart_word_length, uart_stop_bits, uart_parity, uart_io_mode))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port uart base init failed!\n");
    goto uart_init_err;
  }

  /* UART DMA初始化 */
  if (PORT_UART_DMA == uart_rx_work_mode || PORT_UART_DMA == uart_tx_work_mode || PORT_UART_DMA_DOUBLE == uart_rx_work_mode || PORT_UART_DMA_DOUBLE == uart_tx_work_mode)
  {
    if (SUCESS != s_e_port_uart_dma_init(uart_port))
    {
      g_e_error_code = INIT_ERROR;
      ERROR_HANDLE("port uart dma init failed!\n");
      goto uart_init_err;
    }
  }

  if (PORT_UART_TX == uart_io_mode || PORT_UART_RX_TX == uart_io_mode)
  {
    /* UART 发送回调函数赋值 */
    if (NULL != uart_tx_cplt)
      memcpy(&uart_info->uart_tx_cplt, uart_tx_cplt, sizeof(port_uart_callback_t));
    else
      memset(&uart_info->uart_tx_cplt, 0, sizeof(port_uart_callback_t));
  }

  if (PORT_UART_RX == uart_io_mode || PORT_UART_RX_TX == uart_io_mode)
  {
    /* UART 接收回调函数赋值 */
    if (NULL != uart_rx_cplt)
      memcpy(&uart_info->uart_rx_cplt, uart_rx_cplt, sizeof(port_uart_callback_t));
    else
      memset(&uart_info->uart_rx_cplt, 0, sizeof(port_uart_callback_t));

    /* UART 接收缓存区一动态内存申请 */
    uart_info->receive_tmp0 = (uint8_t*)Malloc(uart_receive_buf_size);
    if (NULL == uart_info->receive_tmp0)
    {
      g_e_error_code = MEMORY_ERROR;
      ERROR_HANDLE("port uart receive_tmp0 Malloc failed!\n");
      goto uart_init_err;
    }
    else
      memset(uart_info->receive_tmp0, 0, uart_receive_buf_size);

    /* UART 接收缓存区二动态内存申请 */
    uart_info->receive_tmp1 = (uint8_t*)Malloc(uart_receive_buf_size);
    if (NULL == uart_info->receive_tmp1)
    {
      g_e_error_code = MEMORY_ERROR;
      ERROR_HANDLE("port uart receive_tmp1 Malloc failed!\n");
      goto uart_init_err;
    }
    else
      memset(uart_info->receive_tmp1, 0, uart_receive_buf_size);

    /* UART 接收缓存区赋值 */
    if (NULL == uart_receive_buf)
    {
      g_e_error_code = NULL_POINTER_ERROR;
      ERROR_HANDLE("port uart receive_buf pointer is null!\n");
      goto uart_init_err;
    }
    else
      uart_info->receive_buffer = uart_receive_buf;

    /* UART 创建接收信号量 */
    uart_info->binary = pt_port_os_semaphore_create(1, 1);
    if (NULL == uart_info->binary)
    {
      g_e_error_code = INIT_ERROR;
      ERROR_HANDLE("port uart binaryaphore init failed!\n");
      goto uart_init_err;
    }

    /* UART 创建接收任务 */
    char task_name[32] = "uart _tx_task";
    task_name[4]       = uart_port + '0';
    uart_info->task    = pt_port_os_thread_create(task_name, UART_TASK_DEF_STACK, UART_TASK_DEF_PRIORITY, v_port_uart_rx_task, uart_info);
    if (NULL == uart_info->task)
    {
      g_e_error_code = INIT_ERROR;
      ERROR_HANDLE("port uart receieve task init failed!\n");
      goto uart_init_err;
    }

    /* UART 初始值设置 */
    uart_info->receive_len       = 0;
    uart_info->receive_total_len = 0;
    uart_info->receive_tmp_len   = 0;
    uart_info->receive_tmp_num   = 0;
    uart_info->receive_tmp_size  = uart_receive_buf_size;
    uart_info->is_finished       = false;
    uart_info->send_buf          = NULL;

    /* UART 清空中断标志位 */
    sl_v_port_uart_clean_flag(uart_port);
    if (PORT_UART_IT == uart_rx_work_mode)
    {
      /* UART 使能中断 */
      __HAL_UART_ENABLE_IT(g_apt_port_uart_handle[uart_port - 1], UART_IT_RXNE);
      __HAL_UART_ENABLE_IT(g_apt_port_uart_handle[uart_port - 1], UART_IT_IDLE);
    }
    else if (PORT_UART_DMA == uart_rx_work_mode)
    {
      /* UART 开启DMA */
      HAL_UART_Receive_DMA(g_apt_port_uart_handle[uart_port - 1], uart_info->receive_tmp0, uart_info->receive_tmp_size);
      /* UART 使能空闲中断 */
      __HAL_UART_ENABLE_IT(g_apt_port_uart_handle[uart_port - 1], UART_IT_IDLE);
    }
    else if (PORT_UART_DMA_DOUBLE == uart_rx_work_mode)
    {
      /* UART 开启DMA */
      HAL_UART_Receive_DMA_Double(g_apt_port_uart_handle[uart_port - 1], uart_info->receive_tmp0, uart_info->receive_tmp1, uart_info->receive_tmp_size);
      /* UART 使能空闲中断 */
      __HAL_UART_ENABLE_IT(g_apt_port_uart_handle[uart_port - 1], UART_IT_IDLE);
    }
  }

  return SUCESS;
/* UART 初始化故障处理 */
uart_init_err:
  if (NULL != uart_info->event_group)
    v_port_os_event_delete(uart_info->event_group);
  if (NULL != uart_info->receive_tmp0)
    Free(uart_info->receive_tmp0);
  if (NULL != uart_info->receive_tmp1)
    Free(uart_info->receive_tmp1);
  if (NULL != uart_info->binary)
    v_port_os_semaphore_delete(uart_info->binary);
  if (NULL != uart_info->task)
    v_port_os_thread_delete(uart_info->task);
  Free(uart_info);
  s_apt_port_uart_info[uart_port - 1] = NULL;

  return g_e_error_code;
}

/**
 * @brief port UART 发送
 *
 * @param  uart_port    UART 通道编号
 * @param  data         发送数据缓存区指针
 * @param  len          发送数据长度
 * @return error_code_e 错误代码
 */
error_code_e e_port_uart_send(const uint8_t uart_port, const void* data, uint16_t len)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port uart handle is not init!\n");
    return g_e_error_code;
  }

  if (PORT_UART_TX == uart_info->io_mode || PORT_UART_RX_TX == uart_info->io_mode)
  {
    /* UART 发送数据指针判断 */
    if (NULL == data || 0 == len)
    {
      g_e_error_code = NULL_POINTER_ERROR;
      ERROR_HANDLE("port uart data or len is null!\n");
      return g_e_error_code;
    }

    if (PORT_UART_IT == uart_info->tx_work_mode || len < 64)
    {
      /* UART 阻塞模式发送 */
      if (HAL_UART_Transmit(uart_handle, data, len, 1000))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port uart transmit failed!\n");
        return g_e_error_code;
      }
      /* UART 跳转回调函数 */
      s_v_port_uart_cplt_callback(uart_port);
    }
    else if (PORT_UART_DMA == uart_info->tx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->tx_work_mode)
    {
      /* UART DMA数据保护区申请 */
      uart_info->send_buf = Malloc(len);
      memcpy(uart_info->send_buf, data, len);
      /* UART DMA模式发送 */
      if (HAL_UART_Transmit_DMA(uart_handle, data, len))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port uart transmit failed!\n");
        return g_e_error_code;
      }
    }
  }

  return SUCESS;
}

/**
 * @brief port UART 接收计数(字节)
 *
 * @param  uart_port UART 通道编号
 * @return uint64_t  UART 接收总数(字节)
 */
uint64_t ull_port_uart_total_received(const uint8_t uart_port)
{
  /* UART 获取指针 */
  port_uart_info_t* uart_info = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_info)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port uart handle is not init!\n");
    return 0;
  }

  return uart_info->receive_total_len;
}

/**
 * @brief port UART 等待接收完成
 *
 * @param  uart_port    UART 通道编号
 * @param  waiting_time 等待时间(ms)
 * @return true         接收完成
 * @return false        等待超时
 */
bool b_port_uart_wait_receive_complete(const uint8_t uart_port, uint32_t waiting_time)
{
  /* UART 获取指针 */
  port_uart_info_t* uart_info = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port uart info is null!\n");
    return false;
  }

  /* UART 等待接收完成 */
  return (ul_port_os_event_wait(uart_info->event_group, UART_RECI_CPLT_BIT, waiting_time, false) & UART_RECI_CPLT_BIT);
}

/**
 * @brief port UART 清除接收完成标志位
 *
 * @param  uart_port UART 通道编号
 * @return true      成功
 * @return false     失败
 */
bool b_port_uart_clean_receive_complete(const uint8_t uart_port)
{
  /* UART 获取指针 */
  port_uart_info_t* uart_info = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port uart info is null!\n");
    return false;
  }

  /* UART 清空接收完成标志位 */
  return (ul_port_os_event_clear(uart_info->event_group, UART_RECI_CPLT_BIT) & UART_RECI_CPLT_BIT);
}

/**
 * @brief port UART 等待发送完成
 *
 * @param  uart_port    UART 通道编号
 * @param  waiting_time 等待时间(ms)
 * @return true         发送完成
 * @return false        等待超时
 */
bool b_port_uart_wait_send_complete(const uint8_t uart_port, uint32_t waiting_time)
{
  /* UART 获取指针 */
  port_uart_info_t* uart_info = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port uart info is null!\n");
    return false;
  }

  /* UART 等待发送完成 */
  uint32_t bit = ul_port_os_event_wait(uart_info->event_group, UART_SEND_CPLT_BIT, waiting_time, false);
  if (0 != (bit & UART_SEND_CPLT_BIT))
  {
    ul_port_os_event_clear(uart_info->event_group, UART_SEND_CPLT_BIT);
    return true;
  }

  return false;
}

/**
 * @brief port UART 解除初始化
 *
 * @param  uart_port    UART 通道编号
 * @return error_code_e 错误代码
 */
error_code_e e_port_uart_deinit(const uint8_t uart_port)
{
  /* UART 获取指针 */
  UART_HandleTypeDef* uart_handle = g_apt_port_uart_handle[uart_port - 1];
  port_uart_info_t*   uart_info   = s_apt_port_uart_info[uart_port - 1];

  /* UART 空指针判断 */
  if (NULL == uart_handle || NULL == uart_info)
    return SUCESS;

  /* UART 关闭DMA */
  if (PORT_UART_DMA == uart_info->rx_work_mode || PORT_UART_DMA == uart_info->tx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->rx_work_mode || PORT_UART_DMA_DOUBLE == uart_info->tx_work_mode)
  {
    HAL_UART_DMAStop(uart_handle);
    g_e_error_code = sl_e_port_uart_dma_deinit(uart_port);
  }

  /* UART 端口解除初始化 */
  g_e_error_code = s_e_port_uart_base_deinit(uart_port);

  /* UART 内存清理与释放 */
  if (uart_info->binary)
    v_port_os_semaphore_delete(uart_info->binary);
  if (uart_info->task)
    v_port_os_thread_delete(uart_info->task);
  if (uart_info->event_group)
    v_port_os_event_delete(uart_info->event_group);
  if (uart_info->receive_tmp0)
  {
    memset(uart_info->receive_tmp0, 0, uart_info->receive_tmp_size);
    Free(uart_info->receive_tmp0);
  }
  if (uart_info->receive_tmp1)
  {
    memset(uart_info->receive_tmp1, 0, uart_info->receive_tmp_size);
    Free(uart_info->receive_tmp1);
  }
  if (uart_info->send_buf)
    Free(uart_info->send_buf);

  /* UART 句柄结构体内存清理与释放 */
  memset(uart_info, 0, sizeof(port_uart_info_t));
  Free(uart_info);
  s_apt_port_uart_info[uart_port - 1] = NULL;

  return g_e_error_code;
}

/**
 * @brief port UART HAL库 发送完成中断回调函数
 *
 * @param huart UART HAL库句柄结构体指针
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
  if (USART1 == huart->Instance)
    s_v_port_uart_cplt_callback(1);
  else if (USART2 == huart->Instance)
    s_v_port_uart_cplt_callback(2);
  else if (USART3 == huart->Instance)
    s_v_port_uart_cplt_callback(3);
  else if (UART4 == huart->Instance)
    s_v_port_uart_cplt_callback(4);
  else if (UART5 == huart->Instance)
    s_v_port_uart_cplt_callback(5);
  else if (USART6 == huart->Instance)
    s_v_port_uart_cplt_callback(6);
  else if (UART7 == huart->Instance)
    s_v_port_uart_cplt_callback(7);
  else if (UART8 == huart->Instance)
    s_v_port_uart_cplt_callback(8);
}

/**
 * @brief port UART HAL库 DMA双缓冲 缓存区0 接收完成中断回调函数
 *
 * @param huart UART HAL库句柄结构体指针·
 */
void HAL_UART_DMA_M0_RxCpltCallback(UART_HandleTypeDef* huart)
{
  if (USART1 == huart->Instance)
    s_v_port_uart_dma_double_callback(1, 0);
  else if (USART2 == huart->Instance)
    s_v_port_uart_dma_double_callback(2, 0);
  else if (USART3 == huart->Instance)
    s_v_port_uart_dma_double_callback(3, 0);
  else if (UART4 == huart->Instance)
    s_v_port_uart_dma_double_callback(4, 0);
  else if (UART5 == huart->Instance)
    s_v_port_uart_dma_double_callback(5, 0);
  else if (USART6 == huart->Instance)
    s_v_port_uart_dma_double_callback(6, 0);
  else if (UART7 == huart->Instance)
    s_v_port_uart_dma_double_callback(7, 0);
  else if (UART8 == huart->Instance)
    s_v_port_uart_dma_double_callback(8, 0);
}

/**
 * @brief port UART HAL库 DMA双缓冲 缓存区1 接收完成中断回调函数
 *
 * @param huart UART HAL库句柄结构体指针·
 */
void HAL_UART_DMA_M1_RxCpltCallback(UART_HandleTypeDef* huart)
{
  if (USART1 == huart->Instance)
    s_v_port_uart_dma_double_callback(1, 1);
  else if (USART2 == huart->Instance)
    s_v_port_uart_dma_double_callback(2, 1);
  else if (USART3 == huart->Instance)
    s_v_port_uart_dma_double_callback(3, 1);
  else if (UART4 == huart->Instance)
    s_v_port_uart_dma_double_callback(4, 1);
  else if (UART5 == huart->Instance)
    s_v_port_uart_dma_double_callback(5, 1);
  else if (USART6 == huart->Instance)
    s_v_port_uart_dma_double_callback(6, 1);
  else if (UART7 == huart->Instance)
    s_v_port_uart_dma_double_callback(7, 1);
  else if (UART8 == huart->Instance)
    s_v_port_uart_dma_double_callback(8, 1);
}