/**
 * @file      port_dma.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL DMA port Driver (DMA驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_DMA_H__
#define __PORT_DMA_H__
#if __cplusplus
extern "C"
{
#endif

#include "error_handle.h"

  /// @brief port DMA 数据流方向
  typedef enum PORT_DMA_DIRECTION_E
  {
    PORT_DMA_PERIPH_TO_MEMORY, /* port DMA 数据流方向 从外设到内存 */
    PORT_DMA_MEMORY_TO_PERIPH, /* port DMA 数据流方向 从内存到外设 */
    PORT_DMA_MEMORY_TO_MEMORY, /* port DMA 数据流方向 从内存到外设 */
  } port_dma_direction_e;

  /// @brief port DMA 工作模式
  typedef enum PORT_DMA_MODE_E
  {
    PORT_DMA_NORMAL,   /* port DMA 普通模式 */
    PORT_DMA_CIRCULAR, /* port DMA 循环模式 */
  } port_dma_mode_e;

  /// @brief port DMA 工作优先级
  typedef enum PORT_DMA_PRIORITY_E
  {
    PORT_DMA_LOW,       /* port DMA 低优先级 */
    PORT_DMA_MEDIUM,    /* port DMA 中优先级 */
    PORT_DMA_HIGH,      /* port DMA 高优先级 */
    PORT_DMA_VERY_HIGH, /* port DMA 极高优先级 */
  } port_dma_priority_e;

  extern void         v_port_dma_nvic_enable(const uint8_t dma_num, const uint8_t dma_stream_num);
  extern void         v_port_dma_nvic_disable(const uint8_t dma_num, const uint8_t dma_stream_num);
  extern void         v_port_dma_nvic_set_priority(const uint8_t dma_num, const uint8_t dma_stream_num, uint8_t dma_nvic_priority);
  extern error_code_e e_port_dma_init(const uint8_t dma_num, const uint8_t dma_stream_num, const uint8_t dma_channel, const port_dma_direction_e dma_direction, const port_dma_mode_e dma_mode, const port_dma_priority_e dma_priority);
  extern error_code_e e_port_dma_deinit(const uint8_t dma_num, const uint8_t dma_stream_num);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_DMA_H__ */
