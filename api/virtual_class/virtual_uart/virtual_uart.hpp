#ifndef __VIRTUAL_UART_HPP__
#define __VIRTUAL_UART_HPP__

#include "port_uart.h"
#include "iostream.hpp"

#define UART_RECEIVE_TMP_SIZE 128

/// @brief 名称空间 库名
namespace OwO
{
namespace Uart
{
/// @brief 枚举 Uart 收发工作模式
enum Work_Mode
{
  IT,         /* 中断模式 */
  DMA,        /* DMA模式 */
  DMA_DOUBLE, /* DMA模式双缓存 */
};

/// @brief 枚举 Uart 校验方式
enum Parity
{
  NONE, /* 无校验 */
  EVEN, /* 奇校验 */
  ODD,  /* 偶校验 */
};

/// @brief 枚举 Uart IO模式
enum IO_Mode
{
  RX,    /* 单接收模式 */
  TX,    /* 单发送模式 */
  RX_TX, /* 收发模式 */
};
}   // namespace Uart

/// @brief 名称空间 纯虚类
namespace virtual_class
{
class VUart : public system::IOStream
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VUart)
  NO_MOVE(VUart)

private:
  /// @brief VUart 端口号
  uint8_t         m_uart_port;
  /// @brief VUart 波特率
  uint32_t        m_baud_rate;
  /// @brief VUart 字长
  uint8_t         m_word_length;
  /// @brief VUart 停止位
  uint8_t         m_stop_bits;
  /// @brief VUart 校验方式
  Uart::Parity    m_parity;
  /// @brief VUart IO模式
  Uart::IO_Mode   m_mode;
  /// @brief VUart 接收工作模式
  Uart::Work_Mode m_rx_mode;
  /// @brief VUart 发送工作模式
  Uart::Work_Mode m_tx_mode;
  /// @brief VUart 接收缓存区大小(tmp大小)
  uint16_t        m_rx_tmp_size;

protected:
  static void send_complete_callback(void* arg);
  static void receive_complete_callback(void* arg);

  bool         hard_recv_wait_bit(uint32_t timeout) override;
  bool         hard_recv_clean_bit() override;
  uint32_t     hard_recv(void* buf, uint32_t length, uint32_t timeout) override;
  uint32_t     hard_send(const void* ram, uint32_t length) override;
  virtual void hard_recv_end() = 0;

  bool reload();

public:
  explicit VUart(const std::string& name, Object* parent = nullptr) : system::IOStream(name, parent) {}

  virtual bool open(const uint8_t& port, const uint32_t& baud_rate = 9600, const uint8_t& word_length = 8, const uint8_t& stop_bits = 1, const Uart::Parity& parity = Uart::NONE, const Uart::IO_Mode& mode = Uart::RX_TX, const Uart ::Work_Mode& rx_mode = Uart::DMA, const Uart::Work_Mode& tx_mode = Uart::DMA, const uint32_t& receive_tmp_size = UART_RECEIVE_TMP_SIZE, const uint32_t& istream_size = 256, const uint32_t& ostream_size = 128);

  virtual bool close() override
  {
    bool ret  = true;
    ret      &= (SUCESS == e_port_uart_deinit(m_uart_port));
    ret      &= IOStream::close();
    return ret;
  }

  bool set_baud_rate(const uint32_t& baud_rate)
  {
    if(m_baud_rate == baud_rate)
      return true;

    m_baud_rate = baud_rate;
    return reload();
  }

  bool set_word_length(const uint8_t& word_length)
  {
    if(m_word_length == word_length)
      return true;

    m_word_length = word_length;
    return reload();
  }

  bool set_stop_bits(const uint8_t& stop_bits)
  {
    if(m_stop_bits == stop_bits)
      return true;

    m_stop_bits = stop_bits;
    return reload();
  }

  bool set_parity(const Uart::Parity& parity)
  {
    if(m_parity == parity)
      return true;

    m_parity = parity;
    return reload();
  }

  uint32_t baud_rate() const
  {
    return m_baud_rate;
  }

  uint8_t word_length() const
  {
    return m_word_length;
  }

  uint8_t stop_bits() const
  {
    return m_stop_bits;
  }

  Uart::Parity parity() const
  {
    return m_parity;
  }

  virtual ~VUart()
  {
    if (is_open())
      e_port_uart_deinit(m_uart_port);
  }
};
} /* namespace virtual_class */
} /* namespace OwO */

#endif /* __VIRTUAL_UART_HPP__ */
