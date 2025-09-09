#include "port_system.h"
#include "port_include.h"

static port_system_work_time_t s_t_port_system_work_time;

static inline void sl_v_port_system_dwt_init()
{
  /* 关闭 TRC */
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  /* 打开 TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* 关闭计数功能 */
  DWT->CTRL        &= ~DWT_CTRL_CYCCNTENA_Msk;
  /* 打开计数功能 */
  DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
  /* 计数清零 */
  DWT->CYCCNT       = (uint32_t)0u;
}

static error_code_e s_e_port_system_clk_init(const uint32_t plln, const uint32_t pllm, const uint32_t pllp, const uint32_t pllq)
{
  HAL_StatusTypeDef  ret          = HAL_OK;
  RCC_ClkInitTypeDef rcc_clk_init = { 0 };
  RCC_OscInitTypeDef rcc_osc_init = { 0 };

  __HAL_RCC_PWR_CLK_ENABLE(); /* 使能PWR时钟 */

  /* 下面这个设置用来设置调压器输出电压级别，以便在器件未以最大频率工作时使性能与功耗实现平衡 */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3); /* 调压器输出电压级别选择：级别3模式 */

  /* 使能HSE，并选择HSE作为PLL时钟源，配置PLL1，开启USB时钟 */
  rcc_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE; /* 时钟源为HSE */
  rcc_osc_init.HSEState       = RCC_HSE_ON;             /* 打开HSE */
  rcc_osc_init.PLL.PLLState   = RCC_PLL_ON;             /* 打开PLL */
  rcc_osc_init.PLL.PLLSource  = RCC_PLLSOURCE_HSE;      /* PLL时钟源选择HSE */
  rcc_osc_init.PLL.PLLN       = plln;
  rcc_osc_init.PLL.PLLM       = pllm;
  rcc_osc_init.PLL.PLLP       = pllp;
  rcc_osc_init.PLL.PLLQ       = pllq;
  ret                         = HAL_RCC_OscConfig(&rcc_osc_init); /* 初始化RCC */
  if (ret != HAL_OK)
  {
    ERROR_HANDLE("port system clk osc config error!\n");
    g_e_error_code = INIT_ERROR;
    return g_e_error_code;
  }

  ret = HAL_PWREx_EnableOverDrive(); /* 开启Over-Driver功能 */
  if (ret != HAL_OK)
  {
    ERROR_HANDLE("port system clk over driver enable error!\n");
    g_e_error_code = INIT_ERROR;
    return g_e_error_code;
  }

  /* 选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2*/
  rcc_clk_init.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);

  rcc_clk_init.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;                             /* 设置系统时钟时钟源为PLL */
  rcc_clk_init.AHBCLKDivider  = RCC_SYSCLK_DIV1;                                     /* AHB分频系数为1 */
  rcc_clk_init.APB1CLKDivider = RCC_HCLK_DIV4;                                       /* APB1分频系数为4 */
  rcc_clk_init.APB2CLKDivider = RCC_HCLK_DIV4;                                       /* APB2分频系数为4 */
  ret                         = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_5); /* 同时设置FLASH延时周期为5WS，也就是6个CPU周期 */
  if (ret != HAL_OK)
  {
    ERROR_HANDLE("port system clk config error!\n");
    g_e_error_code = INIT_ERROR;
    return g_e_error_code;
  }

  return SUCESS;
}

error_code_e e_port_system_init()
{
  HAL_Init();
  /* 设置主频,72Mhz */
  g_e_error_code = s_e_port_system_clk_init(144, 25, 2, 4);
  sl_v_port_system_dwt_init();
  return g_e_error_code;
}

void v_port_system_reset()
{
  NVIC_SystemReset();
}

port_system_work_time_t* p_port_system_get_work_time()
{
  return &s_t_port_system_work_time;
}

void v_port_system_worktime_irq_function()
{
  s_t_port_system_work_time.ticks++;

  if (0 == (s_t_port_system_work_time.ticks % 1000))
  {
    s_t_port_system_work_time.ticks = 0;
    s_t_port_system_work_time.seconds++;

    if (60 == s_t_port_system_work_time.seconds)
    {
      s_t_port_system_work_time.minutes++;
      s_t_port_system_work_time.seconds = 0;
    }

    if (60 == s_t_port_system_work_time.minutes)
    {
      s_t_port_system_work_time.hours++;
      s_t_port_system_work_time.minutes = 0;
    }

    if (24 == s_t_port_system_work_time.hours)
    {
      s_t_port_system_work_time.days++;
      s_t_port_system_work_time.hours = 0;
    }
  }
}

#include "port_os_isr.h"
void v_port_system_delay_us(uint32_t us)
{
  uint32_t tickStart  = DWT->CYCCNT;
  /* 转换为us对应的时钟跳动次数*/
  us                 *= (SystemCoreClock / 1000000);
  /* 延时等待 */
  while ((DWT->CYCCNT - tickStart) < us)
    v_port_os_thread_yield();
}