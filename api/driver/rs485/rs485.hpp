#ifndef __RS485_HPP__
#define __RS485_HPP__

#include "signal.hpp"
#include "virtual_uart.hpp"
#include "virtual_gpio.hpp"

/// @brief 名称空间 库名
namespace OwO
{
namespace driver
{
class RS485 : public virtual_class::VUart
{
  O_OBJECT
  NO_COPY(RS485)
  NO_MOVE(RS485)

private:
  Class::io* m_de;
  Class::io* m_re;

protected:
  virtual void send_start() override
  {
    if (m_re)
      m_re->high();
    if (m_de)
      m_de->high();
  }

  virtual void send_end() override
  {
    if (m_re)
      m_re->low();
    if (m_de)
      m_de->low();
  }

  virtual void hard_recv_end() override
  {
    signal_recv_finished(this);
  }

public:
  system::Signal<IOStream*> signal_recv_finished;

  explicit RS485(const std::string& name = "RS485", Object* parent = nullptr) : virtual_class::VUart(name, parent)
  {
    m_de = nullptr;
    m_re = nullptr;
  }

  virtual bool open(const uint8_t& port, const uint32_t& baud_rate = 9600, const uint8_t& word_length = 8, const uint8_t& stop_bits = 1, const Uart::Parity& parity = Uart::NONE, const Uart::IO_Mode& mode = Uart::RX_TX, const Uart ::Work_Mode& rx_mode = Uart::DMA, const Uart::Work_Mode& tx_mode = Uart::DMA, const uint32_t& receive_tmp_size = UART_RECEIVE_TMP_SIZE, const uint32_t& istream_size = 256, const uint32_t& ostream_size = 128);

  virtual bool open(const uint8_t& port, Gpio::Port de_port, uint8_t de_pin, const uint32_t& baud_rate = 9600, const uint8_t& word_length = 8, const uint8_t& stop_bits = 1, const Uart::Parity& parity = Uart::NONE, const Uart::IO_Mode& mode = Uart::RX_TX, const Uart ::Work_Mode& rx_mode = Uart::DMA, const Uart::Work_Mode& tx_mode = Uart::DMA, const uint32_t& receive_tmp_size = UART_RECEIVE_TMP_SIZE, const uint32_t& istream_size = 256, const uint32_t& ostream_size = 128);
  
  virtual bool open(const uint8_t& port, Gpio::Port de_port, uint8_t de_pin, Gpio::Port re_port, uint8_t re_pin, const uint32_t& baud_rate = 9600, const uint8_t& word_length = 8, const uint8_t& stop_bits = 1, const Uart::Parity& parity = Uart::NONE, const Uart::IO_Mode& mode = Uart::RX_TX, const Uart ::Work_Mode& rx_mode = Uart::DMA, const Uart::Work_Mode& tx_mode = Uart::DMA, const uint32_t& receive_tmp_size = UART_RECEIVE_TMP_SIZE, const uint32_t& istream_size = 256, const uint32_t& ostream_size = 128);

  virtual bool close() override
  {
    if (m_de)
    {
      delete m_de;
      m_de = nullptr;
    }
    
    if (m_re)
    {
      delete m_re;
      m_re = nullptr;
    }

    return virtual_class::VUart::close();
  }

  virtual ~RS485()
  {
    close();
  }
};

} /* namespace driver */
} /* namespace OwO */

#endif /* __RS485_HPP__ */
