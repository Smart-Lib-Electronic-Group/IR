#ifndef __W25Q256_HPP__
#define __W25Q256_HPP__

#include "thread.hpp"
#include "ioport.hpp"
#include "virtual_spi.hpp"

/// @brief 名称空间 库名
namespace OwO
{
namespace driver
{
class W25Q256 : public system::IOPort
{
  O_MEMORY
  O_OBJECT
  NO_COPY(W25Q256)
  NO_MOVE(W25Q256)

  static constexpr inline uint32_t PAGE_SIZE                  = 256;
  static constexpr inline uint32_t SECTOR_PAGE_COUNT          = 16;
  static constexpr inline uint32_t BLOCK_SECTOR_COUNT         = 16;

  static constexpr inline uint32_t SECTOR_SIZE                = SECTOR_PAGE_COUNT * PAGE_SIZE;
  static constexpr inline uint32_t BLOCK_SIZE                 = BLOCK_SECTOR_COUNT * SECTOR_SIZE;

  static constexpr inline uint32_t BLOCK_COUNT                = 512;
  static constexpr inline uint32_t SECTOR_COUNT               = BLOCK_COUNT * BLOCK_SECTOR_COUNT;
  static constexpr inline uint32_t PAGE_COUNT                 = SECTOR_COUNT * SECTOR_PAGE_COUNT;
  static constexpr inline uint32_t CHIP_SIZE                  = SECTOR_COUNT * SECTOR_SIZE;

  static constexpr inline uint8_t CMD_ENABLE_WRITE_STATUS_REG = 0x50;
  static constexpr inline uint8_t CMD_ENABLE_WRITE            = 0x06;
  static constexpr inline uint8_t CMD_DISABLE_WRITE           = 0x04;

  static constexpr inline uint8_t CMD_WRITE_STATUS            = 0x01;
  static constexpr inline uint8_t CMD_READ_STATUS             = 0x05;

  static constexpr inline uint8_t CMD_READ                    = 0x13;
  static constexpr inline uint8_t CMD_FAST_READ               = 0x0C;
  static constexpr inline uint8_t CMD_PAGE_PROGRAM            = 0x12;

  static constexpr inline uint8_t CMD_SECTOR_ERASE            = 0x21;
  static constexpr inline uint8_t CMD_BLOCK_32_ERASE          = 0x52;
  static constexpr inline uint8_t CMD_BLOCK_64_ERASE          = 0xD8;
  static constexpr inline uint8_t CMD_CHIP_ERASE              = 0xC7;

  static constexpr inline uint8_t CMD_READ_ID                 = 0x9F;
  static constexpr inline uint8_t CMD_ENTER_4B_ADDR_MODE      = 0xB7;
  static constexpr inline uint8_t CMD_EXIT_4B_ADDR_MODE       = 0xE9;

private:
  uint8_t              m_buffer[SECTOR_SIZE];
  virtual_class::VSpi* m_spi;

protected:
  void send_command(uint8_t cmd)
  {
    m_spi->send(&cmd, 1);
    m_spi->wait_send_complete();
  }

  void send_command_and_address(uint8_t cmd, uint32_t addr)
  {
    uint8_t data[] = { cmd, static_cast<uint8_t>((addr & 0xFF000000) >> 24), static_cast<uint8_t>((addr & 0xFF0000) >> 16), static_cast<uint8_t>((addr & 0xFF00) >> 8), static_cast<uint8_t>(addr & 0xFF) };
    m_spi->send(data, sizeof(data));
    m_spi->wait_send_complete();
  }

  void send_data(const uint8_t* data, uint32_t size, uint32_t addr)
  {
    send_command_and_address(CMD_PAGE_PROGRAM, addr);
    m_spi->send(data, size);
    m_spi->wait_send_complete();
  }

  uint8_t read_status()
  {
    uint8_t data[2] = { 0 };
    m_spi->spi_start();
    send_command(CMD_READ_STATUS);
    m_spi->spi_stop();

    m_spi->spi_start();
    m_spi->receive(data, 2);
    m_spi->wait_receive_complete();
    m_spi->spi_stop();

    return data[1];
  }

  bool is_need_erase(const uint8_t* old_data, const uint8_t* new_data, uint32_t size)
  {
    uint8_t old;

    for (uint32_t i = 0; i < size; i++)
    {
      old = *old_data++;
      old = ~old;

      if ((old & (*new_data++)) != 0)
        return true;
    }

    return false;
  }

  bool compare_data(const uint8_t* cmp_data, uint32_t size, uint8_t src_addr)
  {
    if (src_addr > CHIP_SIZE || (src_addr + size) > CHIP_SIZE)
      return false;

    if (size == 0)
      return true;

    m_spi->spi_start();
    send_command_and_address(CMD_READ, src_addr);

    for (uint32_t i = 0; i < size / SECTOR_SIZE; i++)
    {
      m_spi->receive(m_buffer, SECTOR_SIZE);
      m_spi->wait_receive_complete();

      for (uint32_t j = 0; j < SECTOR_SIZE; j++)
      {
        if (m_buffer[j] != *cmp_data++)
        {
          m_spi->spi_stop();
          return false;
        }
      }
    }

    if (size % SECTOR_SIZE)
    {
      m_spi->receive(m_buffer, size % SECTOR_SIZE);
      m_spi->wait_receive_complete();

      for (uint32_t j = 0; j < size % SECTOR_SIZE; j++)
      {
        if (m_buffer[j] != *cmp_data++)
        {
          m_spi->spi_stop();
          return false;
        }
      }
    }

    m_spi->spi_stop();
    return true;
  }

  void write_enable()
  {
    m_spi->spi_start();
    send_command(CMD_ENABLE_WRITE);
    m_spi->spi_stop();
  }

  void erase_sector(uint32_t sector_addr)
  {
    write_enable();

    m_spi->spi_start();
    send_command_and_address(CMD_SECTOR_ERASE, sector_addr);
    m_spi->spi_stop();

    wait_write_complete();
  }

  void write_page(uint32_t start_addr, const uint8_t* data, uint32_t size)
  {
    for (uint32_t i = 0; i < size / PAGE_SIZE; i++)
    {
      write_enable();

      m_spi->spi_start();
      send_data(data, PAGE_SIZE, start_addr);
      m_spi->spi_stop();

      wait_write_complete();
      data       += PAGE_SIZE;
      start_addr += PAGE_SIZE;
    }

    if (size % PAGE_SIZE)
    {
      write_enable();

      m_spi->spi_start();
      send_data(data, size % PAGE_SIZE, start_addr);
      m_spi->spi_stop();

      wait_write_complete();
      data += PAGE_SIZE;
    }

    m_spi->spi_start();
    send_command(CMD_DISABLE_WRITE);
    m_spi->spi_stop();

    wait_write_complete();
  }

  bool auto_write_sector(uint32_t write_addr, const uint8_t* data, uint32_t size)
  {
    if (size == 0)
      return true;

    if (size > SECTOR_SIZE || write_addr >= CHIP_SIZE || (write_addr + size) > CHIP_SIZE)
      return false;

    uint32_t sector_start_addr = write_addr & ~(SECTOR_SIZE - 1);
    bool     need_erase        = false;

    m_read(m_buffer, size, 0, write_addr);
    if (0 == std::memcmp(m_buffer, data, size))
    {
      return true;
    }

    need_erase = is_need_erase(m_buffer, data, size);

    for (uint8_t i = 0; i < 3; i++)
    {
      if (need_erase)
      {
        m_read(m_buffer, SECTOR_SIZE, 0, sector_start_addr);
        std::memcpy(m_buffer + write_addr - sector_start_addr, data, size);

        erase_sector(sector_start_addr);
        write_page(sector_start_addr, m_buffer, SECTOR_SIZE);
      }
      else
      {
        write_page(write_addr, data, size);
      }

      if (compare_data(data, size, write_addr))
        return true;
      else
      {
        if (compare_data(data, size, write_addr))
          return true;

        system::kernel::Thread::msleep(100);
      }
    }

    return false;
  }

  virtual uint32_t m_read(void* data, uint32_t size, uint8_t device_address, uint32_t start_addr) override
  {
    (void)device_address;

    if (size == 0 || start_addr >= CHIP_SIZE || (start_addr + size) > CHIP_SIZE)
      return 0;

    uint8_t* p = static_cast<uint8_t*>(data);
    m_spi->spi_start();
    send_command_and_address(CMD_READ, start_addr);

    for (uint32_t i = 0; i < size / SECTOR_SIZE; i++)
    {
      m_spi->receive(p, SECTOR_SIZE);
      m_spi->wait_receive_complete();
      p += SECTOR_SIZE;
    }

    if ((size % SECTOR_SIZE) > 0)
    {
      m_spi->receive(p, size % SECTOR_SIZE);
      m_spi->wait_receive_complete();
    }

    m_spi->spi_stop();
    return size;
  }

  virtual uint32_t m_write(const void* data, uint32_t size, uint8_t device_address, uint32_t start_addr) override
  {
    (void)device_address;

    if (size == 0 || start_addr >= CHIP_SIZE || (start_addr + size) > CHIP_SIZE)
      return 0;

    const uint8_t* p                       = static_cast<const uint8_t*>(data);
    uint32_t       first_sector_start_addr = start_addr % SECTOR_SIZE;
    uint32_t       first_sector_save_size  = first_sector_start_addr ? SECTOR_SIZE - first_sector_start_addr : 0;
    uint32_t       full_erase_sector_count = (size > first_sector_save_size) ? (size - first_sector_save_size) / SECTOR_SIZE : 0;
    uint32_t       last_sector_erase_size  = (size > first_sector_save_size) ? (size - first_sector_save_size) % SECTOR_SIZE : 0;
    uint32_t       ret                     = 0;

    if (0 == first_sector_start_addr)
    {
      if (0 == full_erase_sector_count)
      {
        ret = auto_write_sector(start_addr, p, size) ? size : 0;
        return ret;
      }
      else
      {
        while (full_erase_sector_count--)
        {
          ret        += auto_write_sector(start_addr, p, SECTOR_SIZE) ? SECTOR_SIZE : 0;
          p          += SECTOR_SIZE;
          start_addr += SECTOR_SIZE;
        }

        if (0 != last_sector_erase_size)
          ret += auto_write_sector(start_addr, p, last_sector_erase_size) ? last_sector_erase_size : 0;
        return ret;
      }
    }
    else
    {
      if (size > first_sector_save_size)
        ret += auto_write_sector(start_addr, p, first_sector_save_size) ? first_sector_save_size : 0;
      else
        ret += auto_write_sector(start_addr, p, size) ? size : 0;

      p          += first_sector_save_size;
      start_addr += first_sector_save_size;

      while (full_erase_sector_count--)
      {
        ret        += auto_write_sector(start_addr, p, SECTOR_SIZE) ? SECTOR_SIZE : 0;
        p          += SECTOR_SIZE;
        start_addr += SECTOR_SIZE;
      }

      if (0 != last_sector_erase_size)
        ret += auto_write_sector(start_addr, p, last_sector_erase_size) ? last_sector_erase_size : 0;
    }

    return ret;
  }

  virtual uint32_t m_erase(uint32_t size, uint8_t device_address, uint32_t start_addr, const char erase_bit) override
  {
    (void)device_address;
    (void)erase_bit;

    if (size == 0 || start_addr >= CHIP_SIZE || (start_addr + size) > CHIP_SIZE)
      return 0;

    uint32_t first_sector_start_addr = start_addr % SECTOR_SIZE;
    uint32_t first_sector_save_size  = first_sector_start_addr ? SECTOR_SIZE - first_sector_start_addr : 0;
    uint32_t full_erase_sector_count = (size > first_sector_save_size) ? (size - first_sector_save_size) / SECTOR_SIZE : 0;
    uint32_t last_sector_erase_size  = (size > first_sector_save_size) ? (size - first_sector_save_size) % SECTOR_SIZE : 0;
    uint32_t ret                     = 0;

    if (0 == first_sector_start_addr)
    {
      if (0 == full_erase_sector_count)
      {
        m_read(m_buffer, SECTOR_SIZE, 0, start_addr);
        std::memset(m_buffer, 0xFF, size);
        erase_sector(start_addr);
        write_page(start_addr, m_buffer, SECTOR_SIZE);
        ret += first_sector_save_size;
      }
      else
      {
        while (full_erase_sector_count--)
        {
          erase_sector(start_addr);
          start_addr += SECTOR_SIZE;
          ret        += SECTOR_SIZE;
        }

        if (0 != last_sector_erase_size)
        {
          m_read(m_buffer, SECTOR_SIZE, 0, start_addr);
          std::memset(m_buffer, 0xFF, last_sector_erase_size);
          erase_sector(start_addr);
          write_page(start_addr, m_buffer, SECTOR_SIZE);
          ret += last_sector_erase_size;
        }
      }
    }
    else
    {
      m_read(m_buffer, SECTOR_SIZE, 0, start_addr - first_sector_start_addr);

      if (size < SECTOR_SIZE)
        std::memset(m_buffer + first_sector_start_addr, 0xFF, size);
      else
        std::memset(m_buffer + first_sector_save_size, 0xFF, SECTOR_SIZE - first_sector_save_size);

      erase_sector(start_addr - first_sector_start_addr);
      write_page(start_addr - first_sector_start_addr, m_buffer, SECTOR_SIZE);
      start_addr += first_sector_save_size;
      ret        += first_sector_save_size;

      while (full_erase_sector_count--)
      {
        erase_sector(start_addr);
        start_addr += SECTOR_SIZE;
        ret        += SECTOR_SIZE;
      }

      if (0 != last_sector_erase_size)
      {
        m_read(m_buffer, SECTOR_SIZE, 0, start_addr);
        std::memset(m_buffer, 0xFF, last_sector_erase_size);
        erase_sector(start_addr);
        write_page(start_addr, m_buffer, SECTOR_SIZE);
        ret += last_sector_erase_size;
      }
    }

    return ret;
  }

  virtual uint32_t m_recv(void* data, uint32_t len, uint8_t device_address)
  {
    return 0;
  }

  virtual uint32_t m_send(const void* data, uint32_t len, uint8_t device_address)
  {
    return 0;
  }

  using IOPort::recv;
  using IOPort::send;
  using IOPort::open;

public:
  explicit W25Q256(const std::string& name = "IOPort", Object* parent = nullptr) : system::IOPort(name, parent)
  {
    m_spi = new virtual_class::VSpi(name + "_spi", this);
  }

  bool open(uint8_t port, Gpio::Port cs_port, uint8_t cs_pin, Spi::WorkMode rx_mode = Spi::DMA, Spi::WorkMode tx_mode = Spi::DMA)
  {
    if (!IOPort::open(0x00))
      return false;

    bool ret = m_spi->open(port, cs_port, cs_pin, rx_mode, tx_mode);
    send_command(CMD_ENTER_4B_ADDR_MODE);
    return ret;
  }

  uint32_t get_id()
  {
    uint8_t data[3];
    m_spi->spi_start();
    send_command(CMD_READ_ID);
    m_spi->receive(data, 3);
    m_spi->wait_receive_complete();
    m_spi->spi_stop();

    return (data[0] << 16) | (data[1] << 8) | data[2];
  }

  void wait_write_complete()
  {
    uint8_t data[2] = { 0 };

    while (1)
    {
      m_spi->spi_start();
      send_command(CMD_READ_STATUS);
      m_spi->receive(data, 2);
      m_spi->wait_receive_complete();
      m_spi->spi_stop();

      if ((data[1] & 0x01) == 0)
      {
        break;
      }

      system::kernel::Thread::msleep(1);
    }
  }

  void erase_chip()
  {
    write_enable();

    m_spi->spi_start();
    send_command(CMD_CHIP_ERASE);
    m_spi->spi_stop();

    wait_write_complete();
  }

  uint32_t get_size()
  {
    return CHIP_SIZE;
  }

  virtual bool close()
  {
    if (!IOPort::close())
      return true;

    return m_spi->close();
  }

  virtual ~W25Q256() {}
};
} /* namespace driver */
} /* namespace OwO */

#endif /* __W25Q256_HPP__ */
