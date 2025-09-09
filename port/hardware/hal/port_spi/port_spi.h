#ifndef __PORT_SPI_H__
#define __PORT_SPI_H__

#include "error_handle.h"
#include "port_gpio.h"
#include "port_os.h"

/// @brief SPI 端口总数
#define SPI_COUNT         6
/// @brief SPI 接收完成事件标志位
#define SPI_RECE_CPLT_BIT 0x00000001
/// @brief SPI 发送完成事件标志位
#define SPI_SEND_CPLT_BIT 0x00000002

#if __cplusplus
extern "C"
{
#endif

  typedef enum PORT_SPI_WORK_MODE_E
  {
    PORT_SPI_IT,
    PORT_SPI_DMA,
  } port_spi_work_mode_e;

  typedef enum PORT_SPI_MODE_E
  {
    PORT_SPI_FULL_DUPLEX_SLAVE,
    PORT_SPI_HALF_DUPLEX_SLAVE,
    PORT_SPI_FULL_DUPLEX_MASTER,
    PORT_SPI_HALF_DUPLEX_MASTER,
  } port_spi_mode_e;

  typedef enum PORT_SPI_IO_MODE_E
  {
    PORT_SPI_RX,
    PORT_SPI_TX,
    PORT_SPI_RX_TX,
  } port_spi_io_mode_e;

  typedef enum PORT_SPI_FIRST_BIT_E
  {
    PORT_SPI_MSB = 0x00,
    PORT_SPI_LSB = 0x80,
  } port_spi_first_bit_e;

  typedef enum PORT_SPI_CLK_POLARITY_E
  {
    PORT_SPI_CLK_POLARITY_LOW,
    PORT_SPI_CLK_POLARITY_HIGH = 2,
  } port_spi_clk_polarity_e;

  extern void         v_port_spi_clean(const uint8_t spi_port);
  extern error_code_e e_port_spi_init(const uint8_t spi_port, const port_spi_mode_e spi_mode, const uint8_t spi_data_size, const uint16_t spi_speed_prescaler, const port_spi_clk_polarity_e spi_clk_polarity, const uint8_t spi_clk_phase, const port_spi_first_bit_e spi_first_bit, const port_spi_io_mode_e spi_io_mode, const port_spi_work_mode_e spi_rx_work_mode, const port_spi_work_mode_e spi_tx_work_mode);
  extern error_code_e e_port_spi_send(const uint8_t spi_port,void* send_data, uint16_t send_length);
  extern error_code_e e_port_spi_receive(const uint8_t spi_port, void* recv_data, uint16_t recv_length);
  extern bool         b_port_spi_wait_receive_finish(const uint8_t spi_port, uint32_t waiting_time);
  extern bool         b_port_spi_wait_send_finish(const uint8_t spi_port, uint32_t waiting_time);
  extern error_code_e e_port_spi_deinit(const uint8_t spi_port);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_SPI_H__ */
