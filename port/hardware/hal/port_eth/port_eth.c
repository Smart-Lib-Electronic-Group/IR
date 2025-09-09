/**
 * @file      port_eth.c
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL ETH port Driver (网络驱动接口)
 * @version   version 1.0.0
 * @date      2025-01-12
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_eth.h"
#include "port_gpio.h"
#include "port_include.h"

/* port ETH 中断优先级 */
#define ETH_NVIC_IRQ_PRIO   7
/* port ETH 复位引脚 定义 */
#define PORT_ETH_RESET_PORT 'H'
#define PORT_ETH_RESET_PIN  8
/* port ETH 引脚 定义 */
#define PORT_ETH_CLK_PORT   'A'
#define PORT_ETH_CLK_PIN    1
#define PORT_ETH_MDIO_PORT  'A'
#define PORT_ETH_MDIO_PIN   2
#define PORT_ETH_CRS_PORT   'A'
#define PORT_ETH_CRS_PIN    7
#define PORT_ETH_MDC_PORT   'C'
#define PORT_ETH_MDC_PIN    1
#define PORT_ETH_RXD0_PORT  'C'
#define PORT_ETH_RXD0_PIN   4
#define PORT_ETH_RXD1_PORT  'C'
#define PORT_ETH_RXD1_PIN   5
#define PORT_ETH_TX_EN_PORT 'B'
#define PORT_ETH_TX_EN_PIN  11
#define PORT_ETH_TXD0_PORT  'G'
#define PORT_ETH_TXD0_PIN   13
#define PORT_ETH_TXD1_PORT  'G'
#define PORT_ETH_TXD1_PIN   14

#include "port_os.h"

/**
 * @brief port ETH 网络 复位
 *
 */
void v_port_eth_reset()
{
  v_port_gpio_write(PORT_ETH_RESET_PORT, PORT_ETH_RESET_PIN, 0);
  v_port_os_delay_ms(100);
  v_port_gpio_write(PORT_ETH_RESET_PORT, PORT_ETH_RESET_PIN, 1);
  v_port_os_delay_ms(100);
}

/**
 * @brief port ETH HAL库 底层初始化
 *
 * @param heth HAL库 以太网句柄
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
  if (heth->Instance == ETH)
  {
    /* 时钟使能 */
    __HAL_RCC_ETH_CLK_ENABLE();

    /* 网卡引脚初始化 */
    v_port_gpio_af_init(PORT_ETH_CLK_PORT, PORT_ETH_CLK_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_MDIO_PORT, PORT_ETH_MDIO_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_CRS_PORT, PORT_ETH_CRS_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_MDC_PORT, PORT_ETH_MDC_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_RXD0_PORT, PORT_ETH_RXD0_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_RXD1_PORT, PORT_ETH_RXD1_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_TX_EN_PORT, PORT_ETH_TX_EN_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_TXD0_PORT, PORT_ETH_TXD0_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);
    v_port_gpio_af_init(PORT_ETH_TXD1_PORT, PORT_ETH_TXD1_PIN, PORT_GPIO_AF_PP, PORT_GPIO_PULL_NONE, PORT_GPIO_SPEED_VERY_HIGH, GPIO_AF11_ETH);

    /* 网卡复位引脚初始化 */
    v_port_gpio_init(PORT_ETH_RESET_PORT, PORT_ETH_RESET_PIN, PORT_GPIO_OUT_PP, PORT_GPIO_EVENT_BOTH, PORT_GPIO_PULL_UP, PORT_GPIO_SPEED_VERY_HIGH, NULL);

    /* 中断使能 */
    HAL_NVIC_SetPriority(ETH_IRQn, ETH_NVIC_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    /* 网卡复位 */
    v_port_eth_reset();
  }
}

/**
 * @brief port ETH HAL库 底层解除初始化
 *
 * @param heth HAL库 以太网句柄
 */
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
  if (heth->Instance == ETH)
  {
    /* 中断失能 */
    HAL_NVIC_DisableIRQ(ETH_IRQn);

    /* 网卡复位引脚解除初始化 */
    v_port_gpio_deinit(PORT_ETH_RESET_PORT, PORT_ETH_RESET_PIN, PORT_GPIO_OUT_PP);

    /* 网卡引脚解除初始化 */
    v_port_gpio_deinit(PORT_ETH_CLK_PORT, PORT_ETH_CLK_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_MDIO_PORT, PORT_ETH_MDIO_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_CRS_PORT, PORT_ETH_CRS_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_MDC_PORT, PORT_ETH_MDC_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_RXD0_PORT, PORT_ETH_RXD0_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_RXD1_PORT, PORT_ETH_RXD1_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_TX_EN_PORT, PORT_ETH_TX_EN_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_TXD0_PORT, PORT_ETH_TXD0_PIN, PORT_GPIO_AF_PP);
    v_port_gpio_deinit(PORT_ETH_TXD1_PORT, PORT_ETH_TXD1_PIN, PORT_GPIO_AF_PP);

    /* 时钟失能 */
    __HAL_RCC_ETH_CLK_DISABLE();
  }
}
