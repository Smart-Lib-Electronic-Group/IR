#ifndef __IR_APP_HPP__
#define __IR_APP_HPP__

#include "modbus_server.hpp"
#include "rom.hpp"
#include "ir.hpp"

namespace OwO
{
namespace main
{
class ir_app : public system::kernel::Thread
{
  O_MEMORY
  O_OBJECT
  NO_COPY(ir_app)
  NO_MOVE(ir_app)
private:
  protocol::modbus::Register& holding_register;
  rom&                        eeprom;

  device::IR& ir_1;
  device::IR& ir_2;
  device::IR& ir_3;
  device::IR& ir_4;
  device::IR& ir_5;
  device::IR& ir_6;
  device::IR& ir_7;
  device::IR& ir_8;

  bool    m_addvance_flag = false;
  bool    m_refresh_flag  = false;
  uint8_t ir_channel;
  uint8_t ir_data_count;
  uint8_t ir_data_len;
  char    ir_data[30];

  static constexpr inline uint16_t ir_holding_reg_start_addr = 23;

protected:
  virtual void event_loop() override
  {
    process();
    msleep(eeprom().ir.flash_time);
  }

  void set_channel_pulse(uint8_t pulse)
  {
    ir_1.set_pulse_width(pulse);
    ir_2.set_pulse_width(pulse);
    ir_3.set_pulse_width(pulse);
    ir_4.set_pulse_width(pulse);
    ir_5.set_pulse_width(pulse);
    ir_6.set_pulse_width(pulse);
    ir_7.set_pulse_width(pulse);
    ir_8.set_pulse_width(pulse);
  }

  void process_addvance()
  {
    if (true == eeprom.get_addvance_flag() && false == m_addvance_flag)
    {
      holding_register.mutex().lock();

      holding_register[ir_holding_reg_start_addr + 19] = eeprom().ir.flash_time;
      holding_register[ir_holding_reg_start_addr + 20] = eeprom().ir.type;
      holding_register[ir_holding_reg_start_addr + 21] = eeprom().ir.auto_clean_flag;
      holding_register[ir_holding_reg_start_addr + 22] = eeprom().ir.channel_pulse;
      holding_register[ir_holding_reg_start_addr + 23] = eeprom().ir.channel_01_enable;
      holding_register[ir_holding_reg_start_addr + 24] = eeprom().ir.channel_02_enable;
      holding_register[ir_holding_reg_start_addr + 25] = eeprom().ir.channel_03_enable;
      holding_register[ir_holding_reg_start_addr + 26] = eeprom().ir.channel_04_enable;
      holding_register[ir_holding_reg_start_addr + 27] = eeprom().ir.channel_05_enable;
      holding_register[ir_holding_reg_start_addr + 28] = eeprom().ir.channel_06_enable;
      holding_register[ir_holding_reg_start_addr + 29] = eeprom().ir.channel_07_enable;
      holding_register[ir_holding_reg_start_addr + 30] = eeprom().ir.channel_08_enable;

      holding_register.mutex().unlock();
      m_addvance_flag = true;
    }

    if (false == eeprom.get_addvance_flag() && true == m_addvance_flag)
    {
      holding_register.clear(ir_holding_reg_start_addr + 19, 12);

      m_addvance_flag = false;
    }

    if (true == m_addvance_flag)
    {
      holding_register.mutex().lock();

      if (eeprom().ir.flash_time != holding_register[ir_holding_reg_start_addr + 19])
      {
        eeprom().ir.flash_time = holding_register[ir_holding_reg_start_addr + 19];
        m_refresh_flag         = true;
      }

      if (eeprom().ir.type != holding_register[ir_holding_reg_start_addr + 20])
      {
        eeprom().ir.type = holding_register[ir_holding_reg_start_addr + 20];
        m_refresh_flag   = true;
      }

      if (eeprom().ir.auto_clean_flag != holding_register[ir_holding_reg_start_addr + 21])
      {
        eeprom().ir.auto_clean_flag = holding_register[ir_holding_reg_start_addr + 21];
        m_refresh_flag              = true;
      }

      if (eeprom().ir.channel_pulse != holding_register[ir_holding_reg_start_addr + 22])
      {
        eeprom().ir.channel_pulse = holding_register[ir_holding_reg_start_addr + 22];
        m_refresh_flag            = true;
      }

      if (eeprom().ir.channel_01_enable != holding_register[ir_holding_reg_start_addr + 23])
      {
        eeprom().ir.channel_01_enable = holding_register[ir_holding_reg_start_addr + 23];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_02_enable != holding_register[ir_holding_reg_start_addr + 24])
      {
        eeprom().ir.channel_02_enable = holding_register[ir_holding_reg_start_addr + 24];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_03_enable != holding_register[ir_holding_reg_start_addr + 25])
      {
        eeprom().ir.channel_03_enable = holding_register[ir_holding_reg_start_addr + 25];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_04_enable != holding_register[ir_holding_reg_start_addr + 26])
      {
        eeprom().ir.channel_04_enable = holding_register[ir_holding_reg_start_addr + 26];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_05_enable != holding_register[ir_holding_reg_start_addr + 27])
      {
        eeprom().ir.channel_05_enable = holding_register[ir_holding_reg_start_addr + 27];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_06_enable != holding_register[ir_holding_reg_start_addr + 28])
      {
        eeprom().ir.channel_06_enable = holding_register[ir_holding_reg_start_addr + 28];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_07_enable != holding_register[ir_holding_reg_start_addr + 29])
      {
        eeprom().ir.channel_07_enable = holding_register[ir_holding_reg_start_addr + 29];
        m_refresh_flag                = true;
      }

      if (eeprom().ir.channel_08_enable != holding_register[ir_holding_reg_start_addr + 30])
      {
        eeprom().ir.channel_08_enable = holding_register[ir_holding_reg_start_addr + 30];
        m_refresh_flag                = true;
      }

      holding_register.mutex().unlock();

      if (m_refresh_flag)
      {
        set_channel_pulse(eeprom().ir.channel_pulse);
        eeprom.update_ir();
        m_refresh_flag = false;
      }
    }
  }

  void process()
  {
    if (true == eeprom.get_addvance_flag() || true == m_addvance_flag)
    {
      process_addvance();
    }

    if (1 == holding_register[ir_holding_reg_start_addr + 3])
    {
      holding_register.get(ir_channel, ir_holding_reg_start_addr + 0);
      holding_register.get(ir_data_count, ir_holding_reg_start_addr + 1);
      holding_register.get(ir_data_len, ir_holding_reg_start_addr + 2);
      holding_register.get(ir_data, 30, ir_holding_reg_start_addr + 4);

      if (ir_channel == 1)
        ir_1.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 2)
        ir_2.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 3)
        ir_3.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 4)
        ir_4.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 5)
        ir_5.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 6)
        ir_6.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 7)
        ir_7.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);
      else if (ir_channel == 8)
        ir_8.send(static_cast<device::ir_type>(eeprom().ir.type), ir_data, ir_data_len, ir_data_count, eeprom().ir.flash_time);

      if (eeprom().ir.auto_clean_flag)
      {
        holding_register.set(1, ir_holding_reg_start_addr + 0);
        holding_register.set(1, ir_holding_reg_start_addr + 1);

        holding_register.clear(ir_holding_reg_start_addr + 2);
        holding_register.clear(ir_holding_reg_start_addr + 4, 15);
      }

      holding_register.clear(ir_holding_reg_start_addr + 3);
    }
  }

public:
  ir_app(const std::string& name, Object* parent, protocol::modbus::Register& holding_register, rom& eeprom) : system::kernel::Thread(name, parent), holding_register(holding_register), eeprom(eeprom), ir_1(*new device::IR("IR1", this)), ir_2(*new device::IR("IR2", this)), ir_3(*new device::IR("IR3", this)), ir_4(*new device::IR("IR4", this)), ir_5(*new device::IR("IR5", this)), ir_6(*new device::IR("IR6", this)), ir_7(*new device::IR("IR7", this)), ir_8(*new device::IR("IR8", this)) {}

  void open()
  {
    if (eeprom().ir.channel_08_enable)
    {
      ir_8.open(Gpio::PC, 6, eeprom().ir.channel_pulse);
      holding_register.set(8, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_07_enable)
    {
      ir_7.open(Gpio::PC, 7, eeprom().ir.channel_pulse);
      holding_register.set(7, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_06_enable)
    {
      ir_6.open(Gpio::PC, 8, eeprom().ir.channel_pulse);
      holding_register.set(6, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_05_enable)
    {
      ir_5.open(Gpio::PC, 9, eeprom().ir.channel_pulse);
      holding_register.set(5, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_04_enable)
    {
      ir_4.open(Gpio::PA, 8, eeprom().ir.channel_pulse);
      holding_register.set(4, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_03_enable)
    {
      ir_3.open(Gpio::PA, 9, eeprom().ir.channel_pulse);
      holding_register.set(3, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_02_enable)
    {
      ir_2.open(Gpio::PA, 10, eeprom().ir.channel_pulse);
      holding_register.set(2, ir_holding_reg_start_addr + 0);
    }

    if (eeprom().ir.channel_01_enable)
    {
      ir_1.open(Gpio::PA, 11, eeprom().ir.channel_pulse);
      holding_register.set(1, ir_holding_reg_start_addr + 0);
    }

    holding_register.set(1, ir_holding_reg_start_addr + 1);
  }

  void start(uint8_t priority = THREAD_DEF_PRIORITY)
  {
    system::kernel::Thread::start(priority, 256, 0);
  }

  virtual ~ir_app() {}
};
} /* namespace main */
} /* namespace OwO */

#endif /* __IR_APP_HPP__ */
