#ifndef __MODBUS_MASTER_HPP__
#define __MODBUS_MASTER_HPP__

#include "register.hpp"
#include "iostream.hpp"
#include "thread.hpp"
#include <random>

namespace OwO
{
namespace protocol
{
namespace modbus
{
typedef struct MODBUS_REQUEST
{
  O_MEMORY
  uint8_t           slave_id;
  uint8_t           function_code;
  uint16_t          reg_addr;
  uint16_t          reg_length;
  uint16_t          timeout;
  bool              circle;
  Modbus_Mode       mode;
  system::IOStream* io;
  Register*         data;
} modbus_request;

class Modbus_Master : public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Modbus_Master)
  NO_MOVE(Modbus_Master)
private:
  enum Modbus_Code
  {
    READ_HOLDING_REGISTERS   = 0x03,
    READ_INPUT_REGISTERS     = 0x04,
    WRITE_SINGLE_REGISTER    = 0x06,
    WRITE_MULTIPLE_REGISTERS = 0x10
  };

  system::kernel::Mutex      m_mutex;
  std::list<modbus_request*> m_requests;
  uint8_t*                   m_recv_buffer;
  uint8_t*                   m_send_buffer;
  std::random_device         m_random;
  uint32_t                   m_mbap_code;

protected:
  uint32_t get_rand_code()
  {
    static std::mt19937                            engine(m_random());
    static std::uniform_int_distribution<uint32_t> distribution;
    return distribution(engine);
  }

  bool check_crc(const uint8_t* data, uint32_t length)
  {
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < length - 2; ++i)
    {
      crc ^= data[i];
      for (int j = 0; j < 8; ++j)
      {
        if (crc & 0x0001)
        {
          crc = (crc >> 1) ^ 0xA001;
        }
        else
        {
          crc >>= 1;
        }
      }
    }
    return (crc == ((data[length - 2] << 8) | data[length - 1]));
  }

  void add_crc(uint8_t* data, uint32_t length)
  {
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < length; ++i)
    {
      crc ^= data[i];
      for (int j = 0; j < 8; ++j)
      {
        if (crc & 0x0001)
        {
          crc = (crc >> 1) ^ 0xA001;
        }
        else
        {
          crc >>= 1;
        }
      }
    }
    data[length]     = crc & 0xFF;
    data[length + 1] = crc >> 8;
  }

  uint16_t creat_tcp_request(modbus_request* request, uint8_t* send_buffer)
  {
    uint16_t index       = 0;
    /* 添加MBAP头 */
    send_buffer[index++] = m_mbap_code >> 24;
    send_buffer[index++] = m_mbap_code >> 16;
    send_buffer[index++] = m_mbap_code >> 8;
    send_buffer[index++] = m_mbap_code;
    /* 清空数据长度 */
    send_buffer[index++] = 0x00;
    send_buffer[index++] = 0x00;
    /* 添加从机地址 */
    send_buffer[index++] = request->slave_id;
    /* 添加功能码 */
    send_buffer[index++] = request->function_code;
    /* 添加寄存器地址 */
    send_buffer[index++] = request->reg_addr >> 8;
    send_buffer[index++] = request->reg_addr & 0xFF;

    if (READ_HOLDING_REGISTERS == request->function_code || READ_INPUT_REGISTERS == request->function_code)
    {
      /* 添加寄存器长度 */
      send_buffer[index++] = request->reg_length >> 8;
      send_buffer[index++] = request->reg_length & 0xFF;
    }
    else if (WRITE_SINGLE_REGISTER == request->function_code)
    {
      /* 添加数据 */
      request->data->write(send_buffer + index,1, request->reg_addr);
      index += 2;
    }
    else if (WRITE_MULTIPLE_REGISTERS == request->function_code)
    {
      /* 添加寄存器长度 */
      send_buffer[index++] = request->reg_length >> 8;
      send_buffer[index++] = request->reg_length & 0xFF;
      /* 添加字节计数 */
      send_buffer[index++] = request->reg_length * 2;
      /* 添加数据 */
      request->data->read(send_buffer + index, request->reg_length, request->reg_addr);
      index += request->reg_length * 2;
    }
    else
      return 0;

    /* 设置数据长度 */
    send_buffer[4] = (index - 6) >> 8;
    send_buffer[5] = (index - 6) & 0xFF;

    return index;
  }

  uint16_t creat_rtu_request(modbus_request* request, uint8_t* send_buffer)
  {
    uint16_t index       = 0;
    /* 添加从机地址 */
    send_buffer[index++] = request->slave_id;
    /* 添加功能码 */
    send_buffer[index++] = request->function_code;
    /* 添加寄存器地址 */
    send_buffer[index++] = request->reg_addr >> 8;
    send_buffer[index++] = request->reg_addr & 0xFF;

    if (READ_HOLDING_REGISTERS == request->function_code || READ_INPUT_REGISTERS == request->function_code)
    {
      /* 添加寄存器长度 */
      send_buffer[index++] = request->reg_length >> 8;
      send_buffer[index++] = request->reg_length & 0xFF;
    }
    else if (WRITE_SINGLE_REGISTER == request->function_code)
    {
      /* 添加数据 */
      request->data->read(send_buffer + index, 1, request->reg_addr);
      index += 2;
    }
    else if (WRITE_MULTIPLE_REGISTERS == request->function_code)
    {
      /* 添加寄存器长度 */
      send_buffer[index++] = request->reg_length >> 8;
      send_buffer[index++] = request->reg_length & 0xFF;
      /* 添加字节计数 */
      send_buffer[index++] = request->reg_length * 2;
      /* 添加数据 */
      request->data->read(send_buffer + index, request->reg_length, request->reg_addr);
      index += request->reg_length * 2;
    }
    else
      return 0;

    /* 添加CRC校验码 */
    add_crc(send_buffer, index);
    return index + 2;
  }

  bool anlyze_tcp_response(modbus_request* request, system::IOStream* port)
  {
    uint16_t length = 0;
    uint16_t index  = 0;

    /* 读取MBAP头 */
    if (port->recv(m_recv_buffer, 4) != 4)
      return false;

    /* 校验MBAP头 */
    if (m_recv_buffer[0] != m_mbap_code >> 24 || m_recv_buffer[1] != m_mbap_code >> 16 || m_recv_buffer[2] != m_mbap_code >> 8 || m_recv_buffer[3] != m_mbap_code)
      return false;
    else
      index += 4;

    /* 读取数据 */
    if (port->recv(m_recv_buffer + index, 4) != 4)
      return false;
    else
    {
      length  = (m_recv_buffer[index] << 8) | m_recv_buffer[index + 1];
      index  += 2;
    }

    /* 校验从机地址 */
    if (m_recv_buffer[index] != request->slave_id)
      return false;
    else
      index += 1;

    /* 校验功能码 */
    if (request->function_code != m_recv_buffer[index])
      return false;
    else
      index += 1;

    if (READ_HOLDING_REGISTERS == request->function_code || READ_INPUT_REGISTERS == request->function_code)
    {
      /* 读取字节长度 */
      if (port->recv(m_recv_buffer + index, 1) != 1)
        return false;

      /* 校验字节长度 */
      if (m_recv_buffer[index] != request->reg_length * 2)
        return false;
      else
      {
        index += 1;
        /* 读取数据 */
        if (port->recv(m_recv_buffer + index, m_recv_buffer[index]) != length)
          return false;
        else
        {
          request->data->write(&m_recv_buffer[index], request->reg_length, request->reg_addr);
          index += m_recv_buffer[index];
        }
      }
    }
    else if (WRITE_SINGLE_REGISTER == request->function_code)
    {
      if (port->recv(m_recv_buffer + index, 4) != 4)
        return false;
      else
      {
        if (m_recv_buffer[index] != (request->reg_addr >> 8) || m_recv_buffer[index + 1] != (request->reg_addr & 0xFF))
          return false;

        index += 2;

        if (m_recv_buffer[index] != ((*request->data)[request->reg_addr] >> 8) || m_recv_buffer[index + 1] != ((*request->data)[request->reg_addr] & 0xFF))
          return false;

        index += 2;
      }
    }
    else if (WRITE_MULTIPLE_REGISTERS == request->function_code)
    {
      if (port->recv(m_recv_buffer + index, 4) != 4)
        return false;
      else
      {
        if (m_recv_buffer[index] != (request->reg_addr >> 8) || m_recv_buffer[index + 1] != (request->reg_addr & 0xFF))
          return false;

        index += 2;

        if (m_recv_buffer[index] != (request->reg_length >> 8) || m_recv_buffer[index + 1] != (request->reg_length & 0xFF))
          return false;

        index += 2;
      }
    }

    return index == length + 6;
  }

  bool anlyze_rtu_response(modbus_request* request, system::IOStream* port)
  {
    uint16_t index = 0;

    /* 读取从机地址 */
    if (port->recv(m_recv_buffer, 2) != 2)
      return false;

    /* 校验从机地址 */
    if (m_recv_buffer[index] != request->slave_id)
      return false;
    else
      index += 1;

    /* 校验功能码 */
    if (m_recv_buffer[index] != request->function_code)
      return false;
    else
      index += 1;

    if (READ_HOLDING_REGISTERS == request->function_code || READ_INPUT_REGISTERS == request->function_code)
    {
      /* 读取寄存器长度 */
      if (port->recv(m_recv_buffer + index, 1) != 1)
        return false;

      /* 校验寄存器长度 */
      if (m_recv_buffer[index] != request->reg_length * 2)
        return false;
      else
      {
        /* 读取数据 */
        if (port->recv(m_recv_buffer + index + 1, m_recv_buffer[index] + 2) != m_recv_buffer[index] + 2)
          return false;
        else
        {
          if (check_crc(m_recv_buffer, index + m_recv_buffer[index] + 3))
            return false;

          request->data->write(&m_recv_buffer[index + 1], request->reg_length, request->reg_addr);
          index += m_recv_buffer[index];
        }
      }
    }
    else if (WRITE_SINGLE_REGISTER == request->function_code)
    {
      if (port->recv(m_recv_buffer + index, 6) != 6)
        return false;
      else
      {
        if (!check_crc(m_recv_buffer, 8))
          return false;

        if (m_recv_buffer[index] != (request->reg_addr >> 8) || m_recv_buffer[index + 1] != (request->reg_addr & 0xFF))
          return false;

        index += 2;

        if (m_recv_buffer[index] != ((*request->data)[request->reg_addr] >> 8) || m_recv_buffer[index + 1] != ((*request->data)[request->reg_addr] & 0xFF))
          return false;

        index += 2;
      }
    }
    else if (WRITE_MULTIPLE_REGISTERS == request->function_code)
    {
      if (port->recv(m_recv_buffer + index, 8) != 8)
        return false;
      else
      {
        if (!check_crc(m_recv_buffer, 10))
          return false;

        if (m_recv_buffer[index] != (request->reg_addr >> 8) || m_recv_buffer[index + 1] != (request->reg_addr & 0xFF))
          return false;

        index += 2;

        if (m_recv_buffer[index] != (request->reg_length >> 8) || m_recv_buffer[index + 1] != (request->reg_length & 0xFF))
          return false;

        index += 2;
      }
    }
    return true;
  }

  bool process_response(modbus_request* request, system::IOStream* port)
  {
    switch (request->mode)
    {
      case Modbus_TCP :
        return anlyze_tcp_response(request, port);
      case Modbus_RTU :
        return anlyze_rtu_response(request, port);
      default :
        return false;
    }
  }

  uint16_t process_request(modbus_request* request, uint8_t* send_buffer)
  {
    switch (request->mode)
    {
      case Modbus_TCP :
        m_mbap_code = get_rand_code();
        return creat_tcp_request(request, send_buffer);
      case Modbus_RTU :
        return creat_rtu_request(request, send_buffer);
      default :
        return 0;
    }
  }

  bool process_function(modbus_request* request)
  {
    uint16_t length = process_request(request, m_send_buffer);
    if (length == 0)
      return false;

    if (length != request->io->send(m_send_buffer, length))
      return false;

    request->io->set_istream_time_out(request->timeout);

    if (process_response(request, request->io))
      return true;
    else
    {
      request->io->istream_reset();
      return false;
    }
  }

  virtual void event_loop() override
  {
    if (m_requests.empty())
      return;

    system::kernel::Mutex_Guard locker(m_mutex);
    uint32_t                    count = m_requests.size();
    auto                        it    = m_requests.begin();
    while (count--)
    {
      process_function(*it);
      if (!(*it)->circle)
        delete *it;
      it++;
    }
  }

public:
  Modbus_Master(const std::string& name = "Modbus_Master", Object* parent = nullptr) : Thread(name, parent)
  {
    m_recv_buffer = static_cast<uint8_t*>(Malloc(256));
    m_send_buffer = static_cast<uint8_t*>(Malloc(256));
  }

  virtual void start(uint8_t priority = THREAD_DEF_PRIORITY, uint16_t stack_size = 256)
  {
    Thread::start(priority, stack_size, 4);
  }

  bool add_request(const modbus_request& request)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_requests.push_back(new modbus_request(request));
    return true;
  }

  bool read_holding_registers(system::IOStream* io, Register* data, uint8_t slave_id, uint16_t reg_addr, uint16_t reg_length, Modbus_Mode mode = Modbus_RTU, bool circle = true, uint16_t timeout = 1000)
  {
    return add_request({ slave_id, READ_HOLDING_REGISTERS, reg_addr, reg_length, timeout, circle, mode, io, data });
  }

  bool read_input_registers(system::IOStream* io, Register* data, uint8_t slave_id, uint16_t reg_addr, uint16_t reg_length, Modbus_Mode mode = Modbus_RTU, bool circle = true, uint16_t timeout = 1000)
  {
    return add_request({ slave_id, READ_INPUT_REGISTERS, reg_addr, reg_length, timeout, circle, mode, io, data });
  }

  bool write_single_register(system::IOStream* io, Register* data, uint8_t slave_id, uint16_t reg_addr, Modbus_Mode mode = Modbus_RTU, bool circle = false, uint16_t timeout = 1000)
  {
    return add_request({ slave_id, WRITE_SINGLE_REGISTER, reg_addr, 1, timeout, circle, mode, io, data });
  }

  bool write_multiple_registers(system::IOStream* io, Register* data, uint8_t slave_id, uint16_t reg_addr, uint16_t reg_length, Modbus_Mode mode = Modbus_RTU, bool circle = false, uint16_t timeout = 1000)
  {
    return add_request({ slave_id, WRITE_MULTIPLE_REGISTERS, reg_addr, reg_length, timeout, circle, mode, io, data });
  }

  void set_flash_time(uint32_t time)
  {
    set_wait_time(time);
  }

  bool remove_request(system::IOStream* port)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    for (auto it = m_requests.begin(); it != m_requests.end(); ++it)
    {
      if ((*it)->io == port)
      {
        delete *it;
        m_requests.erase(it);
        return true;
      }
    }
    return false;
  }

  virtual ~Modbus_Master()
  {
    Free(m_recv_buffer);
    Free(m_send_buffer);
  }
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace OwO */

#endif /* __MODBUS_MASTER_HPP__ */
