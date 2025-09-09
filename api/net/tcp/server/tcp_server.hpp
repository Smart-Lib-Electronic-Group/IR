#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#include "thread.hpp"
#include "tcp_client.hpp"

namespace OwO
{
namespace tcp
{
class Tcp_Client;

class Server : public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Server)
  NO_MOVE(Server)

  friend class Tcp_Client;

private:
  char*                  m_server_ip;
  uint16_t               m_server_port;
  Socket_t               m_server_socket;
  uint32_t               m_client_istream_size;
  uint32_t               m_client_ostream_size;
  SocketSet_t            m_socket_set;
  std::list<Tcp_Client*> m_clients;

  virtual void run() override;
  virtual void event_loop() {};

  void server_loop();
  void server_init();
  void add_client();
  void write_client();
  void clear_client();

  using system::kernel::Thread::exit;
  using system::kernel::Thread::is_finished;
  using system::kernel::Thread::is_running;
  using system::kernel::Thread::quit;

protected:
  void remove_client(Tcp_Client* client);

  void get_server_addr(char* ip, uint16_t& port)
  {
    port = m_server_port;
    memcpy(ip, m_server_ip, strlen(m_server_ip));
  }

  virtual void client_connect(Tcp_Client* client) {};
  virtual void client_disconnect(Tcp_Client* client) {};

public:
  system::Signal<Tcp_Client*> signal_client_connect;
  system::Signal<Tcp_Client*> signal_client_disconnect;

  Server(const std::string& name = "tcp_server", Object* parent = nullptr);

  virtual void start(uint16_t port, uint8_t priority = THREAD_DEF_PRIORITY, uint32_t client_istream_size = 256, uint32_t client_ostream_size = 0);
  virtual void stop();
  uint8_t      client_count() const
  {
    return (uint8_t)m_clients.size();
  }
  Tcp_Client* get_client(uint8_t index) const
  {
    return *m_clients.begin();
  }
  virtual ~Server();
};
} /* namespace tcp */
} /* namespace OwO */

#endif /* __TCP_SERVER_HPP__ */
