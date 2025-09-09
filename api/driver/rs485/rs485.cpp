#include "rs485.hpp"

using namespace OwO;
using namespace virtual_class;
using namespace Uart;
using namespace Gpio;
using namespace system;
using namespace driver;

O_METAOBJECT(RS485, VUart)

bool RS485::open(const uint8_t& port, const uint32_t& baud_rate, const uint8_t& word_length, const uint8_t& stop_bits, const Parity& parity, const IO_Mode& mode, const Work_Mode& rx_mode, const Work_Mode& tx_mode, const uint32_t& receive_tmp_size, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  return VUart::open(port, baud_rate, word_length, stop_bits, parity, mode, rx_mode, tx_mode, receive_tmp_size, istream_size, ostream_size);
}

bool RS485::open(const uint8_t& port, Port de_port, uint8_t de_pin, const uint32_t& baud_rate, const uint8_t& word_length, const uint8_t& stop_bits, const Parity& parity, const IO_Mode& mode, const Work_Mode& rx_mode, const Work_Mode& tx_mode, const uint32_t& receive_tmp_size, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (false == VUart::open(port, baud_rate, word_length, stop_bits, parity, mode, rx_mode, tx_mode, receive_tmp_size, istream_size, ostream_size))
    return false;

  m_de = new Class::io(name() + "_de", this);
  if (false == m_de->open(de_port, de_pin))
  {
    close();
    return false;
  }

  m_de->low();
  return true;
}

bool RS485::open(const uint8_t& port, Port de_port, uint8_t de_pin, Port re_port, uint8_t re_pin, const uint32_t& baud_rate, const uint8_t& word_length, const uint8_t& stop_bits, const Parity& parity, const IO_Mode& mode, const Work_Mode& rx_mode, const Work_Mode& tx_mode, const uint32_t& receive_tmp_size, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (false == VUart::open(port, baud_rate, word_length, stop_bits, parity, mode, rx_mode, tx_mode, receive_tmp_size, istream_size, ostream_size))
    return false;

  m_de = new Class::io(name() + "_de", this);
  if (false == m_de->open(de_port, de_pin))
  {
    close();
    return false;
  }

  m_re = new Class::io(name() + "_re", this);
  if (false == m_re->open(re_port, re_pin))
  {
    close();
    return false;
  }

  m_de->low();
  m_re->low();
  return true;
}
