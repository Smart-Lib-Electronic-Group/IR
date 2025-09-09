#ifndef __MAIN_APP_HPP__
#define __MAIN_APP_HPP__

#include "port_net_init.h"
#include "port_iwdg.h"
#include "ir_app.hpp"
#include "key.hpp"

namespace OwO
{
namespace main
{
class main_app : public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(main_app)
  NO_MOVE(main_app)
private:
  typedef struct version_t
  {
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
  } version_t;

  typedef struct net_timeout_t
  {
    bool     is_connected;
    uint16_t disconnect_time;
  } net_timeout_t;

  protocol::modbus::Register&          holding_register;
  protocol::modbus::Register&          input_register;
  protocol::modbus::Modbus_Tcp_Server& modbus_tcp;

  rom&         eeprom;
  device::Key& bios_key;

  ir_app* ir = nullptr;

  bool          advanced_mode_flag;
  version_t     version;
  net_timeout_t net_timeout;

  void get_version(const char* date, const char* time)
  {
    const char* month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    for (uint16_t i = 0; i < 12; i++)
    {
      if (memcmp(date, month[i], 3) == 0)
      {
        version.month = i + 1;
        break;
      }
    }
    version.year   = (uint16_t)atoi(date + 6);
    version.day    = (uint16_t)atoi(date + 4);
    version.hour   = (uint16_t)atoi(time + 0);
    version.minute = (uint16_t)atoi(time + 3);
    version.second = (uint16_t)atoi(time + 6);
  }

protected:
  virtual void event_loop() override
  {
    process();
    check_timeout();
    check_bios_key();
    msleep(10);
  }

  void check_timeout()
  {
    if (eeprom().system.system_watch_dog_enable)
      e_port_iwdg_feed();

    if (eeprom().system.net_watch_dog_enable)
    {
      if (net_timeout.is_connected)
      {
        if (net_timeout.disconnect_time > eeprom().system.net_watch_dog_time)
          v_port_system_reset();

        if (modbus_tcp.client_count() == 0)
        {
          net_timeout.disconnect_time += 10;
        }
        else
        {
          net_timeout.disconnect_time = 0;
        }
      }
      else
      {
        if (modbus_tcp.client_count() > 0)
          net_timeout.is_connected = true;
      }
    }
  }

  void check_bios_key()
  {
    if (bios_key.read())
    {
      eeprom().bios_flag = 1;
      eeprom.update_bios_flag();
      v_port_system_reset();
    }
  }

  void process_input_register()
  {
    if (advanced_mode_flag)
    {
      input_register.set(version.year, 0);
      input_register.set(version.month, 1);
      input_register.set(version.day, 2);
      input_register.set(version.hour, 3);
      input_register.set(version.minute, 4);
      input_register.set(version.second, 5);

      uint16_t value = configTOTAL_HEAP_SIZE / 1024;
      input_register.set(value, 6);
      value = value - xPortGetFreeHeapSize() / 1024;
      input_register.set(value, 7);
      value = 100 - ul_port_os_get_space();
      input_register.set(value, 8);
    }
    else
      input_register.clear(0, 9);

    input_register.set(p_port_system_get_work_time()->days, 9);
    input_register.set(p_port_system_get_work_time()->hours, 10);
    input_register.set(p_port_system_get_work_time()->minutes, 11);
    input_register.set(p_port_system_get_work_time()->seconds, 12);
  }

  void process_holding_register()
  {
    if (!advanced_mode_flag && 1 == holding_register[0])
    {
      holding_register.mutex().lock();

      holding_register[2]  = eeprom().system.system_watch_dog_enable;
      holding_register[3]  = eeprom().system.net_watch_dog_enable;
      holding_register[4]  = eeprom().system.net_watch_dog_time;
      holding_register[6]  = eeprom().bios_flag;

      holding_register[8]  = eeprom().net.ip[0];
      holding_register[9]  = eeprom().net.ip[1];
      holding_register[10] = eeprom().net.ip[2];
      holding_register[11] = eeprom().net.ip[3];
      holding_register[12] = eeprom().net.mask[0];
      holding_register[13] = eeprom().net.mask[1];
      holding_register[14] = eeprom().net.mask[2];
      holding_register[15] = eeprom().net.mask[3];
      holding_register[16] = eeprom().net.gateway[0];
      holding_register[17] = eeprom().net.gateway[1];
      holding_register[18] = eeprom().net.gateway[2];
      holding_register[19] = eeprom().net.gateway[3];
      holding_register[20] = eeprom().net.modbus_server_port;
      holding_register[21] = eeprom().net.modbus_server_addr;

      holding_register.mutex().unlock();

      eeprom.set_addvance_flag(true);
      advanced_mode_flag = true;
    }

    if (advanced_mode_flag && 0 == holding_register[0])
    {
      holding_register.mutex().lock();

      eeprom().system.system_watch_dog_enable = holding_register[2];
      eeprom().system.net_watch_dog_enable    = holding_register[3];
      eeprom().system.net_watch_dog_time      = holding_register[4];
      eeprom().bios_flag                      = holding_register[6];

      eeprom().net.ip[0]                      = holding_register[8];
      eeprom().net.ip[1]                      = holding_register[9];
      eeprom().net.ip[2]                      = holding_register[10];
      eeprom().net.ip[3]                      = holding_register[11];
      eeprom().net.mask[0]                    = holding_register[12];
      eeprom().net.mask[1]                    = holding_register[13];
      eeprom().net.mask[2]                    = holding_register[14];
      eeprom().net.mask[3]                    = holding_register[15];
      eeprom().net.gateway[0]                 = holding_register[16];
      eeprom().net.gateway[1]                 = holding_register[17];
      eeprom().net.gateway[2]                 = holding_register[18];
      eeprom().net.gateway[3]                 = holding_register[19];
      eeprom().net.modbus_server_port         = holding_register[20];
      eeprom().net.modbus_server_addr         = holding_register[21];

      holding_register.mutex().unlock();

      eeprom.update_system();
      eeprom.update_bios_flag();
      eeprom.update_net();

      holding_register.clear(2, 3);
      holding_register.clear(6);
      holding_register.clear(8, 14);

      eeprom.set_addvance_flag(false);
      advanced_mode_flag = false;
    }

    if (advanced_mode_flag)
    {
      if (1 == holding_register[1])
      {
        holding_register.mutex().lock();

        eeprom().system.system_watch_dog_enable = holding_register[2];
        eeprom().system.net_watch_dog_enable    = holding_register[3];
        eeprom().system.net_watch_dog_time      = holding_register[4];
        eeprom().bios_flag                      = holding_register[6];

        eeprom().net.ip[0]                      = holding_register[8];
        eeprom().net.ip[1]                      = holding_register[9];
        eeprom().net.ip[2]                      = holding_register[10];
        eeprom().net.ip[3]                      = holding_register[11];
        eeprom().net.mask[0]                    = holding_register[12];
        eeprom().net.mask[1]                    = holding_register[13];
        eeprom().net.mask[2]                    = holding_register[14];
        eeprom().net.mask[3]                    = holding_register[15];
        eeprom().net.gateway[0]                 = holding_register[16];
        eeprom().net.gateway[1]                 = holding_register[17];
        eeprom().net.gateway[2]                 = holding_register[18];
        eeprom().net.gateway[3]                 = holding_register[19];
        eeprom().net.modbus_server_port         = holding_register[20];
        eeprom().net.modbus_server_addr         = holding_register[21];

        holding_register.mutex().unlock();

        eeprom.updata();
        v_port_system_reset();
      }

      if (1 == holding_register[22])
      {
        holding_register.mutex().lock();

        eeprom().net.ip[0]              = holding_register[8];
        eeprom().net.ip[1]              = holding_register[9];
        eeprom().net.ip[2]              = holding_register[10];
        eeprom().net.ip[3]              = holding_register[11];
        eeprom().net.mask[0]            = holding_register[12];
        eeprom().net.mask[1]            = holding_register[13];
        eeprom().net.mask[2]            = holding_register[14];
        eeprom().net.mask[3]            = holding_register[15];
        eeprom().net.gateway[0]         = holding_register[16];
        eeprom().net.gateway[1]         = holding_register[17];
        eeprom().net.gateway[2]         = holding_register[18];
        eeprom().net.gateway[3]         = holding_register[19];
        eeprom().net.modbus_server_port = holding_register[20];
        eeprom().net.modbus_server_addr = holding_register[21];

        holding_register.mutex().unlock();

        eeprom.update_net();

        v_port_net_reset_address_arr(eeprom().net.ip, eeprom().net.mask, eeprom().net.gateway);
        modbus_tcp.set_id(eeprom().net.modbus_server_addr);

        holding_register.clear(22);
      }
    }
  }

  void process()
  {
    process_input_register();
    process_holding_register();
  }

  void load_def_eeprom_data(bool is_load_ip = true)
  {
    eeprom().system.system_watch_dog_enable = 1;
    eeprom().system.net_watch_dog_enable    = 1;
    eeprom().system.net_watch_dog_time      = 5000;
    eeprom().bios_flag                      = 0;

    if (is_load_ip)
    {
      eeprom().net.ip[0]      = 192;
      eeprom().net.ip[1]      = 168;
      eeprom().net.ip[2]      = 1;
      eeprom().net.ip[3]      = 10;
      eeprom().net.mask[0]    = 255;
      eeprom().net.mask[1]    = 255;
      eeprom().net.mask[2]    = 254;
      eeprom().net.mask[3]    = 0;
      eeprom().net.gateway[0] = 192;
      eeprom().net.gateway[1] = 168;
      eeprom().net.gateway[2] = 1;
      eeprom().net.gateway[3] = 10;
    }

    eeprom().net.modbus_server_port = 502;
    eeprom().net.modbus_server_addr = 1;

    eeprom().ir.flash_time          = 500;
    eeprom().ir.type                = 0;
    eeprom().ir.auto_clean_flag     = 0;
    eeprom().ir.channel_01_enable   = 1;
    eeprom().ir.channel_02_enable   = 1;
    eeprom().ir.channel_03_enable   = 1;
    eeprom().ir.channel_04_enable   = 1;
    eeprom().ir.channel_05_enable   = 1;
    eeprom().ir.channel_06_enable   = 1;
    eeprom().ir.channel_07_enable   = 1;
    eeprom().ir.channel_08_enable   = 1;
    eeprom().ir.channel_pulse       = 32;

    eeprom.updata();
  }

public:
  main_app(const std::string& name, Object* parent = nullptr) : system::kernel::Thread(name, parent), holding_register(*new protocol::modbus::Register("HOLDING_REGISTER", this, 60)), input_register(*new protocol::modbus::Register("INPUT_REGISTER", this, 20)), modbus_tcp(*new protocol::modbus::Modbus_Tcp_Server("MODBUS_TCP", this)), eeprom(*new rom("EEPROM", this)), bios_key(*new device::Key("BIOS_KEY", this))
  {
    get_version(__DATE__, __TIME__);

    eeprom.open();
    eeprom.download();

    if (eeprom.check())
      load_def_eeprom_data();
    else
    {
      if (eeprom().net.ip[0] != 192 || eeprom().net.ip[1] != 168 || eeprom().net.gateway[0] != 192 || eeprom().net.gateway[1] != 168)
        load_def_eeprom_data();
      else
      {
        if (eeprom().net.modbus_server_port == 0 || eeprom().ir.flash_time == 0)
          load_def_eeprom_data(false);
      }
    }

    e_port_net_init_arr(eeprom().net.ip, eeprom().net.mask, eeprom().net.gateway);

    if (eeprom().system.system_watch_dog_enable)
      e_port_iwdg_init(PORT_IWDG_32, 5000);
  }

  void start(uint8_t priority = THREAD_DEF_PRIORITY)
  {
    ir = new ir_app("IR", this, holding_register, eeprom);
    ir->open();
    ir->start(priority + 3);

    modbus_tcp.set_holding_registers(holding_register);
    modbus_tcp.set_input_registers(input_register);
    modbus_tcp.start(eeprom().net.modbus_server_port, eeprom().net.modbus_server_addr, protocol::modbus::Modbus_TCP, priority + 2);

    bios_key.open(Gpio::PA, 0, device::Key_Type::Long_Press, 5000, Gpio::LEVEL_HIGH);

    system::kernel::Thread::start(priority - 1, 256, 0);
  }

  virtual ~main_app() {}
};
} /* namespace main */
} /* namespace OwO */

#endif /* __MAIN_APP_HPP__ */
