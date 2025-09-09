/**
 * @file      delixi_meter.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for Delixi Meter Device (德力西 电参数表)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __DELIXI_METER_HPP__
#define __DELIXI_METER_HPP__

/// @brief 德力西 电参数表 默认地址
#define DELIXI_METER_DEF_ADDRESS 0x01

#include "rs485.hpp"
#include "thread.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 类 德力西 电参数表
class Delixi_Meter : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Delixi_Meter)
  NO_MOVE(Delixi_Meter)

public:
  /// @brief 德力西 电参数表 电参数信息结构体
  typedef struct DELIXI_METER_DATA_T
  {
    uint16_t dpt_dct;            /* 德力西 电参数表 电压电流小数点位 */
    uint16_t dpq;                /* 德力西 电参数表 功率小数点位 */
    uint16_t a_voltage_vlue;     /* 德力西 电参数表 A相电压值 */
    uint16_t b_voltage_vlue;     /* 德力西 电参数表 B相电压值 */
    uint16_t c_voltage_vlue;     /* 德力西 电参数表 C相电压值 */
    uint16_t ab_voltage_vlue;    /* 德力西 电参数表 AB相电压值 */
    uint16_t bc_voltage_vlue;    /* 德力西 电参数表 BC相电压值 */
    uint16_t ca_voltage_vlue;    /* 德力西 电参数表 CA相电压值 */
    uint16_t a_current_vlue;     /* 德力西 电参数表 A相电流值 */
    uint16_t b_current_vlue;     /* 德力西 电参数表 B相电流值 */
    uint16_t c_current_vlue;     /* 德力西 电参数表 C相电流值 */
    uint16_t a_active_power;     /* 德力西 电参数表 A相有功功率 */
    uint16_t b_active_power;     /* 德力西 电参数表 B相有功功率 */
    uint16_t c_active_power;     /* 德力西 电参数表 C相有功功率 */
    uint16_t all_active_power;   /* 德力西 电参数表 三相有功功率 */
    uint16_t a_reactive_power;   /* 德力西 电参数表 A相无功功率 */
    uint16_t b_reactive_power;   /* 德力西 电参数表 B相无功功率 */
    uint16_t c_reactive_power;   /* 德力西 电参数表 C相无功功率 */
    uint16_t all_reactive_power; /* 德力西 电参数表 三相无功功率 */
    uint16_t a_power_factor;     /* 德力西 电参数表 A相功率因数 */
    uint16_t b_power_factor;     /* 德力西 电参数表 B相功率因数 */
    uint16_t c_power_factor;     /* 德力西 电参数表 C相功率因数 */
    uint16_t all_power_factor;   /* 德力西 电参数表 三相功率因数 */
    uint16_t a_apparent_power;   /* 德力西 电参数表 A相视在功率 */
    uint16_t b_apparent_power;   /* 德力西 电参数表 B相视在功率 */
    uint16_t c_apparent_power;   /* 德力西 电参数表 C相视在功率 */
    uint16_t all_apparent_power; /* 德力西 电参数表 三相视在功率 */
    uint16_t frequency;          /* 德力西 电参数表 频率值 */
  } meter_data_t;

private:
  /// @brief 德力西 电参数表 互斥锁
  mutable system::kernel::Mutex m_mutex;
  /// @brief 德力西 电参数表 串口
  driver::RS485*                m_port;
  /// @brief 德力西 电参数表 数据
  meter_data_t                  m_data;
  /// @brief 德力西 电参数表 地址
  uint8_t                       m_address;
  /// @brief 德力西 电参数表 发送缓冲区
  uint8_t                       m_send_buf[8];
  /// @brief 德力西 电参数表 接收缓冲区
  uint8_t                       m_recv_buf[64];

  /**
   * @brief 德力西 电参数表 获取命令
   *
   */
  void get_cmd();
  /**
   * @brief 德力西 电参数表 解析数据
   *
   * @param  timeout  超时时间
   * @return true     解析成功
   * @return false    解析失败
   */
  bool analyze_data(uint32_t timeout);
  /**
   * @brief 德力西 电参数表 发送命令
   *
   * @return true   发送成功
   * @return false  发送失败
   */
  bool send_cmd()
  {
    return (8 == m_port->send(m_send_buf, 8));
  }

public:
  /**
   * @brief  德力西 电参数表 解析数据
   *
   * @param  timeout  超时时间
   * @return true     解析成功
   * @return false    解析失败
   */
  bool analyze(uint32_t timeout)
  {
    if (!is_open())
    {
      system::kernel::Thread::msleep(timeout);
      return false;
    }

    send_cmd();
    return analyze_data(timeout);
  }

  /**
   * @brief 德力西 电参数表 构造函数
   *
   * @param name    名称
   * @param parent  父对象指针
   */
  explicit Delixi_Meter(const std::string& name = "Delixi_Meter", Object* parent = nullptr);
  /**
   * @brief 德力西 电参数表 初始化
   *
   * @param  port       端口号
   * @param  address    电表地址
   * @return true       成功
   * @return false      失败
   */
  virtual bool open(const uint8_t& port, uint8_t address = DELIXI_METER_DEF_ADDRESS);
  /**
   * @brief 德力西 电参数表 初始化
   *
   * @param  port       端口号
   * @param  de_port    DE端口号
   * @param  de_pin     DE引脚号
   * @param  address    电表地址
   * @return true       成功
   * @return false      失败
   */
  virtual bool open(const uint8_t& port, Gpio::Port de_port, uint8_t de_pin, uint8_t address = DELIXI_METER_DEF_ADDRESS);
  /**
   * @brief 德力西 电参数表 初始化
   *
   * @param port        端口号
   * @param de_port     DE端口号
   * @param de_pin      DE引脚号
   * @param re_port     RE端口号
   * @param re_pin      RE引脚号
   * @param address     电表地址
   * @return true       成功
   * @return false      失败
   */
  virtual bool open(const uint8_t& port, Gpio::Port de_port, uint8_t de_pin, Gpio::Port re_port, uint8_t re_pin, uint8_t address = DELIXI_METER_DEF_ADDRESS);

  /**
   * @brief 德力西 电参数表 关闭
   *
   * @return true   成功
   * @return false  失败
   */
  virtual bool close();

  /**
   * @brief 德力西 电参数表 获取数据
   *
   * @return const meter_data_t& 数据引用
   */
  const meter_data_t& get_data() const
  {
    return m_data;
  }

  /**
   * @brief 德力西 电参数表 获取互斥锁
   * 
   */
  system::kernel::Mutex& mutex() const
  {
    return m_mutex;
  }

  /**
   * @brief 德力西 电参数表 获取数据
   *
   * @param  p              数据指针
   * @return meter_data_t*  数据指针
   */
  meter_data_t* get_data(meter_data_t* p) const
  {
    system::kernel::Mutex_Guard lock(m_mutex);
    std::memcpy(p, &m_data, sizeof(meter_data_t));
    return p;
  }

  /**
   * @brief 德力西 电参数表 析构函数
   *
   */
  virtual ~Delixi_Meter()
  {
    delete m_port;
  }
};
} /* namespace device */
} /* namespace OwO */

#endif /* __DELIXI_METER_HPP__ */
