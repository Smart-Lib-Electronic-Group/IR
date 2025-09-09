/**
 * @file      eprom.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for EPROM device
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __EPROM_HPP__
#define __EPROM_HPP__

#include "virtual_iic.hpp"
#include "orom.hpp"

/// @brief EPROM 默认通讯时钟周期(通讯延时)
#define EPROM_DEF_DELAY_TIME 0x01
/// @brief EPROM 默认芯片地址
#define EPROM_DEF_ADDRESS    0xA0
/// @brief EPROM 默认校验字节内容
#define EPROM_DEF_CHECK_FLAG 0x79
/// @brief EPROM 默认清零字节内容
#define EPROM_DEF_ERASE_BIT  0xFF

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 类 EPROM
class EPROM : public system::ORom
{
  O_MEMORY
  O_OBJECT
  NO_COPY(EPROM)
  NO_MOVE(EPROM)
public:
  /// @brief 枚举 EPROM 芯片类型
  typedef enum
  {
    AT24C01 = 0x01,   // AT24c01芯片
    AT24C02 = 0x02,   // AT24c02芯片
    AT24C04 = 0x04,   // AT24c04芯片
    AT24C08 = 0x08,   // AT24c08芯片
    AT24C16 = 0x10,   // AT24c16芯片
    AT24C32 = 0x20,   // AT24c32芯片
    AT24C64 = 0x40    // AT24c64芯片
  } eprom_type_e;

private:
  /// @brief EPROM IIC 接口
  virtual_class::VIIC* m_iic;
  /// @brief EPROM 芯片类型
  eprom_type_e         m_type;

public:
  /**
   * @brief EPROM 构造函数
   *
   * @param name    名称
   * @param parent  父对象指针
   */
  EPROM(const std::string& name = "EPROM", Object* parent = nullptr) : ORom(name, parent)
  {
    m_iic  = new virtual_class::VIIC(name + "_iic", this);
    m_type = AT24C01;
  }

  /**
   * @brief EPROM 开启设备
   *
   * @param  scl_port IIC SCL 端口
   * @param  scl_pin  IIC SCL 引脚
   * @param  sda_port IIC SDA 端口
   * @param  sda_pin  IIC SDA 引脚
   * @param  type     EPROM 芯片类型
   * @param  address  EPROM 芯片地址
   * @return true     成功
   * @return false    失败
   */
  virtual bool open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin, eprom_type_e type, uint8_t address = EPROM_DEF_ADDRESS)
  {
    m_type    = type;
    bool ret  = true;
    ret      &= m_iic->open(address, scl_port, scl_pin, sda_port, sda_pin, (type > AT24C16) ? 2 : 0, EPROM_DEF_DELAY_TIME, 10);
    ret      &= ORom::open(m_iic);
    return ret;
  }

  /**
   * @brief  EPROM 关闭设备
   *
   * @return true  成功
   * @return false 失败
   */
  virtual bool close()
  {
    bool ret  = true;
    ret      &= m_iic->close();
    ret      &= ORom::close();
    return ret;
  }

  /**
   * @brief  EPROM 检查设备是否为第一次写入
   *
   * @param  auto_reload 自动擦除
   * @return true        第一次写入
   * @return false       非第一次写入
   */
  bool check(bool auto_reload = true);

  /**
   * @brief  EPROM 析构函数
   *
   */
  virtual ~EPROM() {}
};
} /* namespace device */
} /* namespace OwO */

#endif /* __EPROM_HPP__ */
