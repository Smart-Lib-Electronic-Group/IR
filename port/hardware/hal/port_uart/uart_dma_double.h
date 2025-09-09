/**
 * @file      uart_dma_double.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 UART DMA双缓冲实现 (仿HAL)
 * @version   version 1.0.0
 * @date      2025-01-20
 * 
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 * 
 */
#ifndef __UART_DMA_DOUBLE_H__
#define __UART_DMA_DOUBLE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "port_include.h"

  extern HAL_StatusTypeDef HAL_UART_Receive_DMA_Double(UART_HandleTypeDef* huart, uint8_t* pData0, uint8_t* pData1, uint16_t Size);
  extern void              HAL_UART_DMA_M0_RxCpltCallback(UART_HandleTypeDef* huart);
  extern void              HAL_UART_DMA_M1_RxCpltCallback(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UART_DMA_DOUBLE_H__ */
