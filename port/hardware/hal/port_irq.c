/**
 * @file      port_irq.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL port Driver IRQ (驱动接口——中断处理)
 * @version   version 2.0.0
 * @date      2025-03-15
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_include.h"
#include "port_os_isr.h"

/* ---------------------------------- port System CLK 中断 ---------------------------------- */

extern void v_port_system_worktime_irq_function();
extern void v_port_os_irq_function();
void        SysTick_Handler(void)
{
  /* HAL库时钟源 */
  HAL_IncTick();

  /* 系统时钟中断 */
  v_port_os_irq_function();

  /* 工作时长记录 */
  v_port_system_worktime_irq_function();
}

/* ---------------------------------- port EXIT 中断 ---------------------------------- */

void EXTI0_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI1_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI9_5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
  port_os_exit_critical_isr(isr_mask);
}

void EXTI15_10_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
  port_os_exit_critical_isr(isr_mask);
}

/* ---------------------------------- port TIM 中断 ---------------------------------- */

#include "port_tim.h"
extern TIM_HandleTypeDef* g_apt_port_timer_handle[TIMER_COUNT];

void TIM1_UP_TIM10_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  if (g_apt_port_timer_handle[0])
    HAL_TIM_IRQHandler(g_apt_port_timer_handle[0]);
  if (g_apt_port_timer_handle[9])
    HAL_TIM_IRQHandler(g_apt_port_timer_handle[9]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[1]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[2]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[3]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[4]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM6_DAC_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[5]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM7_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[6]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM8_UP_TIM13_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  if (g_apt_port_timer_handle[7])
    HAL_TIM_IRQHandler(g_apt_port_timer_handle[7]);
  if (g_apt_port_timer_handle[12])
    HAL_TIM_IRQHandler(g_apt_port_timer_handle[12]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[8]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[10]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM8_BRK_TIM12_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[11]);
  port_os_exit_critical_isr(isr_mask);
}

void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_TIM_IRQHandler(g_apt_port_timer_handle[13]);
  port_os_exit_critical_isr(isr_mask);
}

/* ---------------------------------- port DMA 中断 ---------------------------------- */

extern DMA_HandleTypeDef* g_apt_port_dma_handle[2][8];

void DMA1_Stream0_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][0]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream1_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][1]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][2]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][3]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][4]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][5]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream6_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][6]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA1_Stream7_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[0][7]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream0_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][0]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream1_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][1]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][2]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][3]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][4]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][5]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream6_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][6]);
  port_os_exit_critical_isr(isr_mask);
}

void DMA2_Stream7_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_DMA_IRQHandler(g_apt_port_dma_handle[1][7]);
  port_os_exit_critical_isr(isr_mask);
}

/* ---------------------------------- port Uart 中断 ---------------------------------- */

#include "port_uart.h"
extern UART_HandleTypeDef* g_apt_port_uart_handle[UART_COUNT];
extern void                v_port_uart_irq_function(uint8_t uart_port);

void USART1_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(1);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[0]);
  port_os_exit_critical_isr(isr_mask);
}

void USART2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(2);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[1]);
  port_os_exit_critical_isr(isr_mask);
}

void USART3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(3);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[2]);
  port_os_exit_critical_isr(isr_mask);
}

void UART4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(4);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[3]);
  port_os_exit_critical_isr(isr_mask);
}

void UART5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(5);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[4]);
  port_os_exit_critical_isr(isr_mask);
}

void USART6_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(6);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[5]);
  port_os_exit_critical_isr(isr_mask);
}

void UART7_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(7);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[6]);
  port_os_exit_critical_isr(isr_mask);
}

void UART8_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  v_port_uart_irq_function(8);
  HAL_UART_IRQHandler(g_apt_port_uart_handle[7]);
  port_os_exit_critical_isr(isr_mask);
}

/* ---------------------------------- port SPI 中断 ---------------------------------- */

#include "port_spi.h"
extern SPI_HandleTypeDef* g_apt_port_spi_handle[SPI_COUNT];

void SPI1_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[0]);
  port_os_exit_critical_isr(isr_mask);
}

void SPI2_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[1]);
  port_os_exit_critical_isr(isr_mask);
}

void SPI3_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[2]);
  port_os_exit_critical_isr(isr_mask);
}

void SPI4_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[3]);
  port_os_exit_critical_isr(isr_mask);
}

void SPI5_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[4]);
  port_os_exit_critical_isr(isr_mask);
}

void SPI6_IRQHandler(void)
{
  uint32_t isr_mask;
  isr_mask = port_os_enter_critical_isr();
  HAL_SPI_IRQHandler(g_apt_port_spi_handle[5]);
  port_os_exit_critical_isr(isr_mask);
}
