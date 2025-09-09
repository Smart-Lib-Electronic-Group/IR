#ifndef __VIRTUAL_SPI_HPP__
#define __VIRTUAL_SPI_HPP__

#include "virtual_gpio.hpp"
#include "object.hpp"
#include "port_spi.h"

/// @brief 名称空间 库名
namespace OwO
{
namespace Spi
{
enum WorkMode
{
  IT,  /* 中断模式 */
  DMA, /* DMA模式 */
};

enum Mode
{
  FULL_DUPLEX_SLAVE,
  HALF_DUPLEX_SLAVE,
  FULL_DUPLEX_MASTER,
  HALF_DUPLEX_MASTER,
};

enum IO_Mode
{
  RX,
  TX,
  RX_TX,
};

enum First_Bit
{
  MSB,
  LSB,
};

enum Clock_Polarity
{
  LOW,
  HIGH,
};
} /* namespace Spi */

/// @brief 名称空间 纯虚类
namespace virtual_class
{
class VSpi : public OwO::system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VSpi)
  NO_MOVE(VSpi)
private:
  uint8_t             m_port;
  Class::io*          m_cs;
  Spi::WorkMode       m_rx_mode;
  Spi::WorkMode       m_tx_mode;
  Spi::Mode           m_mode;
  uint8_t             m_data_size;
  uint16_t            m_speed_prescaler;
  Spi::Clock_Polarity m_clock_polarity;
  uint8_t             m_clock_phase;
  Spi::First_Bit      m_first_bit;
  Spi::IO_Mode        m_io_mode;

public:
  explicit VSpi(const std::string& name = "VSPI", Object* parent = nullptr) : OwO::system::Object(name, parent)
  {
    m_port            = 0;
    m_cs              = nullptr;
    m_rx_mode         = Spi::IT;
    m_tx_mode         = Spi::IT;
    m_mode            = Spi::FULL_DUPLEX_MASTER;
    m_data_size       = 8;
    m_speed_prescaler = 4;
    m_clock_polarity  = Spi::LOW;
    m_clock_phase     = 0;
    m_first_bit       = Spi::MSB;
    m_io_mode         = Spi::RX_TX;
  }

  virtual bool open(uint8_t port, Gpio::Port cs_port, uint8_t cs_pin, Spi::WorkMode rx_mode = Spi::IT, Spi::WorkMode tx_mode = Spi::IT, uint16_t speed_prescaler = 4, uint8_t clock_phase = 1, Spi::Clock_Polarity clock_polarity = Spi::LOW, Spi::Mode mode = Spi::FULL_DUPLEX_MASTER, uint8_t data_size = 8, Spi::First_Bit first_bit = Spi::MSB, Spi::IO_Mode io_mode = Spi::RX_TX)
  {
    if (true == m_is_open)
      return false;

    m_port = port;
    m_cs   = new Class::io(name() + "_cs", this);

    if (false == m_cs->open(cs_port, cs_pin))
    {
      delete m_cs;
      return false;
    }

    m_rx_mode         = rx_mode;
    m_tx_mode         = tx_mode;
    m_mode            = mode;
    m_data_size       = data_size;
    m_speed_prescaler = speed_prescaler;
    m_clock_polarity  = clock_polarity;
    m_clock_phase     = clock_phase;
    m_first_bit       = first_bit;
    m_io_mode         = io_mode;

    if (SUCESS != e_port_spi_init(m_port, static_cast<port_spi_mode_e>(m_mode), m_data_size, m_speed_prescaler, static_cast<port_spi_clk_polarity_e>(m_clock_polarity), m_clock_phase, static_cast<port_spi_first_bit_e>(m_first_bit), static_cast<port_spi_io_mode_e>(m_io_mode), static_cast<port_spi_work_mode_e>(m_rx_mode), static_cast<port_spi_work_mode_e>(m_tx_mode)))
    {
      delete m_cs;
      return false;
    }

    m_is_open = true;
    return true;
  }

  virtual bool close()
  {
    if (false == m_is_open)
      return true;

    bool ret  = true;
    ret      &= e_port_spi_deinit(m_port) == SUCESS;
    ret      &= m_cs->close();
    delete m_cs;
    m_is_open = false;
    return ret;
  }

  void spi_start()
  {
    m_cs->low();
  }

  void spi_stop()
  {
    m_cs->high();
  }

  uint32_t send(const void* data, uint16_t len)
  {
    return (e_port_spi_send(m_port, const_cast<void*>(data), len) == SUCESS) ? len : 0;
  }

  bool wait_send_complete(uint32_t timeout = WAIT_FOREVER)
  {
    return b_port_spi_wait_send_finish(m_port, timeout);
  }

  uint32_t receive(void* data, uint16_t len)
  {
    return (e_port_spi_receive(m_port, data, len) == SUCESS) ? len : 0;
  }

  bool wait_receive_complete(uint32_t timeout = WAIT_FOREVER)
  {
    return b_port_spi_wait_receive_finish(m_port, timeout);
  }

  virtual ~VSpi()
  {
    close();
  }
};
} /* namespace virtual_class */
} /* namespace OwO */

#endif /* __VIRTUAL_SPI_HPP__ */
