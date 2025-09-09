#ifndef __ROM_HPP__
#define __ROM_HPP__

#include "eprom.hpp"

namespace OwO
{
namespace main
{
class rom : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(rom)
  NO_MOVE(rom)
public:
#pragma pack(push, 1)
  typedef struct
  {
    char     name[8];
    uint32_t password;
  } user_info_t;

  typedef struct
  {
    uint8_t  ip[4];
    uint8_t  mask[4];
    uint8_t  gateway[4];
    uint8_t  modbus_server_addr;
    uint16_t modbus_server_port;
    uint8_t  command_server_port;
    uint8_t  tmp[5];
  } net_info_t;

  typedef struct
  {
    uint16_t net_watch_dog_time;
    uint8_t  protocol_type;
    bool     system_watch_dog_enable : 1;
    bool     net_watch_dog_enable    : 1;
    uint8_t  tmp                     : 6;
  } system_info_t;

  typedef struct
  {
    uint16_t flash_time;
    uint8_t  type              : 5;
    bool     auto_clean_flag   : 1;
    bool     channel_01_enable : 1;
    bool     channel_02_enable : 1;
    bool     channel_03_enable : 1;
    bool     channel_04_enable : 1;
    bool     channel_05_enable : 1;
    bool     channel_06_enable : 1;
    bool     channel_07_enable : 1;
    bool     channel_08_enable : 1;
    uint8_t  tmp               : 2;
    uint8_t  channel_pulse;
  } ir_info_t;

  typedef struct
  {
    user_info_t   user;
    uint8_t       bios_flag;
    net_info_t    net;
    system_info_t system;
    ir_info_t     ir;
  } rom_info_t;

private:
  device::EPROM* m_rom;
  rom_info_t     m_info;
  bool           m_addvance_flag = false;

#pragma pack(pop)

public:
  rom(const std::string& name = "ROM", Object* parent = nullptr) : Object(name, parent)
  {
    m_rom = new device::EPROM(name + "_EPROM", this);
  }

  bool open()
  {
    if (is_open())
      return false;

    if (m_rom->open(Gpio::PH, 4, Gpio::PH, 5, device::EPROM::AT24C02))
      m_is_open = true;

    return m_is_open;
  }

  bool close()
  {
    if (!is_open())
      return true;

    m_rom->close();
    m_is_open = false;

    return true;
  }

  bool download()
  {
    return (sizeof(rom_info_t) == m_rom->read(0, (uint8_t*)&m_info, sizeof(rom_info_t)));
  }

  bool updata()
  {
    return (sizeof(rom_info_t) == m_rom->write(0, (uint8_t*)&m_info, sizeof(rom_info_t)));
  }

  bool check()
  {
    return m_rom->check();
  }

  bool update_bios_flag()
  {
    return (1 == m_rom->write(offsetof(rom_info_t, bios_flag), (uint8_t*)&m_info.bios_flag, 1));
  }

  bool update_net()
  {
    return (sizeof(net_info_t) == m_rom->write(offsetof(rom_info_t, net), (uint8_t*)&m_info.net, sizeof(net_info_t)));
  }

  bool update_system()
  {
    return (sizeof(system_info_t) == m_rom->write(offsetof(rom_info_t, system), (uint8_t*)&m_info.system, sizeof(system_info_t)));
  }

  bool update_ir()
  {
    return (sizeof(ir_info_t) == m_rom->write(offsetof(rom_info_t, ir), (uint8_t*)&m_info.ir, sizeof(ir_info_t)));
  }

  void set_addvance_flag(bool flag)
  {
    m_addvance_flag = flag;
  }

  bool get_addvance_flag()
  {
    return m_addvance_flag;
  }

  rom_info_t& operator()()
  {
    return m_info;
  }

  virtual ~rom()
  {
    close();
    delete m_rom;
  }
};
} /* namespace main */
} /* namespace OwO */

#endif /* __ROM_HPP__ */
