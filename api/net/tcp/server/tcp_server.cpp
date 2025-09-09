#include "tcp_server.hpp"

#include "FreeRTOS_IP.h"

using namespace OwO::tcp;
using namespace OwO::system;
using namespace kernel;

O_METAOBJECT(Server, Thread)

void Server::run()
{
  server_init();
  while (!is_finished())
  {
    server_loop();
    event_loop();
    event();
  }
}

void Server::server_loop()
{
  int cnt = FreeRTOS_select(m_socket_set, portMAX_DELAY);

  if (cnt & eSELECT_READ)
  {
    if (FreeRTOS_FD_ISSET(m_server_socket, m_socket_set))
      add_client();
    else
      write_client();
  }
  else if (cnt & eSELECT_EXCEPT)
  {
    clear_client();
  }
}

void Server::add_client()
{
  struct freertos_sockaddr client_addr;
  uint32_t                 addr_len      = sizeof(client_addr);
  Socket_t                 client_socket = FreeRTOS_accept(m_server_socket, &client_addr, &addr_len);

  if (client_socket != NULL)
  {
    Tcp_Client* client = new Tcp_Client("tcp_client", this);
    if (client->open(client_socket, &client_addr, m_client_istream_size, m_client_ostream_size))
    {
      FreeRTOS_FD_SET(client_socket, m_socket_set, eSELECT_READ | eSELECT_EXCEPT);
      m_clients.push_front(client);
      client_connect(client);
      signal_client_connect(client);
    }
    else
      delete client;
  }
}

void Server::remove_client(Tcp_Client* client)
{
  signal_client_disconnect(client);
  client_disconnect(client);
  FreeRTOS_FD_CLR(client->fd(), m_socket_set, eSELECT_ALL);
  m_clients.remove(client);
}

void Server::write_client()
{
  if (m_clients.empty())
    return;

  for (auto client : m_clients)
  {
    if (FreeRTOS_FD_ISSET(client->fd(), m_socket_set))
      client->data_input();
  }
}

void Server::clear_client()
{
  if (m_clients.empty())
    return;

  auto        it            = m_clients.begin();
  Tcp_Client* delete_client = nullptr;

  while (it != m_clients.end())
  {
    if (FreeRTOS_FD_ISSET((*it)->fd(), m_socket_set))
    {
      delete_client = *it;
      ++it;
      delete delete_client;
    }
    else
      ++it;
  }
}

void Server::server_init()
{
  struct freertos_sockaddr server_addr;
  uint32_t                 addr_len = sizeof(server_addr);

  m_server_socket                   = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
  server_addr.sin_family            = FREERTOS_AF_INET;
  server_addr.sin_port              = FreeRTOS_htons(m_server_port);
  server_addr.sin_addr              = FreeRTOS_htonl(FREERTOS_INADDR_ANY);

  FreeRTOS_bind(m_server_socket, &server_addr, addr_len);
  FreeRTOS_listen(m_server_socket, 5);

  m_socket_set = FreeRTOS_CreateSocketSet();
  FreeRTOS_FD_SET(m_server_socket, m_socket_set, eSELECT_READ | eSELECT_EXCEPT);
  Tcp_Client::process_addr(&server_addr, m_server_ip, m_server_port);
}

Server::Server(const std::string& name, Object* parent) : Thread(name, parent)
{
  m_server_ip     = (char*)Malloc(16);
  m_server_port   = 0;
  m_server_socket = nullptr;
  m_socket_set    = nullptr;
}

void Server::start(uint16_t port, uint8_t priority, uint32_t client_istream_size, uint32_t client_ostream_size)
{
  m_server_port         = port;
  m_client_istream_size = client_istream_size;
  m_client_ostream_size = client_ostream_size;
  Thread::start(priority, 256, 4);
}

void Server::stop()
{
  Thread::exit(1);
  join();
  if (!m_clients.empty())
  {
    for (auto client : m_clients)
      delete client;
  }
  if (m_server_socket != nullptr)
  {
    FreeRTOS_shutdown(m_server_socket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(m_server_socket);
  }
  if (m_socket_set != nullptr)
    FreeRTOS_DeleteSocketSet(m_socket_set);
  if (m_server_ip != nullptr)
    Free(m_server_ip);
}

Server::~Server()
{
  stop();
}
