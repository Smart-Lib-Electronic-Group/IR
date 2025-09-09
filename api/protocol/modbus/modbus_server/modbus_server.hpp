#ifndef __MODBUS_SERVER_HPP__
#define __MODBUS_SERVER_HPP__

#include "tcp_server.hpp"
#include "modbus_slave.hpp"

namespace OwO
{
namespace protocol
{
namespace modbus
{
class Modbus_Tcp_Server : public tcp::Server
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Modbus_Tcp_Server)
  NO_MOVE(Modbus_Tcp_Server)
private:
  Modbus_Slave* m_modbus_tcp;

protected:
  virtual void client_connect(tcp::Tcp_Client* client) override
  {
    connect(client->signal_recv_finished, m_modbus_tcp, &Modbus_Slave::process, system::Connection_Queued);
  };

  virtual void client_disconnect(tcp::Tcp_Client* client) override {};

public:
  Modbus_Tcp_Server(const std::string& name, Object* parent) : tcp::Server(name, parent)
  {
    m_modbus_tcp = new Modbus_Slave("modbus_tcp", this);
  }

  virtual void start(uint16_t port = 502, uint8_t id = 1, Modbus_Mode mode = Modbus_TCP, uint8_t priority = THREAD_DEF_PRIORITY)
  {
    m_modbus_tcp->start(id, mode, priority, 512);
    tcp::Server::start(port, priority);
    tcp::Server::set_priority(priority - 1);
  }

  virtual void stop()
  {
    tcp::Server::stop();
  }

  void set_mode(Modbus_Mode mode)
  {
    m_modbus_tcp->set_mode(mode);
  }

  void set_id(uint8_t id)
  {
    m_modbus_tcp->set_id(id);
  }

  void set_holding_coils(Coil* coils)
  {
    m_modbus_tcp->set_holding_coils(coils);
  }

  void set_input_coils(Coil* coils)
  {
    m_modbus_tcp->set_input_coils(coils);
  }

  void set_holding_registers(Register* registers)
  {
    m_modbus_tcp->set_holding_registers(registers);
  }

  void set_input_registers(Register* registers)
  {
    m_modbus_tcp->set_input_registers(registers);
  }

  void set_holding_coils(Coil& coils)
  {
    m_modbus_tcp->set_holding_coils(&coils);
  }

  void set_input_coils(Coil& coils)
  {
    m_modbus_tcp->set_input_coils(&coils);
  }

  void set_holding_registers(Register& registers)
  {
    m_modbus_tcp->set_holding_registers(&registers);
  }

  void set_input_registers(Register& registers)
  {
    m_modbus_tcp->set_input_registers(&registers);
  }

  virtual ~Modbus_Tcp_Server() {}
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace OwO */

#endif /* __MODBUS_SERVER_HPP__ */
