/**
 * @file      nor_flash.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for NOR Flash device (W25Q256)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __NOR_FLASH_HPP__
#define __NOR_FLASH_HPP__

#include "w25q256.hpp"
#include "orom.hpp"

/// @brief NOR FLASH 默认校验字节内容
#define NOR_FLASH_DEF_CHECK_FLAG 0x97

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 类 NOR Flash
class Nor_Flash : public system::ORom
{
  O_OBJECT
  O_MEMORY
  NO_COPY(Nor_Flash)
  NO_MOVE(Nor_Flash)
private:
  /// @brief W25Q256 对象
  driver::W25Q256* m_flash;

public:
  /**
   * @brief NOR Flash 构造函数
   *
   * @param name    名称
   * @param parent  父对象指针
   */
  Nor_Flash(const std::string& name = "NOR Flash", Object* parent = nullptr) : system::ORom(name, parent)
  {
    m_flash = new driver::W25Q256("W25Q256", this);
  }

  /**
   * @brief NOR Flash 打开设备
   *
   * @param  port      端口号
   * @param  cs_port   片选端口号
   * @param  cs_pin    片选引脚号
   * @return true      成功
   * @return false     失败
   */
  virtual bool open(uint8_t port, Gpio::Port cs_port, uint8_t cs_pin)
  {
    bool ret  = true;
    ret      &= m_flash->open(port, cs_port, cs_pin, Spi::DMA, Spi::DMA);
    ret      &= system::ORom::open(m_flash);
    return ret;
  }

  /**
   * @brief NOR Flash 擦除设备
   *
   */
  void erase_chip()
  {
    m_flash->erase_chip();
  }

  /**
   * @brief NOR Flash 等待设备准备就绪
   *
   */
  void wait_ready()
  {
    m_flash->wait_write_complete();
  }

  /**
   * @brief  NOR Flash 关闭设备
   *
   * @return true 成功
   * @return false 失败
   */
  virtual bool close() override
  {
    bool ret  = true;
    ret      &= m_flash->close();
    ret      &= system::ORom::close();
    return ret;
  }

  /**
   * @brief NOR Flash 析构函数
   *
   */
  virtual ~Nor_Flash()
  {
    delete m_flash;
  }
};
} /* namespace device */
} /* namespace OwO */

#endif /* __NOR_FLASH_HPP__ */
