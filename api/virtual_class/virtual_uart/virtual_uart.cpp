#include "virtual_uart.hpp"

using namespace OwO::virtual_class;
using namespace OwO::system;
using namespace OwO::Uart;

O_METAOBJECT(VUart, IOStream)

void VUart::send_complete_callback(void* arg)
{
  static_cast<VUart*>(arg)->hard_send_end();
}

void VUart::receive_complete_callback(void* arg)
{
  static_cast<VUart*>(arg)->hard_recv_end();
}

bool VUart::hard_recv_wait_bit(uint32_t timeout)
{
  if (is_open())
    return b_port_uart_wait_receive_complete(m_uart_port, timeout);
  else
    return false;
}

bool VUart::hard_recv_clean_bit()
{
  if (is_open())
    return b_port_uart_clean_receive_complete(m_uart_port);
  else
    return false;
}

uint32_t VUart::hard_recv(void* buf, uint32_t length, uint32_t timeout)
{
  return 0;
}

uint32_t VUart::hard_send(const void* buf, uint32_t length)
{
  if (is_open())
  {
    if (SUCESS == e_port_uart_send(m_uart_port, buf, length))
      return length;
  }
  return 0;
}

bool VUart::reload()
{
  if (is_open())
  {
    if (SUCESS == e_port_uart_reinit(m_uart_port, m_baud_rate, m_word_length, m_stop_bits, static_cast<port_uart_parity_e>(m_parity)))
      return true;
  }
  return false;
}

bool VUart::open(const uint8_t& port, const uint32_t& baud_rate, const uint8_t& word_length, const uint8_t& stop_bits, const Parity& parity, const IO_Mode& mode, const Work_Mode& rx_mode, const Work_Mode& tx_mode, const uint32_t& receive_tmp_size, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (false == IOStream::open(istream_size, ostream_size))
    return false;

  m_uart_port                = port;
  m_baud_rate                = baud_rate;
  m_word_length              = word_length;
  m_stop_bits                = stop_bits;
  m_parity                   = parity;
  m_mode                     = mode;
  m_rx_mode                  = rx_mode;
  m_tx_mode                  = tx_mode;
  m_rx_tmp_size              = receive_tmp_size;

  port_uart_callback_t rx_cb = { 0 };
  rx_cb.function             = receive_complete_callback;
  rx_cb.arg                  = this;

  port_uart_callback_t tx_cb = { 0 };
  tx_cb.function             = send_complete_callback;
  tx_cb.arg                  = this;

  if (SUCESS != e_port_uart_init(m_uart_port, m_baud_rate, m_word_length, m_stop_bits, static_cast<port_uart_parity_e>(m_parity), static_cast<port_uart_io_mode_e>(m_mode), static_cast<port_uart_work_mode_e>(m_rx_mode), static_cast<port_uart_work_mode_e>(m_tx_mode), istream(), m_rx_tmp_size, &rx_cb, &tx_cb))
  {
    close();
    return false;
  }

  return true;
}
