#ifndef __MODBUS_SLAVE_HPP__
#define __MODBUS_SLAVE_HPP__

#include "iostream.hpp"
#include "thread.hpp"
#include "coil.hpp"

namespace OwO
{
namespace protocol
{
namespace modbus
{
class Modbus_Slave : public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Modbus_Slave)
  NO_MOVE(Modbus_Slave)
private:
  enum Modbus_Code
  {
    READ_HOLDING_COILS       = 1,
    READ_INPUT_COILS         = 2,
    READ_HOLDING_REGISTERS   = 3,
    READ_INPUT_REGISTERS     = 4,
    WRITE_SINGLE_COIL        = 5,
    WRITE_SINGLE_REGISTER    = 6,
    WRITE_MULTIPLE_COILS     = 15,
    WRITE_MULTIPLE_REGISTERS = 16,
    REPORT_SLAVE_ID          = 17,
    MASK_WRITE_REGISTER      = 22,
    READ_WRITE_REGISTERS     = 23,
  };

  enum Modbus_Error_Code
  {
    ILLEGAL_FUNC_CODE  = 1,
    ILLEGAL_ADDR_CODE  = 2,
    ILLEGAL_VALUE_CODE = 3,
  };

private:
  uint8_t                       m_slave_address;
  uint8_t*                      m_recv_buffer;
  uint8_t*                      m_send_buffer;
  Coil*                         m_holding_coils;
  Coil*                         m_input_coils;
  Register*                     m_holding_registers;
  Register*                     m_input_registers;
  Modbus_Mode                   m_mode;
  mutable system::kernel::Mutex m_mutex;

private:
  bool check_crc(const uint8_t* data, uint32_t length)
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
    return crc == 0;
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

  uint16_t get_response_read_holding_coils(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr  = (request[2] << 8) | request[3];
    uint16_t coils_count = (request[4] << 8) | request[5];

    if (start_addr + coils_count > m_holding_coils->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    uint16_t coils_byte_count = ((coils_count % 8) ? 1 : 0) + coils_count / 8;

    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = READ_HOLDING_COILS;
      response[2] = coils_byte_count;
      m_holding_coils->read(response + 3, coils_count, start_addr);
      add_crc(response, 3 + coils_byte_count);
      return (5 + coils_byte_count);
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = (coils_byte_count + 3) >> 8;
      response[5] = (coils_byte_count + 3) & 0xFF;
      response[6] = m_slave_address;
      response[7] = READ_HOLDING_COILS;
      response[8] = coils_byte_count;
      m_holding_coils->read(response + 9, coils_count, start_addr);
      return (9 + coils_byte_count);
    }
    return 0;
  }

  uint16_t get_response_read_input_coils(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr  = (request[2] << 8) | request[3];
    uint16_t coils_count = (request[4] << 8) | request[5];

    if (start_addr + coils_count > m_input_coils->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    uint16_t coils_byte_count = ((coils_count % 8) ? 1 : 0) + coils_count / 8;

    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = READ_INPUT_COILS;
      response[2] = coils_byte_count;
      m_input_coils->read(response + 3, coils_count, start_addr);
      add_crc(response, 3 + coils_byte_count);
      return (5 + coils_byte_count);
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = (coils_byte_count + 3) >> 8;
      response[5] = (coils_byte_count + 3) & 0xFF;
      response[6] = m_slave_address;
      response[7] = READ_INPUT_COILS;
      response[8] = coils_byte_count;
      m_input_coils->read(response + 9, coils_count, start_addr);
      return (9 + coils_byte_count);
    }
    return 0;
  }

  uint16_t get_response_read_holding_registers(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr      = (request[2] << 8) | request[3];
    uint16_t registers_count = (request[4] << 8) | request[5];

    if (start_addr + registers_count > m_holding_registers->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = READ_HOLDING_REGISTERS;
      response[2] = registers_count * 2;
      m_holding_registers->read(response + 3, registers_count, start_addr);
      add_crc(response, 3 + registers_count * 2);
      return (5 + registers_count * 2);
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = (registers_count * 2 + 3) >> 8;
      response[5] = (registers_count * 2 + 3) & 0xFF;
      response[6] = m_slave_address;
      response[7] = READ_HOLDING_REGISTERS;
      response[8] = registers_count * 2;
      m_holding_registers->read(response + 9, registers_count, start_addr);
      return (9 + registers_count * 2);
    }
    return 0;
  }

  uint16_t get_response_read_input_registers(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr      = (request[2] << 8) | request[3];
    uint16_t registers_count = (request[4] << 8) | request[5];

    if (start_addr + registers_count > m_input_registers->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = READ_INPUT_REGISTERS;
      response[2] = registers_count * 2;
      m_input_registers->read(response + 3, registers_count, start_addr);
      add_crc(response, 3 + registers_count * 2);
      return (5 + registers_count * 2);
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = (registers_count * 2 + 3) >> 8;
      response[5] = (registers_count * 2 + 3) & 0xFF;
      response[6] = m_slave_address;
      response[7] = READ_INPUT_REGISTERS;
      response[8] = registers_count * 2;
      m_input_registers->read(response + 9, registers_count, start_addr);
      return (9 + registers_count * 2);
    }
    return 0;
  }

  uint16_t get_response_write_single_coil(const uint8_t* request, uint8_t* response)
  {
    uint16_t addr = (request[2] << 8) | request[3];

    if (addr >= m_holding_coils->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    m_holding_coils->write(request[4], addr);

    if (Modbus_RTU == m_mode)
    {
      memcpy(response, request, 8);
      return 8;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x06;
      memcpy(response + 6, request, 6);
      return 12;
    }
    return 0;
  }

  uint16_t get_response_write_multiple_coils(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr  = (request[2] << 8) | request[3];
    uint16_t coils_count = (request[4] << 8) | request[5];
    uint8_t  byte_count  = request[6];

    if (start_addr + coils_count > m_holding_coils->size() || byte_count != ((coils_count % 8) ? 1 : 0 + coils_count / 8))
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    m_holding_coils->write(request + 7, coils_count, start_addr);

    if (Modbus_RTU == m_mode)
    {
      memcpy(response, request, 6);
      add_crc(response, 6);
      return 8;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x06;
      memcpy(response + 6, request, 6);
      return 12;
    }
    return 0;
  }

  uint16_t get_response_write_single_register(const uint8_t* request, uint8_t* response)
  {
    uint16_t addr = (request[2] << 8) | request[3];

    if (addr >= m_holding_registers->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    m_holding_registers->write(request + 4, 1, addr);

    if (Modbus_RTU == m_mode)
    {
      memcpy(response, request, 8);
      return 8;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x06;
      memcpy(response + 6, request, 6);
      return 12;
    }
    return 0;
  }

  uint16_t get_response_write_multiple_registers(const uint8_t* request, uint8_t* response)
  {
    uint16_t start_addr      = (request[2] << 8) | request[3];
    uint16_t registers_count = (request[4] << 8) | request[5];
    uint8_t  byte_count      = request[6];

    if (start_addr + registers_count > m_holding_registers->size() || byte_count != registers_count * 2)
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    m_holding_registers->write(request + 7, registers_count, start_addr);

    if (Modbus_RTU == m_mode)
    {
      memcpy(response, request, 6);
      add_crc(response, 6);
      return 8;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x06;
      memcpy(response + 6, request, 6);
      return 12;
    }
    return 0;
  }

  uint16_t get_response_report_slave_id(const uint8_t* request, uint8_t* response)
  {
    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = REPORT_SLAVE_ID;
      response[2] = 0x09;
      response[3] = 0xFF;

      memset(response + 4, 0, 10);

      if (m_holding_coils)
      {
        response[4] = m_holding_coils->size() << 8;
        response[5] = m_holding_coils->size() & 0xFF;
      }

      if (m_input_coils)
      {
        response[6] = m_input_coils->size() << 8;
        response[7] = m_input_coils->size() & 0xFF;
      }

      if (m_holding_registers)
      {
        response[8] = m_holding_registers->size() << 8;
        response[9] = m_holding_registers->size() & 0xFF;
      }

      if (m_input_registers)
      {
        response[10] = m_input_registers->size() << 8;
        response[11] = m_input_registers->size() & 0xFF;
      }

      add_crc(response, 12);
      return 14;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x0C;
      response[6] = m_slave_address;
      response[7] = REPORT_SLAVE_ID;
      response[8] = 0x09;
      response[9] = 0xFF;

      memset(response + 10, 0, 10);

      if (m_holding_coils)
      {
        response[10] = m_holding_coils->size() >> 8;
        response[11] = m_holding_coils->size() & 0xFF;
      }

      if (m_input_coils)
      {
        response[12] = m_input_coils->size() >> 8;
        response[13] = m_input_coils->size() & 0xFF;
      }

      if (m_holding_registers)
      {
        response[14] = m_holding_registers->size() >> 8;
        response[15] = m_holding_registers->size() & 0xFF;
      }

      if (m_input_registers)
      {
        response[16] = m_input_registers->size() >> 8;
        response[17] = m_input_registers->size() & 0xFF;
      }

      return 18;
    }
    return 0;
  }

  uint16_t get_response_mask_write_register(const uint8_t* request, uint8_t* response)
  {
    uint16_t addr = (request[2] << 8) | request[3];

    if (addr >= m_holding_registers->size())
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    uint16_t and_mask = (request[4] << 8) | request[5];
    uint16_t or_mask  = (request[6] << 8) | request[7];

    m_holding_registers->mask_write(and_mask, or_mask, addr);

    if (Modbus_RTU == m_mode)
    {
      memcpy(response, request, 8);
      add_crc(response, 8);
      return 10;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x06;
      memcpy(response + 6, request, 6);
      return 12;
    }
    return 0;
  }

  uint16_t get_response_read_write_registers(const uint8_t* request, uint8_t* response)
  {
    uint16_t read_start_addr       = (request[2] << 8) | request[3];
    uint16_t read_registers_count  = (request[4] << 8) | request[5];
    uint16_t write_start_addr      = (request[6] << 8) | request[7];
    uint16_t write_registers_count = (request[8] << 8) | request[9];
    uint8_t  byte_count            = request[10];

    if (read_start_addr + read_registers_count > m_holding_registers->size() || write_start_addr + write_registers_count > m_holding_registers->size() || byte_count != write_registers_count * 2)
      return create_exception_response(request, response, ILLEGAL_ADDR_CODE);

    m_holding_registers->write(request + 11, write_registers_count, write_start_addr);

    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = READ_WRITE_REGISTERS;
      response[2] = read_registers_count * 2;
      m_holding_registers->read(response + 3, read_registers_count, read_start_addr);
      response[3 + read_registers_count * 2] = write_registers_count * 2;
      add_crc(response, 3 + read_registers_count * 2);
      return (5 + read_registers_count * 2);
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = (read_registers_count * 2 + 3) >> 8;
      response[5] = (read_registers_count * 2 + 3) & 0xFF;
      response[6] = m_slave_address;
      response[7] = READ_WRITE_REGISTERS;
      response[8] = read_registers_count * 2;
      m_holding_registers->read(response + 9, read_registers_count, read_start_addr);
      return (9 + read_registers_count * 2);
    }
    return 0;
  }

  uint16_t create_exception_response(const uint8_t* request, uint8_t* response, uint8_t error_code)
  {
    if (Modbus_RTU == m_mode)
    {
      response[0] = m_slave_address;
      response[1] = static_cast<uint8_t>(request[1] | 0x80);
      response[2] = error_code;
      add_crc(response, 3);
      return 5;
    }
    else if (Modbus_TCP == m_mode)
    {
      response[4] = 0x00;
      response[5] = 0x03;
      response[6] = m_slave_address;
      response[7] = static_cast<uint8_t>(request[1] | 0x80);
      response[8] = error_code;
      return 9;
    }
    return 0;
  }

  uint16_t process_request(const uint8_t* request, uint8_t* response)
  {
    switch (request[1])
    {
      case READ_HOLDING_COILS :
        if (m_holding_coils)
          return get_response_read_holding_coils(request, response);
      case READ_INPUT_COILS :
        if (m_input_coils)
          return get_response_read_input_coils(request, response);
      case READ_HOLDING_REGISTERS :
        if (m_holding_registers)
          return get_response_read_holding_registers(request, response);
      case READ_INPUT_REGISTERS :
        if (m_input_registers)
          return get_response_read_input_registers(request, response);
      case WRITE_SINGLE_COIL :
        if (m_holding_coils)
          return get_response_write_single_coil(request, response);
      case WRITE_SINGLE_REGISTER :
        if (m_holding_registers)
          return get_response_write_single_register(request, response);
      case WRITE_MULTIPLE_COILS :
        if (m_holding_coils)
          return get_response_write_multiple_coils(request, response);
      case WRITE_MULTIPLE_REGISTERS :
        if (m_holding_registers)
          return get_response_write_multiple_registers(request, response);
      case REPORT_SLAVE_ID :
        return get_response_report_slave_id(request, response);
      case MASK_WRITE_REGISTER :
        if (m_holding_registers)
          return get_response_mask_write_register(request, response);
      case READ_WRITE_REGISTERS :
        if (m_holding_registers)
          return get_response_read_write_registers(request, response);
      default :
        return create_exception_response(request, response, ILLEGAL_FUNC_CODE);
    }
  }

  int32_t get_length(uint8_t func_code)
  {
    switch (func_code)
    {
      case WRITE_MULTIPLE_COILS :
      case WRITE_MULTIPLE_REGISTERS :
      case READ_WRITE_REGISTERS :
        return -1;
      case REPORT_SLAVE_ID :
        return 2;
      case MASK_WRITE_REGISTER :
        return 8;
      case READ_HOLDING_COILS :
      case READ_INPUT_COILS :
      case READ_HOLDING_REGISTERS :
      case READ_INPUT_REGISTERS :
      case WRITE_SINGLE_COIL :
      case WRITE_SINGLE_REGISTER :
      default :
        return 6;
    }
  }

  void process_tcp_frame(system::IOStream* iostream)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    if (iostream->recv(m_recv_buffer, 7) != 7)
    {
      iostream->istream_reset();
      return;
    }

    if (m_recv_buffer[6] != m_slave_address)
    {
      iostream->istream_reset();
      return;
    }

    uint16_t length = (m_recv_buffer[4] << 8) | m_recv_buffer[5];
    if (iostream->recv(m_recv_buffer + 7, length - 1) != length - 1)
    {
      iostream->istream_reset();
      return;
    }

    length = process_request(m_recv_buffer + 6, m_send_buffer);
    memcpy(m_send_buffer, m_recv_buffer, 4);
    iostream->send(m_send_buffer, length);
    iostream->istream_reset();
  }

  void process_rtu_frame(system::IOStream* iostream)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    if (iostream->recv(m_recv_buffer, 2) != 2)
    {
      iostream->istream_reset();
      return;
    }

    if (m_recv_buffer[0] != m_slave_address)
    {
      iostream->istream_reset();
      return;
    }

    uint8_t  func_code = m_recv_buffer[1];
    uint16_t length    = get_length(func_code);
    if (-1 == get_length(func_code))
    {
      if (func_code == WRITE_MULTIPLE_COILS || func_code == WRITE_MULTIPLE_REGISTERS)
      {
        if (iostream->recv(m_recv_buffer + 2, 5) != 5)
        {
          iostream->istream_reset();
          return;
        }

        if (iostream->recv(m_recv_buffer + 7, m_recv_buffer[6] + 2) != m_recv_buffer[6] + 2)
        {
          iostream->istream_reset();
          return;
        }

        length = m_recv_buffer[6] + 9;
      }
      else if (func_code == READ_WRITE_REGISTERS)
      {
        if (iostream->recv(m_recv_buffer + 2, 9) != 9)
        {
          iostream->istream_reset();
          return;
        }

        if (iostream->recv(m_recv_buffer + 11, m_recv_buffer[10] + 2) != m_recv_buffer[10] + 2)
        {
          iostream->istream_reset();
          return;
        }

        length = m_recv_buffer[10] + 13;
      }
    }
    else
    {
      if (iostream->recv(m_recv_buffer + 2, length) != length)
      {
        iostream->istream_reset();
        return;
      }

      length += 2;
    }

    if (!check_crc(m_recv_buffer, length))
    {
      iostream->istream_reset();
      return;
    }

    length = process_request(m_recv_buffer, m_send_buffer);
    iostream->send(m_send_buffer, length);
    iostream->istream_reset();
  }

protected:
  virtual void event_loop() override {}

public:
  Modbus_Slave(const std::string& name, Object* parent) : Thread(name, parent)
  {
    m_recv_buffer       = static_cast<uint8_t*>(Malloc(256));
    m_send_buffer       = static_cast<uint8_t*>(Malloc(256));
    m_slave_address     = 0x01;
    m_holding_coils     = nullptr;
    m_input_coils       = nullptr;
    m_holding_registers = nullptr;
    m_input_registers   = nullptr;
    m_mode              = Modbus_RTU;
  }

  virtual void start(uint8_t id, Modbus_Mode mode, uint8_t priority = THREAD_DEF_PRIORITY, uint16_t stack_size = 256)
  {
    m_slave_address = id;
    m_mode          = mode;
    set_wait_time(WAIT_FOREVER);
    Thread::start(priority, stack_size, 16);
  }

  void process(system::IOStream* iostream)
  {
    if (Modbus_TCP == m_mode)
      process_tcp_frame(iostream);
    else if (Modbus_RTU == m_mode)
      process_rtu_frame(iostream);
  }

  void set_mode(Modbus_Mode mode)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_mode = mode;
  }

  void set_id(uint8_t id)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_slave_address = id;
  }

  void set_holding_coils(Coil* coils)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_holding_coils = coils;
  }

  void set_input_coils(Coil* coils)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_input_coils = coils;
  }

  void set_holding_registers(Register* registers)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_holding_registers = registers;
  }

  void set_input_registers(Register* registers)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_input_registers = registers;
  }

  void set_holding_coils(Coil& coils)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_holding_coils = &coils;
  }

  void set_input_coils(Coil& coils)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_input_coils = &coils;
  }

  void set_holding_registers(Register& registers)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_holding_registers = &registers;
  }

  void set_input_registers(Register& registers)
  {
    system::kernel::Mutex_Guard locker(m_mutex);
    m_input_registers = &registers;
  }

  virtual ~Modbus_Slave()
  {
    Free(m_recv_buffer);
    Free(m_send_buffer);
  }
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace OwO */

#endif /* __MODBUS_SLAVE_HPP__ */
