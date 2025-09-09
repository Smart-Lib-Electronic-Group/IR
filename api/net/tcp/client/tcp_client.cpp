#include "Tcp_Client.hpp"
#include "tcp_server.hpp"

#include "FreeRTOS_IP.h"

#define RECV_EVENT_FLAG 0x01

using namespace OwO::tcp;
using namespace OwO::system;
using namespace kernel;

O_METAOBJECT(Tcp_Client, IOStream)

uint32_t Tcp_Client::hard_recv(void* buf, uint32_t length, uint32_t timeout)
{
  return FreeRTOS_recv(m_socket, buf, length, timeout);
}

uint32_t Tcp_Client::hard_send(const void* buf, uint32_t length)
{
  uint32_t ret = FreeRTOS_send(m_socket, buf, length, 0);
  hard_send_end();
  return ret;
}

bool Tcp_Client::hard_recv_wait_bit(uint32_t timeout)
{
  Mutex_Guard locker(m_open_mutex);
  return m_recv_event.wait(RECV_EVENT_FLAG, timeout);
}

bool Tcp_Client::hard_recv_clean_bit()
{
  return m_recv_event.clear(RECV_EVENT_FLAG);
}

bool Tcp_Client::open(Socket_t client_socket, freertos_sockaddr* sockaddr, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (false == IOStream::open(istream_size, ostream_size))
    return false;

  if (client_socket < 0)
  {
    close();
    return false;
  }

  uint32_t timeout = 2000;
  if (FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_RCVTIMEO, &timeout, 0) != 0)
  {
    close();
    return false;
  }

  if (FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_SNDTIMEO, &timeout, 0) != 0)
  {
    close();
    return false;
  }

  m_socket = client_socket;
  process_addr(sockaddr, m_client_ip, m_client_port);
  dynamic_cast<Server*>(parent())->get_server_addr(m_server_ip, m_server_port);

  m_recv_event.clear(RECV_EVENT_FLAG);
  m_open_mutex.unlock();
  return true;
}

void Tcp_Client::data_input()
{
  uint32_t avail = istream_size() - IOStream::istream_available();
  if (0 == avail)
    return;

  char* buf    = (char*)Malloc(avail);
  int   length = FreeRTOS_recv(m_socket, buf, avail, 0);
  if (length < 0)
  {
    Free(buf);
    delete this;
  }
  else
  {
    hard_recv_input(buf, length);
    signal_recv_finished(this);
    m_recv_event.set(RECV_EVENT_FLAG);
    Free(buf);
  }
}

void Tcp_Client::process_addr(freertos_sockaddr* sockaddr, char* ip, uint16_t& port)
{
  port = FreeRTOS_ntohs(sockaddr->sin_port);
  FreeRTOS_inet_ntoa(sockaddr->sin_addr, ip);
}

Tcp_Client::Tcp_Client(const std::string& name, Object* parent) : IOStream(name, parent)
{
  m_open_mutex.lock();
  m_client_ip   = (char*)Malloc(16);
  m_server_ip   = (char*)Malloc(16);
  m_client_port = 0;
  m_server_port = 0;
  m_socket      = nullptr;
}

bool Tcp_Client::open(const char* server_ip, uint16_t server_port, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (is_open())
    return false;

  m_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
  if (m_socket == nullptr)
    return false;

  uint32_t timeout = 2000;
  if (FreeRTOS_setsockopt(m_socket, 0, FREERTOS_SO_RCVTIMEO, &timeout, 0) != 0)
  {
    close();
    return false;
  }

  if (FreeRTOS_setsockopt(m_socket, 0, FREERTOS_SO_SNDTIMEO, &timeout, 0) != 0)
  {
    close();
    return false;
  }

  freertos_sockaddr server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = FREERTOS_AF_INET;
  server_addr.sin_port   = FreeRTOS_htons(server_port);
  server_addr.sin_addr   = FreeRTOS_inet_addr(server_ip);

  process_addr(&server_addr, m_client_ip, m_client_port);

  if (FreeRTOS_connect(m_socket, &server_addr, sizeof(server_addr)) != 0)
  {
    FreeRTOS_shutdown(m_socket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(m_socket);
    return false;
  }

  process_addr(&server_addr, m_client_ip, m_client_port);

  bool ret = IOStream::open(istream_size, ostream_size);
  m_recv_event.clear(RECV_EVENT_FLAG);
  m_open_mutex.unlock();
  return ret;
}

bool Tcp_Client::close()
{
  if (!is_open())
    return true;

  if (dynamic_cast<Server*>(parent()))
    dynamic_cast<Server*>(parent())->remove_client(this);

  memset(m_client_ip, 0, 16);
  memset(m_server_ip, 0, 16);
  m_client_port = 0;
  m_server_port = 0;

  if (m_socket != nullptr)
  {
    FreeRTOS_shutdown(m_socket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(m_socket);
  }

  m_socket = nullptr;
  m_open_mutex.lock();
  m_recv_event.set(RECV_EVENT_FLAG);

  return IOStream::close();
}

/*
O_METAOBJECT(client, Tcp_Client)

void OwO::tcp::client::run()
{
  while (!is_finished())
  {
    event();
    if (Stream_Mode::Read_Buffer == stream_mode() || Stream_Mode::Both_Buffer == stream_mode())
    {
      if (!is_open())
      {
        Tcp_Client::open(m_server_ip, m_server_port, stream_mode(), istream_size(), ostream_size());
        sleep(1);
        continue;
      }

      m_rece_length = ::recv(fd(), m_tmp, m_tmp_length, 0);
      if (m_rece_length < 0)
      {
        Tcp_Client::close();
        sleep(1);
        continue;
      }
      hard_recv_input(m_tmp, m_rece_length);
      signal_recv_finished(this);
      m_recv_event.set(RECV_EVENT_FLAG);
    }
    else
    {
      if (!is_open())
        Tcp_Client::open(m_server_ip, m_server_port, stream_mode(), istream_size(), ostream_size());
    }
    event_loop();
  }
}

bool OwO::tcp::client::open(const char* ip, uint16_t port, const Stream_Mode& mode, const uint32_t& istream_size, const uint32_t& ostream_size)
{
  if (is_open())
    return false;

  if (Stream_Mode::No_Buffer == mode || Stream_Mode::Write_Buffer == mode)
  {
    m_server_ip = (char*)Malloc(INET_ADDRSTRLEN);
    memcpy(m_server_ip, ip, strlen(ip));
    m_server_port = port;

    Thread::set_loop_time(100);
    Thread::start(3, 256, 16);

    return Tcp_Client::open(ip, port, mode, istream_size, ostream_size);
  }
  else
  {
    m_server_ip = (char*)Malloc(INET_ADDRSTRLEN);
    memcpy(m_server_ip, ip, strlen(ip));
    m_server_port = port;

    m_tmp_length  = istream_size;
    m_tmp         = (char*)Malloc(istream_size);
    m_rece_length = 0;

    Tcp_Client::open(m_server_ip, m_server_port, mode, istream_size, ostream_size);
    Thread::set_loop_time(100);
    Thread::start(3, 256, 16);

    return true;
  }
}

bool OwO::tcp::client::close()
{
  if (!is_open())
    return true;

  Thread::exit(1);

  bool ret = Tcp_Client::close();
  if (m_tmp)
    Free(m_tmp);

  m_tmp         = nullptr;
  m_rece_length = 0;
  m_tmp_length  = 0;

  sleep(2);

  wait_exit();
  return ret;
}
*/