#ifndef __VIRTUAL_IIC_HPP__
#define __VIRTUAL_IIC_HPP__

#include "thread.hpp"
#include "ioport.hpp"
#include "virtual_gpio.hpp"

/// @brief 名称空间 库名
namespace OwO
{
namespace virtual_class
{
/// @brief 虚拟IIC-软件模拟
class VSoft_IIC : public OwO::system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VSoft_IIC)
  NO_MOVE(VSoft_IIC)
private:
  uint8_t    m_address_type;
  uint8_t    m_clk_time;
  Class::io* m_sda;
  Class::io* m_scl;

private:
  void clk_delay()
  {
    system::kernel::Thread::usleep(m_clk_time);
  }

  void byte_delay(uint32_t time)
  {
    system::kernel::Thread::msleep(time);
  }

  void m_start()
  {
    m_sda->set_mode(Gpio::OUT_OD);

    m_sda->high();
    m_scl->high();
    clk_delay();
    m_sda->low();
    clk_delay();
    m_scl->low();
  }

  void m_stop()
  {
    m_sda->set_mode(Gpio::OUT_OD);

    m_sda->low();
    clk_delay();
    m_scl->high();
    clk_delay();
    m_sda->high();
    clk_delay();
  }

  void m_ack()
  {
    m_sda->set_mode(Gpio::OUT_OD);

    m_sda->low();
    clk_delay();
    m_scl->high();
    clk_delay();
    m_scl->low();
    clk_delay();
  }

  void m_nack()
  {
    m_sda->set_mode(Gpio::OUT_OD);

    m_sda->high();
    clk_delay();
    m_scl->high();
    clk_delay();
    m_scl->low();
    clk_delay();
  }

  void m_write_byte(const uint8_t byte)
  {
    m_sda->set_mode(Gpio::OUT_OD);

    for (int i = 0; i < 8; i++)
    {
      m_sda->write((byte << i) & 0x80);
      clk_delay();
      m_scl->high();
      clk_delay();
      m_scl->low();
      clk_delay();
    }

    m_sda->high();
    clk_delay();
  }

  bool m_wait_ack()
  {
    uint8_t time = 0;

    m_sda->set_mode(Gpio::IN);

    m_scl->high();
    clk_delay();

    while (1 == m_sda->read())
    {
      if (time > 250)
      {
        m_stop();
        return true;
      }
      else
      {
        clk_delay();
        time++;
      }
    }

    m_scl->low();
    clk_delay();

    return false;
  }

  bool m_busy_wait(uint32_t time)
  {
    uint32_t t = 0;

    while (0 == m_scl->read())
    {
      if (t >= time)
        return true;
      else
      {
        clk_delay();
        t++;
      }
    }

    return false;
  }

  bool m_read_byte(uint8_t& byte, bool ack)
  {
    uint8_t data = 0;

    m_sda->set_mode(Gpio::IN);
    clk_delay();
    m_scl->high();

    if (m_busy_wait(200))
    {
      m_stop();
      return true;
    }
    clk_delay();

    data <<= 1;
    if (1 == m_sda->read())
      data++;

    m_scl->low();
    clk_delay();

    for (uint8_t i = 1; i < 8; i++)
    {
      data <<= 1;
      m_scl->high();
      clk_delay();

      if (1 == m_sda->read())
        data++;

      m_scl->low();
      clk_delay();
    }

    if (ack)
      m_ack();
    else
      m_nack();

    byte = data;
    return false;
  }

protected:
  uint32_t memory_read(void* data, uint32_t len, uint8_t device_address, uint32_t reg_address)
  {
    uint8_t* p     = (uint8_t*)data;
    uint32_t count = 0;

    m_start();

    if (m_address_type == 0)
      m_write_byte(device_address + ((reg_address >> 8) << 1));
    else if (m_address_type == 1)
      m_write_byte(device_address | 0);
    else if (m_address_type == 2)
    {
      m_write_byte(device_address | 0);
      if (m_wait_ack())
        return count;
      m_write_byte((uint8_t)(reg_address >> 8));
    }

    if (m_wait_ack())
      return count;
    m_write_byte((uint8_t)reg_address);

    if (m_wait_ack())
      return count;

    m_start();
    m_write_byte(device_address | 1);

    if (m_wait_ack())
      return count;

    while (len)
    {
      if (len > 1)
      {
        if (m_read_byte(*p, true))
          return count;
      }
      else
      {
        if (m_read_byte(*p, false))
          return count;
      }

      len--;
      count++;
      p++;
    }

    m_stop();
    return count;
  }

  uint32_t memory_write(const void* data, uint32_t len, uint8_t device_address, uint32_t reg_address)
  {
    const uint8_t* p     = (const uint8_t*)data;
    uint32_t       count = 0;

    m_start();

    if (m_address_type == 0)
      m_write_byte(device_address + ((reg_address >> 8) << 1));
    else if (m_address_type == 1)
      m_write_byte(device_address | 0);
    else if (m_address_type == 2)
    {
      m_write_byte(device_address | 0);
      if (m_wait_ack())
        return count;
      m_write_byte((uint8_t)(reg_address >> 8));
    }

    if (m_wait_ack())
      return count;
    m_write_byte((uint8_t)reg_address);

    if (m_wait_ack())
      return count;

    while (len)
    {
      m_write_byte(*p);
      if (m_wait_ack())
        return count;

      len--;
      count++;
      p++;
    }

    m_stop();
    return count;
  }

  uint32_t memory_write(const void* data, uint32_t len, uint8_t device_address, uint32_t reg_address, uint32_t delay_time)
  {
    const uint8_t* p     = (const uint8_t*)data;
    uint32_t       count = 0;

    while (len)
    {
      m_start();

      if (m_address_type == 0)
        m_write_byte(device_address + ((reg_address >> 8) << 1));
      else if (m_address_type == 1)
        m_write_byte(device_address | 0);
      else if (m_address_type == 2)
      {
        m_write_byte(device_address | 0);
        if (m_wait_ack())
          return count;
        m_write_byte((uint8_t)(reg_address >> 8));
      }

      if (m_wait_ack())
        return count;
      m_write_byte((uint8_t)reg_address);

      if (m_wait_ack())
        return count;

      m_write_byte(*p);
      if (m_wait_ack())
        return count;

      m_stop();
      byte_delay(delay_time);

      len--;
      reg_address++;
      count++;
      p++;
    }

    return count;
  }

  uint32_t master_send(const void* data, uint32_t len, uint8_t device_address)
  {
    const uint8_t* p     = (const uint8_t*)data;
    uint32_t       count = 0;

    m_start();
    m_write_byte(device_address | 0);

    if (m_wait_ack())
      return count;

    while (len)
    {
      m_write_byte(*p);
      if (m_wait_ack())
        return count;

      len--;
      count++;
      p++;
    }

    m_stop();
    return count;
  }

  uint32_t master_send(const void* data, uint32_t len, uint8_t device_address, uint32_t delay_time)
  {
    const uint8_t* p     = (const uint8_t*)data;
    uint32_t       count = 0;

    while (len)
    {
      m_start();
      m_write_byte(device_address | 0);

      if (m_wait_ack())
        return count;

      m_write_byte(*p);
      if (m_wait_ack())
        return count;

      m_stop();
      byte_delay(delay_time);

      len--;
      count++;
      p++;
    }

    return count;
  }

  uint32_t master_recv(void* data, uint32_t len, uint8_t device_address)
  {
    uint8_t* p     = (uint8_t*)data;
    uint32_t count = 0;

    m_start();
    m_write_byte(device_address | 1);

    if (m_wait_ack())
      return count;

    while (len)
    {
      if (len > 1)
      {
        if (m_read_byte(*p, true))
          return count;
      }
      else
      {
        if (m_read_byte(*p, false))
          return count;
      }

      len--;
      count++;
      p++;
    }

    m_stop();
    return count;
  }

  explicit VSoft_IIC(const std::string& name = "VSoft_IIC", Object* parent = nullptr) : Object(name, parent)
  {
    m_sda = new Class::io(name + "_sda", this);
    m_scl = new Class::io(name + "_scl", this);
  }

  virtual bool open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin, uint8_t addr_type, uint8_t clk_time = 1)
  {
    bool ret        = true;
    m_clk_time      = clk_time;
    m_address_type  = addr_type;
    ret            &= m_scl->open(scl_port, scl_pin, Gpio::OUT_OD);
    ret            &= m_sda->open(sda_port, sda_pin, Gpio::OUT_OD);
    return ret;
  }

  void set_clk_time(uint8_t clk_time)
  {
    m_clk_time = clk_time;
  }

  void set_address_type(uint8_t addr_type)
  {
    m_address_type = addr_type;
  }

  virtual bool close()
  {
    bool ret  = true;
    ret      &= m_sda->close();
    ret      &= m_scl->close();
    return ret;
  }

  virtual ~VSoft_IIC()
  {
    delete m_sda;
    delete m_scl;
  }

  friend class VIIC;
};

/// @brief 虚拟IIC类
class VIIC : public OwO::system::IOPort
{
  O_MEMORY
  O_OBJECT
  NO_COPY(VIIC)
  NO_MOVE(VIIC)
private:
  enum class IIC_Type
  {
    IIC_Soft,
  };

  uint32_t   m_delay_time;
  IIC_Type   m_iic_type;
  VSoft_IIC* m_soft_iic;

  virtual uint32_t m_read(void* data, uint32_t len, uint8_t device_address, uint32_t reg_address) override
  {
    if (IIC_Type::IIC_Soft == m_iic_type)
      return m_soft_iic->memory_read(data, len, device_address, reg_address);
    else
      return 0;
  }

  virtual uint32_t m_write(const void* data, uint32_t len, uint8_t device_address, uint32_t reg_address) override
  {
    if (IIC_Type::IIC_Soft == m_iic_type)
    {
      if (m_delay_time)
        return m_soft_iic->memory_write(data, len, device_address, reg_address, m_delay_time);
      else
        return m_soft_iic->memory_write(data, len, device_address, reg_address);
    }
    else
      return 0;
  }

  virtual uint32_t m_recv(void* data, uint32_t len, uint8_t device_address) override
  {
    if (IIC_Type::IIC_Soft == m_iic_type)
      return m_soft_iic->master_recv(data, len, device_address);
    else
      return 0;
  }

  virtual uint32_t m_send(const void* data, uint32_t len, uint8_t device_address) override
  {
    if (IIC_Type::IIC_Soft == m_iic_type)
    {
      if (m_delay_time)
        return m_soft_iic->master_send(data, len, device_address, m_delay_time);
      else
        return m_soft_iic->master_send(data, len, device_address);
    }
    else
      return 0;
  }

  virtual uint32_t m_erase(uint32_t len, uint8_t device_address, uint32_t reg_address, const char erase_bit) override
  {
    uint32_t i = 0;
    for (; i < len; i++)
    {
      if (1 != m_write(&erase_bit, 1, device_address, reg_address + i))
        return i;
    }
    return i;
  }

protected:
  void set_delay_time(uint32_t delay_time)
  {
    m_delay_time = delay_time;
  }

  using OwO::system::IOPort::open;

public:
  explicit VIIC(const std::string& name = "VIIC", Object* parent = nullptr) : OwO::system::IOPort(name, parent)
  {
    m_delay_time = 0;
    m_iic_type   = IIC_Type::IIC_Soft;
    m_soft_iic   = nullptr;
  }

  virtual bool open(uint8_t address, Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin, uint8_t addr_type = 0, uint8_t clk_time = 1, uint32_t delay_time = 0)
  {
    if (false == IOPort::open(address))
      return false;

    m_delay_time = delay_time;
    m_iic_type   = IIC_Type::IIC_Soft;
    m_soft_iic   = new VSoft_IIC(name() + "_soft_iic", this);

    if (false == m_soft_iic->open(scl_port, scl_pin, sda_port, sda_pin, addr_type, clk_time))
    {
      delete m_soft_iic;
      return false;
    }
    return true;
  }

  void set_clk_time(uint8_t clk_time)
  {
    if (nullptr != m_soft_iic)
      m_soft_iic->set_clk_time(clk_time);
  }

  void set_address_type(uint8_t addr_type)
  {
    if (nullptr != m_soft_iic)
      m_soft_iic->set_address_type(addr_type);
  }

  virtual bool close()
  {
    bool ret  = true;
    ret      &= IOPort::close();

    if (nullptr != m_soft_iic)
    {
      delete m_soft_iic;
      m_soft_iic = nullptr;
    }

    return ret;
  }

  virtual ~VIIC()
  {
    close();
  }
};
} /* namespace virtual_class */
} /* namespace OwO */

#endif /* __VIRTUAL_IIC_HPP__ */
