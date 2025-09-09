/**
 * @file      port_uart.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL UART port Driver (UART——驱动接口)
 * @version   version 2.0.0
 * @date      2025-03-16
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_UART_H__
#define __PORT_UART_H__

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "error_handle.h"
#include "port_os.h"

/// @brief UART 端口总数
#define UART_COUNT 8

  /// @brief port UART 工作模式
  typedef enum PORT_UART_WORK_MODE_E
  {
    PORT_UART_IT,         /* port UART 中断模式 */
    PORT_UART_DMA,        /* port UART DMA模式 */
    PORT_UART_DMA_DOUBLE, /* port UART DMA双缓冲模式 */
  } port_uart_work_mode_e;

  /// @brief port UART 校验模式
  typedef enum PORT_UART_PARITY_E
  {
    PORT_UART_NONE, /* port UART 无校验模式 */
    PORT_UART_EVEN, /* port UART 偶校验模式 */
    PORT_UART_ODD,  /* port UART 奇校验模式 */
  } port_uart_parity_e;

  /// @brief port UART 模式
  typedef enum PORT_UART_IO_MODE_E
  {
    PORT_UART_RX,    /* port UART 接收模式 */
    PORT_UART_TX,    /* port UART 发送模式 */
    PORT_UART_RX_TX, /* port UART 收发模式 */
  } port_uart_io_mode_e;

  /// @brief port UART 回调函数
  typedef struct PORT_UART_CALLBACK_T
  {
    void (*function)(void*); /* port UART 回调函数 */
    void* arg;               /* port UART 回调函数参数 */
  } port_uart_callback_t;

  extern void         v_port_uart_clean(const uint8_t uart_port);
  extern error_code_e e_port_uart_reinit(const uint8_t uart_port, const uint32_t uart_baud_rate, const uint8_t uart_word_length, const uint8_t uart_stop_bits, const port_uart_parity_e uart_parity);
  extern error_code_e e_port_uart_init(const uint8_t uart_port, const uint32_t uart_baud_rate, const uint8_t uart_word_length, const uint8_t uart_stop_bits, const port_uart_parity_e uart_parity, const port_uart_io_mode_e uart_io_mode, const port_uart_work_mode_e uart_rx_work_mode, const port_uart_work_mode_e uart_tx_work_mode, port_os_stream_t uart_receive_buf, const uint32_t uart_receive_buf_size, const port_uart_callback_t* uart_rx_cplt, const port_uart_callback_t* uart_tx_cplt);
  extern error_code_e e_port_uart_send(const uint8_t uart_port, const void* data, uint16_t len);
  extern uint64_t     ull_port_uart_total_received(const uint8_t uart_port);
  extern bool         b_port_uart_wait_receive_complete(const uint8_t uart_port, uint32_t waiting_time);
  extern bool         b_port_uart_clean_receive_complete(const uint8_t uart_port);
  extern bool         b_port_uart_wait_send_complete(const uint8_t uart_port, uint32_t waiting_time);
  extern error_code_e e_port_uart_deinit(const uint8_t uart_port);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_UART_H__ */
