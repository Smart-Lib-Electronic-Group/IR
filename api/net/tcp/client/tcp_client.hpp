#ifndef __TCP_CLIENT_HPP__
#define __TCP_CLIENT_HPP__

#include "iostream.hpp"
#include "signal.hpp"
#include "event_flags.hpp"

#include "FreeRTOS_Sockets.h"

namespace OwO
{
namespace tcp
{
class Tcp_Client : public system::IOStream
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Tcp_Client)
  NO_MOVE(Tcp_Client)

  friend class Server;

private:
  char*    m_client_ip;
  char*    m_server_ip;
  uint16_t m_client_port;
  uint16_t m_server_port;
  Socket_t m_socket;

protected:
  system::kernel::Event_Flags m_recv_event;
  system::kernel::Mutex       m_open_mutex;

protected:
  virtual void send_start() override {};
  virtual void send_end() override {};

  virtual uint32_t hard_recv(void* buf, uint32_t length, uint32_t timeout) override;
  virtual uint32_t hard_send(const void* buf, uint32_t length) override;

  virtual bool hard_recv_wait_bit(uint32_t timeout) override;
  virtual bool hard_recv_clean_bit() override;

  virtual bool open(Socket_t client_socket, freertos_sockaddr* sockaddr, const uint32_t& istream_size = 128, const uint32_t& ostream_size = 128);
  void         data_input();
  static void  process_addr(freertos_sockaddr* sockaddr, char* ip, uint16_t& port);

  Socket_t fd()
  {
    return m_socket;
  }

public:
  system::Signal<IOStream*> signal_recv_finished;

  explicit Tcp_Client(const std::string& name = "tcp_base_client", Object* parent = nullptr);

  virtual bool open(const char* server_ip, uint16_t server_port, const uint32_t& istream_size = 128, const uint32_t& ostream_size = 128);

  virtual bool close();

  char* client_ip() const
  {
    return m_client_ip;
  }

  uint16_t client_port() const
  {
    return m_client_port;
  }

  char* server_ip() const
  {
    return m_server_ip;
  }

  uint16_t server_port() const
  {
    return m_server_port;
  }

  virtual ~Tcp_Client()
  {
    close();
    Free(m_client_ip);
    Free(m_server_ip);
  }
};

/*
class client : public Tcp_Client, public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(client)
  NO_MOVE(client)

private:
  using system::kernel::Thread::exit;
  using system::kernel::Thread::is_finished;
  using system::kernel::Thread::is_running;
  using system::kernel::Thread::loop_time;
  using system::kernel::Thread::quit;
  using system::kernel::Thread::set_loop_time;
  using system::kernel::Thread::start;

  char*    m_tmp;
  uint32_t m_tmp_length;
  int      m_rece_length;
  char*    m_server_ip;
  uint16_t m_server_port;

  virtual void run() override;

protected:
  virtual void event_loop() override {}
  virtual void write_start() override {};
  virtual void write_end() override {};

public:
  explicit client(const std::string& name, Object* parent = nullptr) : Tcp_Client(name, parent), system::kernel::Thread(name, parent)
  {
    m_tmp         = nullptr;
    m_tmp_length  = 0;
    m_rece_length = 0;
  }
  virtual bool open(const char* ip, uint16_t port, const Stream_Mode& mode = Stream_Mode::Both_Buffer, const uint32_t& istream_size = 128, const uint32_t& ostream_size = 128) override;
  virtual bool close() override;
  virtual ~client()
  {
    close();
  }
};
*/

} /* namespace tcp */
} /* namespace OwO */

#endif /* __TCP_CLIENT_HPP__ */
