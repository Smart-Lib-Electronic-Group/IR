#include "port_spi.h"
#include "port_dma.h"
#include "port_include.h"

/// @brief port SPI 中断默认优先级
#define SPI_NVIC_DEF_PRIORITY PORT_OS_INTERRUPT_PRIORITY

/**
 *
 * @brief (全局变量) port SPI HAL 句柄指针数组
 *
 */
SPI_HandleTypeDef* g_apt_port_spi_handle[SPI_COUNT]          = { 0 };

/**
 * @brief (静态常量) port SPI 引脚信息数组
 *
 */
static const uint8_t sc_auc_port_spi_gpio_info[SPI_COUNT][6] = {
  { 'A', 5,  'A', 6,  'B', 5  }, /* SPI1 SCLK-PA-05 MISO-PA-06 MOSI-PB-05 */
  { 'B', 13, 'B', 14, 'B', 15 }, /* SPI2 SCLK-PB-13 MISO-PB-14 MOSI-PB-15 */
  { 'B', 3,  'B', 4,  'B', 5  }, /* SPI3 SCLK-PB-03 MISO-PB-04 MOSI-PB-05 */
  { 'E', 2,  'E', 5,  'E', 6  }, /* SPI4 SCLK-PE-02 MISO-PE-05 MOSI-PE-06 */
  { 'F', 7,  'F', 8,  'F', 9  }, /* SPI5 SCLK-PF-07 MISO-PF-08 MOSI-PF-09 */
  { 'G', 12, 'G', 13, 'G', 14 }, /* SPI6 SCLK-PG-12 MISO-PG-13 MOSI-PG-14 */
};

/**
 * @brief (静态常量) port SPI DMA信息数组
 *
 */
static const uint8_t sc_auc_port_spi_dma_info[SPI_COUNT][6] = {
  { 2, 3, 3, 2, 0, 3 }, /* SP1  TX -> DMA2-Stream3-Channel3  RX -> DMA2-Stream0-Channel3 */
  { 1, 4, 0, 1, 3, 0 }, /* SP2  TX -> DMA1-Stream4-Channel0  RX -> DMA1-Stream0-Channel1 */
  { 1, 5, 0, 1, 2, 0 }, /* SP3  TX -> DMA1-Stream5-Channel0  RX -> DMA1-Stream0-Channel2 */
  { 2, 4, 5, 2, 3, 5 }, /* SP4  TX -> DMA2-Stream4-Channel5  RX -> DMA2-Stream3-Channel5 */
  { 2, 4, 2, 2, 3, 2 }, /* SP5  TX -> DMA2-Stream4-Channel2  RX -> DMA2-Stream3-Channel2 */
  { 2, 5, 1, 2, 6, 1 }  /* SP6  TX -> DMA2-Stream5-Channel1  RX -> DMA2-Stream6-Channel1 */
};

/**
 * @brief port SPI 串口信息结构体
 *
 */
typedef struct PORT_SPI_INFO_T
{
  port_os_event_t      event_group;  /* port SPI 事件标志组句柄 */
  port_spi_io_mode_e   io_mode;      /* port SPI 模式 */
  port_spi_work_mode_e rx_work_mode; /* port SPI 接收端工作模式 */
  port_spi_work_mode_e tx_work_mode; /* port SPI 发送端工作模式 */
} port_spi_info_t;

/**
 * @brief (静态局部变量) port SPI 串口信息结构体指针数组
 *
 */
static port_spi_info_t* s_apt_port_spi_info[SPI_COUNT] = { 0 };

extern DMA_HandleTypeDef* pt_port_dma_get_handle(const uint8_t dma_num, const uint8_t dma_stream_num);

static SPI_TypeDef* s_pt_port_spi_get_instance(const uint8_t spi_port)
{
  switch (spi_port)
  {
    case 1 :
      return SPI1;
    case 2 :
      return SPI2;
    case 3 :
      return SPI3;
    case 4 :
      return SPI4;
    case 5 :
      return SPI5;
    case 6 :
      return SPI6;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get instance error!\n");
      return NULL;
  }
}

static uint32_t s_ul_port_spi_get_mode(const port_spi_mode_e spi_mode)
{
  switch (spi_mode)
  {
    case PORT_SPI_FULL_DUPLEX_SLAVE :
    case PORT_SPI_HALF_DUPLEX_SLAVE :
      return SPI_MODE_SLAVE;
    case PORT_SPI_FULL_DUPLEX_MASTER :
    case PORT_SPI_HALF_DUPLEX_MASTER :
      return SPI_MODE_MASTER;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get mode error!\n");
      return 0;
  }
}

static uint32_t s_ul_port_spi_get_direction(const port_spi_mode_e spi_mode)
{
  switch (spi_mode)
  {
    case PORT_SPI_FULL_DUPLEX_SLAVE :
    case PORT_SPI_FULL_DUPLEX_MASTER :
      return SPI_DIRECTION_2LINES;
    case PORT_SPI_HALF_DUPLEX_SLAVE :
    case PORT_SPI_HALF_DUPLEX_MASTER :
      return SPI_DIRECTION_1LINE;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get direction error!\n");
      return 0;
  }
}

static uint32_t s_ul_port_spi_get_data_size(const uint8_t spi_data_size)
{
  switch (spi_data_size)
  {
    case 8 :
      return SPI_DATASIZE_8BIT;
    case 16 :
      return SPI_DATASIZE_16BIT;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get data size error!\n");
      return 0;
  }
}

static uint32_t s_ul_port_spi_get_speed_prescaler(const uint16_t spi_speed_prescaler)
{
  switch (spi_speed_prescaler)
  {
    case 2 :
      return SPI_BAUDRATEPRESCALER_2;
    case 4 :
      return SPI_BAUDRATEPRESCALER_4;
    case 8 :
      return SPI_BAUDRATEPRESCALER_8;
    case 16 :
      return SPI_BAUDRATEPRESCALER_16;
    case 32 :
      return SPI_BAUDRATEPRESCALER_32;
    case 64 :
      return SPI_BAUDRATEPRESCALER_64;
    case 128 :
      return SPI_BAUDRATEPRESCALER_128;
    case 256 :
      return SPI_BAUDRATEPRESCALER_256;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get speed prescaler error!\n");
      return 0;
  }
}

static IRQn_Type s_e_port_spi_get_irqn(const uint8_t spi_port)
{
  switch (spi_port)
  {
    case 1 :
      return SPI1_IRQn;
    case 2 :
      return SPI2_IRQn;
    case 3 :
      return SPI3_IRQn;
    case 4 :
      return SPI4_IRQn;
    case 5 :
      return SPI5_IRQn;
    case 6 :
      return SPI6_IRQn;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get irqn error!\n");
      return 0;
  }
}

static void s_v_port_spi_clock_enable(const uint8_t spi_port)
{
  switch (spi_port)
  {
    case 1 :
      __HAL_RCC_SPI1_CLK_ENABLE();
      break;
    case 2 :
      __HAL_RCC_SPI2_CLK_ENABLE();
      break;
    case 3 :
      __HAL_RCC_SPI3_CLK_ENABLE();
      break;
    case 4 :
      __HAL_RCC_SPI4_CLK_ENABLE();
      break;
    case 5 :
      __HAL_RCC_SPI5_CLK_ENABLE();
      break;
    case 6 :
      __HAL_RCC_SPI6_CLK_ENABLE();
      break;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi clock enable error!\n");
      break;
  }
}

static void s_v_port_spi_clock_disable(const uint8_t spi_port)
{
  switch (spi_port)
  {
    case 1 :
      __HAL_RCC_SPI1_CLK_DISABLE();
      break;
    case 2 :
      __HAL_RCC_SPI2_CLK_DISABLE();
      break;
    case 3 :
      __HAL_RCC_SPI3_CLK_DISABLE();
      break;
    case 4 :
      __HAL_RCC_SPI4_CLK_DISABLE();
      break;
    case 5 :
      __HAL_RCC_SPI5_CLK_DISABLE();
      break;
    case 6 :
      __HAL_RCC_SPI6_CLK_DISABLE();
      break;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi clock disable error!\n");
      break;
  }
}

static inline void sl_v_port_spi_clean_flag(const uint8_t spi_port)
{
  SPI_HandleTypeDef* spi_handle = g_apt_port_spi_handle[spi_port - 1];
  if (NULL != spi_handle)
  {
    __HAL_UNLOCK(spi_handle);
    __HAL_SPI_CLEAR_OVRFLAG(spi_handle);
    __HAL_SPI_CLEAR_MODFFLAG(spi_handle);
    __HAL_SPI_CLEAR_CRCERRFLAG(spi_handle);
  }
}

static uint32_t s_ul_port_spi_get_gpio_af(const uint8_t spi_port)
{
  switch (spi_port)
  {
    case 1 :
      return GPIO_AF5_SPI1;
    case 2 :
      return GPIO_AF5_SPI2;
    case 3 :
      return GPIO_AF6_SPI3;
    case 4 :
      return GPIO_AF5_SPI4;
    case 5 :
      return GPIO_AF5_SPI5;
    case 6 :
      return GPIO_AF5_SPI6;
    default :
      g_e_error_code = UNDEFINED_ERROR;
      ERROR_HANDLE("port spi get gpio af error!\n");
      return 0;
  }
}

static inline void sl_v_port_spi_gpio_init(const uint8_t spi_port)
{
  v_port_gpio_af_init(sc_auc_port_spi_gpio_info[spi_port - 1][0], sc_auc_port_spi_gpio_info[spi_port - 1][1], PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_HIGH, s_ul_port_spi_get_gpio_af(spi_port));
  if (PORT_SPI_RX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
    v_port_gpio_af_init(sc_auc_port_spi_gpio_info[spi_port - 1][2], sc_auc_port_spi_gpio_info[spi_port - 1][3], PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_HIGH, s_ul_port_spi_get_gpio_af(spi_port));
  if (PORT_SPI_TX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
    v_port_gpio_af_init(sc_auc_port_spi_gpio_info[spi_port - 1][4], sc_auc_port_spi_gpio_info[spi_port - 1][5], PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_HIGH, s_ul_port_spi_get_gpio_af(spi_port));
}

static inline void sl_v_port_spi_gpio_deinit(const uint8_t spi_port)
{
  v_port_gpio_deinit(sc_auc_port_spi_gpio_info[spi_port - 1][0], sc_auc_port_spi_gpio_info[spi_port - 1][1], PORT_GPIO_AF_PP);
  if (PORT_SPI_RX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
    v_port_gpio_deinit(sc_auc_port_spi_gpio_info[spi_port - 1][2], sc_auc_port_spi_gpio_info[spi_port - 1][3], PORT_GPIO_AF_PP);
  if (PORT_SPI_TX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
    v_port_gpio_deinit(sc_auc_port_spi_gpio_info[spi_port - 1][4], sc_auc_port_spi_gpio_info[spi_port - 1][5], PORT_GPIO_AF_PP);
}

static error_code_e s_e_port_spi_dma_init(const uint8_t spi_port)
{
  if (PORT_SPI_TX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
  {
    if (PORT_SPI_DMA == s_apt_port_spi_info[spi_port - 1]->tx_work_mode)
    {
      g_e_error_code = e_port_dma_init(sc_auc_port_spi_dma_info[spi_port - 1][0], sc_auc_port_spi_dma_info[spi_port - 1][1], sc_auc_port_spi_dma_info[spi_port - 1][2], PORT_DMA_MEMORY_TO_PERIPH, PORT_DMA_NORMAL, PORT_DMA_MEDIUM);
      __HAL_LINKDMA(g_apt_port_spi_handle[spi_port - 1], hdmatx, *pt_port_dma_get_handle(sc_auc_port_spi_dma_info[spi_port - 1][0], sc_auc_port_spi_dma_info[spi_port - 1][1]));
    }
  }

  if (PORT_SPI_RX == s_apt_port_spi_info[spi_port - 1]->io_mode || PORT_SPI_RX_TX == s_apt_port_spi_info[spi_port - 1]->io_mode)
  {
    if (PORT_SPI_DMA == s_apt_port_spi_info[spi_port - 1]->rx_work_mode)
    {
      g_e_error_code = e_port_dma_init(sc_auc_port_spi_dma_info[spi_port - 1][3], sc_auc_port_spi_dma_info[spi_port - 1][4], sc_auc_port_spi_dma_info[spi_port - 1][5], PORT_DMA_PERIPH_TO_MEMORY, PORT_DMA_NORMAL, PORT_DMA_MEDIUM);
      __HAL_LINKDMA(g_apt_port_spi_handle[spi_port - 1], hdmarx, *pt_port_dma_get_handle(sc_auc_port_spi_dma_info[spi_port - 1][3], sc_auc_port_spi_dma_info[spi_port - 1][4]));
    }
  }

  return g_e_error_code;
}

static error_code_e s_e_port_spi_dma_deinit(const uint8_t spi_port)
{
  g_e_error_code = e_port_dma_deinit(sc_auc_port_spi_dma_info[spi_port - 1][0], sc_auc_port_spi_dma_info[spi_port - 1][1]);
  g_e_error_code = e_port_dma_deinit(sc_auc_port_spi_dma_info[spi_port - 1][3], sc_auc_port_spi_dma_info[spi_port - 1][4]);

  return g_e_error_code;
}

static error_code_e s_e_port_spi_base_init(const uint8_t spi_port, const port_spi_mode_e spi_mode, const uint8_t spi_data_size, const uint16_t spi_speed_prescaler, const port_spi_clk_polarity_e spi_clk_polarity, const uint8_t spi_clk_phase, const port_spi_first_bit_e spi_first_bit, const port_spi_io_mode_e spi_io_mode)
{
  SPI_HandleTypeDef* spi_handle = (SPI_HandleTypeDef*)Malloc(sizeof(SPI_HandleTypeDef));
  if (NULL == spi_handle)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port spi handle Malloc failed!\n");
    return g_e_error_code;
  }

  IRQn_Type spi_irqn = s_e_port_spi_get_irqn(spi_port);
  memset(spi_handle, 0, sizeof(SPI_HandleTypeDef));

  g_apt_port_spi_handle[spi_port - 1] = spi_handle;

  s_v_port_spi_clock_enable(spi_port);

  spi_handle->Instance               = s_pt_port_spi_get_instance(spi_port);
  spi_handle->Init.Mode              = s_ul_port_spi_get_mode(spi_mode);
  spi_handle->Init.Direction         = s_ul_port_spi_get_direction(spi_mode);
  spi_handle->Init.DataSize          = s_ul_port_spi_get_data_size(spi_data_size);
  spi_handle->Init.CLKPolarity       = (uint32_t)spi_clk_polarity;
  spi_handle->Init.CLKPhase          = (uint32_t)spi_clk_phase - 1;
  spi_handle->Init.NSS               = SPI_NSS_SOFT;
  spi_handle->Init.BaudRatePrescaler = s_ul_port_spi_get_speed_prescaler(spi_speed_prescaler);
  spi_handle->Init.FirstBit          = (uint32_t)spi_first_bit;
  spi_handle->Init.TIMode            = SPI_TIMODE_DISABLE;
  spi_handle->Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  spi_handle->Init.CRCPolynomial     = 7;

  HAL_NVIC_DisableIRQ(spi_irqn);
  HAL_NVIC_ClearPendingIRQ(spi_irqn);

  sl_v_port_spi_gpio_deinit(spi_port);

  if (HAL_OK != HAL_SPI_DeInit(spi_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port spi base deinit failed!\n");
    goto spi_base_err;
  }

  sl_v_port_spi_gpio_init(spi_port);

  if (HAL_OK != HAL_SPI_Init(spi_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port spi base init failed!\n");
    goto spi_base_err;
  }

  if (PORT_SPI_RX == spi_io_mode || PORT_SPI_RX_TX == spi_io_mode)
  {
    HAL_NVIC_SetPriority(spi_irqn, SPI_NVIC_DEF_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(spi_irqn);
  }

  return SUCESS;
spi_base_err:
  memset(spi_handle, 0, sizeof(SPI_HandleTypeDef));
  Free(spi_handle);
  g_apt_port_spi_handle[spi_port - 1] = NULL;
  return g_e_error_code;
}

static error_code_e s_e_port_spi_base_deinit(const uint8_t spi_port)
{
  SPI_HandleTypeDef* spi_handle = g_apt_port_spi_handle[spi_port - 1];
  if (NULL == spi_handle)
    return SUCESS;

  IRQn_Type spi_irqn = s_e_port_spi_get_irqn(spi_port);

  HAL_NVIC_DisableIRQ(spi_irqn);
  HAL_NVIC_ClearPendingIRQ(spi_irqn);

  sl_v_port_spi_gpio_deinit(spi_port);

  if (HAL_OK != HAL_SPI_DeInit(spi_handle))
  {
    g_e_error_code = DEINIT_ERROR;
    ERROR_HANDLE("port spi base deinit failed!\n");
  }

  s_v_port_spi_clock_disable(spi_port);

  memset(spi_handle, 0, sizeof(SPI_HandleTypeDef));
  Free(spi_handle);
  g_apt_port_spi_handle[spi_port - 1] = NULL;
  return SUCESS;
}

static void s_v_port_spi_tx_cplt_callback(const uint8_t spi_port)
{
  port_spi_info_t* spi_info = s_apt_port_spi_info[spi_port - 1];

  if (NULL != spi_info)
    ul_port_os_event_set(spi_info->event_group, SPI_SEND_CPLT_BIT);
}

static void s_v_port_spi_rx_cplt_callback(const uint8_t spi_port)
{
  port_spi_info_t* spi_info = s_apt_port_spi_info[spi_port - 1];

  if (NULL != spi_info)
    ul_port_os_event_set(spi_info->event_group, SPI_RECE_CPLT_BIT);
}

void v_port_spi_clean(const uint8_t spi_port)
{
  sl_v_port_spi_clean_flag(spi_port);
}

error_code_e e_port_spi_init(const uint8_t spi_port, const port_spi_mode_e spi_mode, const uint8_t spi_data_size, const uint16_t spi_speed_prescaler, const port_spi_clk_polarity_e spi_clk_polarity, const uint8_t spi_clk_phase, const port_spi_first_bit_e spi_first_bit, const port_spi_io_mode_e spi_io_mode, const port_spi_work_mode_e spi_rx_work_mode, const port_spi_work_mode_e spi_tx_work_mode)
{
  port_spi_info_t* spi_info = (port_spi_info_t*)Malloc(sizeof(port_spi_info_t));
  if (NULL == spi_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port spi info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(spi_info, 0, sizeof(port_spi_info_t));

  s_apt_port_spi_info[spi_port - 1] = spi_info;

  spi_info->event_group             = pt_port_os_event_create();
  if (NULL == spi_info->event_group)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port spi event create failed!\n");
    goto spi_init_err;
  }

  spi_info->io_mode      = spi_io_mode;
  spi_info->rx_work_mode = spi_rx_work_mode;
  spi_info->tx_work_mode = spi_tx_work_mode;

  if (SUCESS != s_e_port_spi_base_init(spi_port, spi_mode, spi_data_size, spi_speed_prescaler, spi_clk_polarity, spi_clk_phase, spi_first_bit, spi_io_mode))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port spi base init failed!\n");
    goto spi_init_err;
  }

  if (PORT_SPI_DMA == spi_rx_work_mode || PORT_SPI_DMA == spi_tx_work_mode)
  {
    if (SUCESS != s_e_port_spi_dma_init(spi_port))
    {
      g_e_error_code = INIT_ERROR;
      ERROR_HANDLE("port spi dma init failed!\n");
      goto spi_init_err;
    }
  }

  sl_v_port_spi_clean_flag(spi_port);

  return SUCESS;
spi_init_err:
  if (NULL != spi_info->event_group)
    v_port_os_event_delete(spi_info->event_group);
  memset(spi_info, 0, sizeof(port_spi_info_t));
  Free(spi_info);
  s_apt_port_spi_info[spi_port - 1] = NULL;
  return g_e_error_code;
}

error_code_e e_port_spi_send(const uint8_t spi_port, void* send_data, uint16_t send_length)
{
  SPI_HandleTypeDef* spi_handle = g_apt_port_spi_handle[spi_port - 1];
  port_spi_info_t*   spi_info   = s_apt_port_spi_info[spi_port - 1];

  if (NULL == spi_handle || NULL == spi_info)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port spi handle or info is not init!\n");
    return g_e_error_code;
  }

  if (PORT_SPI_TX == spi_info->io_mode || PORT_SPI_RX_TX == spi_info->io_mode)
  {
    if (PORT_SPI_IT == spi_info->tx_work_mode)
    {
      if (HAL_OK != HAL_SPI_Transmit_IT(spi_handle, send_data, send_length))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port spi it send failed!\n");
      }
    }
    else if (PORT_SPI_DMA == spi_info->tx_work_mode)
    {
      if (HAL_OK != HAL_SPI_Transmit_DMA(spi_handle, send_data, send_length))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port spi dma send failed!\n");
      }
    }
  }

  return SUCESS;
}

error_code_e e_port_spi_receive(const uint8_t spi_port, void* rece_data, uint16_t rece_length)
{
  SPI_HandleTypeDef* spi_handle = g_apt_port_spi_handle[spi_port - 1];
  port_spi_info_t*   spi_info   = s_apt_port_spi_info[spi_port - 1];

  if (NULL == spi_handle || NULL == spi_info)
  {
    g_e_error_code = UNDEFINED_ERROR;
    ERROR_HANDLE("port spi handle or info is not init!\n");
  }

  if (PORT_SPI_RX == spi_info->io_mode || PORT_SPI_RX_TX == spi_info->io_mode)
  {
    if (PORT_SPI_IT == spi_info->rx_work_mode)
    {
      if (HAL_OK != HAL_SPI_Receive_IT(spi_handle, rece_data, rece_length))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port spi it receive failed!\n");
      }
    }
    else if (PORT_SPI_DMA == spi_info->rx_work_mode)
    {
      if (HAL_OK != HAL_SPI_Receive_DMA(spi_handle, rece_data, rece_length))
      {
        g_e_error_code = TRANSFER_ERROR;
        ERROR_HANDLE("port spi dma receive failed!\n");
      }
    }
  }

  return SUCESS;
}

bool b_port_spi_wait_receive_finish(const uint8_t spi_port, uint32_t waiting_time)
{
  port_spi_info_t* spi_info = s_apt_port_spi_info[spi_port - 1];

  if (NULL == spi_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port spi info is not init!\n");
    return false;
  }

  uint32_t bit = ul_port_os_event_wait(spi_info->event_group, SPI_RECE_CPLT_BIT, waiting_time, false);
  if (0 != (bit & SPI_RECE_CPLT_BIT))
  {
    ul_port_os_event_clear(spi_info->event_group, SPI_RECE_CPLT_BIT);
    return true;
  }

  return false;
}

bool b_port_spi_wait_send_finish(const uint8_t spi_port, uint32_t waiting_time)
{
  port_spi_info_t* spi_info = s_apt_port_spi_info[spi_port - 1];

  if (NULL == spi_info)
  {
    g_e_error_code = NULL_POINTER_ERROR;
    ERROR_HANDLE("port spi info is not init!\n");
    return false;
  }

  uint32_t bit = ul_port_os_event_wait(spi_info->event_group, SPI_SEND_CPLT_BIT, waiting_time, false);
  if (0 != (bit & SPI_SEND_CPLT_BIT))
  {
    ul_port_os_event_clear(spi_info->event_group, SPI_SEND_CPLT_BIT);
    return true;
  }

  return false;
}

error_code_e e_port_spi_deinit(const uint8_t spi_port)
{
  SPI_HandleTypeDef* spi_handle = g_apt_port_spi_handle[spi_port - 1];
  port_spi_info_t*   spi_info   = s_apt_port_spi_info[spi_port - 1];

  if (NULL == spi_handle || NULL == spi_info)
    return g_e_error_code;

  if (NULL != spi_info->event_group)
    v_port_os_event_delete(spi_info->event_group);

  if (PORT_SPI_DMA == spi_info->rx_work_mode || PORT_SPI_DMA == spi_info->tx_work_mode)
  {
    HAL_SPI_DMAStop(spi_handle);
    g_e_error_code = s_e_port_spi_dma_deinit(spi_port);
  }

  g_e_error_code = s_e_port_spi_base_deinit(spi_port);

  memset(spi_info, 0, sizeof(port_spi_info_t));
  Free(spi_info);
  s_apt_port_spi_info[spi_port - 1] = NULL;

  return g_e_error_code;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
  if (SPI1 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(1);
  else if (SPI2 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(2);
  else if (SPI3 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(3);
  else if (SPI4 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(4);
  else if (SPI5 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(5);
  else if (SPI6 == hspi->Instance)
    s_v_port_spi_tx_cplt_callback(6);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi)
{
  if (SPI1 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(1);
  else if (SPI2 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(2);
  else if (SPI3 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(3);
  else if (SPI4 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(4);
  else if (SPI5 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(5);
  else if (SPI6 == hspi->Instance)
    s_v_port_spi_rx_cplt_callback(6);
}