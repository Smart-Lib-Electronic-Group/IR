/**
 * @file      ir.cpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for IR device (红外遥控)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "ir.hpp"
#include "thread.hpp"
#include <cstring>

using namespace OwO;
using namespace device;
using namespace system;
using namespace kernel;

O_METAOBJECT(IR, Object)

/**
 * @brief (私有函数) IR 输出接口
 *
 * @param high_time 高电平周期
 * @param low_time  低电平周期
 */
void IR::m_ir_flash(uint32_t high_time, uint32_t low_time)
{
  high_time = high_time / m_pulse_width;
  low_time  = low_time / m_pulse_width;

  while (high_time--)
  {
    m_gpio->high();
    Thread::usleep(8);

    m_gpio->low();
    Thread::usleep(17);
  }
  while (low_time--)
  {
    m_gpio->low();
    Thread::usleep(26);
  }
}

/**
 * @brief (私有函数) IR 奥克斯红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_aux(uint8_t len)
{
  // 长度判断
  if (13 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_aux_start();
  for (i = 0; i < 13; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_aux_data(i) & (0x01 << j))
        m_aux_1();
      else
        m_aux_0();
    }
  }
  // 结束码
  m_aux_stop();
  return true;
}

/**
 * @brief (私有函数) IR TCL红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_tcl(uint8_t len)
{
  // 长度判断
  if (28 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_tcl_start();
  for (i = 0; i < 14; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_tcl_data(1, i) & (0x01 << j))
        m_tcl_1();
      else
        m_tcl_0();
    }
  }
  m_tcl_stop();
  m_tcl_wait();
  m_tcl_start();
  for (i = 0; i < 14; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_tcl_data(2, i) & (0x01 << j))
        m_tcl_1();
      else
        m_tcl_0();
    }
  }
  // 结束码
  m_tcl_stop();
  return true;
}

/**
 * @brief (私有函数) IR 格力红外遥控 数据获取
 *
 * @param loaction  数据位置
 * @param num       发送段数
 * @param pos       位移量
 * @return char&
 */
char& IR::m_gree_data(uint8_t loaction, uint8_t num, uint8_t pos)
{
  if (1 == loaction)
    return m_data_tmp[(num - 1) * 10 + pos];
  else if (2 == loaction)
    return m_data_tmp[(num - 1) * 10 + 4];
  else if (3 == loaction)
    return m_data_tmp[(num - 1) * 10 + 5];
  else if (4 == loaction)
    return m_data_tmp[(num - 1) * 10 + pos + 6];
  else
    return m_data_tmp[(num - 1) * 10 + 9];
}

/**
 * @brief (私有函数) IR 格力红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_gree(uint8_t len)
{
  // 长度判断
  if (30 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;

  // 第一段
  // 头码
  m_gree_start();
  // 先发送11 12 13 14 寄存器数据，8位发送
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(1, 1, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 15 寄存器数据，3位发送
  for (j = 0; j < 3; j++)
  {
    if (m_gree_data(2, 1) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 发送连接码，等待20MS
  m_gree_wait(20);
  // 16 寄存器数据，1位发送
  if (m_gree_data(3, 1) & (0x01))
    m_gree_1();
  else
    m_gree_0();
  // 先发送17 18 19 寄存器数据，8位发送
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(4, 1, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 先发送28 寄存器数据，7位发送
  for (j = 0; j < 7; j++)
  {
    if (m_gree_data(5, 1) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 发送连接码，等待40MS
  m_gree_wait(40);
  // 第一段结束

  // 第二段
  // 头码
  m_gree_start();
  // 先发送11 12 13 14 寄存器数据，8位发送
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(1, 2, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 15 寄存器数据，3位发送
  for (j = 0; j < 3; j++)
  {
    if (m_gree_data(2, 2) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 发送连接码，等待20MS
  m_gree_wait(20);
  // 16 寄存器数据，1位发送
  if (m_gree_data(3, 2) & (0x01))
    m_gree_1();
  else
    m_gree_0();
  // 先发送17 18 19 寄存器数据，8位发送
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(4, 2, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 先发送28 寄存器数据，7位发送
  for (j = 0; j < 7; j++)
  {
    if (m_gree_data(5, 2) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 发送连接码，等待40MS
  m_gree_wait(40);
  // 第二段结束

  // 第三段
  // 头码
  m_gree_start();
  // 先发送21 22 23 24 寄存器数据，8位发送
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(1, 3, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 25 寄存器数据，3位发送
  for (j = 0; j < 3; j++)
  {
    if (m_gree_data(2, 3) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 发送连接码，等待20MS
  m_gree_wait(20);
  // 26 寄存器数据，1位发送
  if (m_gree_data(3, 3) & (0x01))
    m_gree_1();
  else
    m_gree_0();
  // 先发27 28 29 寄存器数据，8位发送
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_gree_data(4, 3, i) & (0x01 << j))
        m_gree_1();
      else
        m_gree_0();
    }
  }
  // 先发送28 寄存器数据，7位发送
  for (j = 0; j < 7; j++)
  {
    if (m_gree_data(5, 3) & (0x01 << j))
      m_gree_1();
    else
      m_gree_0();
  }
  // 结束码
  m_gree_stop();
  return true;
}

/**
 * @brief (私有函数) IR 中广欧斯特红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_outes(uint8_t len)
{
  // 长度判断
  if (15 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_outes_start();
  // 中广贴牌奥克斯为15个字节   2025.06.01修改
  for (i = 0; i < 15; i++)
  {
    for (j = 0; j < 8; j++)
    {
      if (m_outes_data(i) & (0x01 << j))
        m_outes_1();
      else
        m_outes_0();
    }
  }
  // 结束码
  m_outes_stop();
  return true;
}

/**
 * @brief (私有函数) IR 美的红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_midea(uint8_t len)
{
  // 长度判断
  if (6 != len && 12 != len && 18 != len && 24 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_midea_start();
  // 8位数据从高到低位发送
  for (i = 0; i < 6; i++)
  {
    // 先发送11 12 13 14 15 16寄存器数据，8位发送
    for (j = 0; j < 8; j++)
    {
      if (m_midea_data(1, i) & (0x80 >> j))
        m_midea_1();
      else
        m_midea_0();
    }
  }

  if (6 == len)
  {
    m_midea_stop();
    return true;
  }
  else
    m_midea_wait();

  // 头码
  m_midea_start();
  // 8位数据从高到低位发送
  for (i = 0; i < 6; i++)
  {
    // 先发送17 18 19 20 21 22 寄存器数据，8位发送
    for (j = 0; j < 8; j++)
    {
      if (m_midea_data(2, i) & (0x80 >> j))
        m_midea_1();
      else
        m_midea_0();
    }
  }

  if (12 == len)
  {
    m_midea_stop();
    return true;
  }
  else
    m_midea_wait();

  // 头码
  m_midea_start();
  // 8位数据从高到低位发送
  for (i = 0; i < 6; i++)
  {
    // 先发送23 24 25 26 27 28 寄存器数据，8位发送
    for (j = 0; j < 8; j++)
    {
      if (m_midea_data(3, i) & (0x80 >> j))
        m_midea_1();
      else
        m_midea_0();
    }
  }

  if (18 == len)
  {
    m_midea_stop();
    return true;
  }
  else
    m_midea_wait();

  // 头码
  m_midea_start();
  // 8位数据从高到低位发送
  for (i = 0; i < 6; i++)
  {
    // 先发送29 30 31 32 33 34 寄存器数据，8位发送
    for (j = 0; j < 8; j++)
    {
      if (m_midea_data(4, i) & (0x80 >> j))
        m_midea_1();
      else
        m_midea_0();
    }
  }

  // 结束码
  m_midea_stop();
  return true;
}

/**
 * @brief (私有函数) IR 小米红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_xiaomi(uint8_t len)
{
  // 长度判断
  if (12 != len && 19 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_xiaomi_start();
  if (12 == len)
  {
    // 发12个字节
    for (i = 0; i < 12; i++)
    {
      // 先发送11 12 13 14 寄存器数据，8位发送
      for (j = 4; j > 0; j--)
      {
        if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x03) == 0x03)
          m_xiaomi_11();
        else if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x01) == 0x01)
          m_xiaomi_01();
        else if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x02) == 0x02)
          m_xiaomi_10();
        else
          m_xiaomi_00();
      }
    }
  }
  else if (19 == len)
  {
    // 第一段码11个字节
    for (i = 0; i < 11; i++)
    {
      // 先发送11 12 13 14 寄存器数据，8位发送
      for (j = 4; j > 0; j--)
      {
        if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x03) == 0x03)
          m_xiaomi_11();
        else if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x01) == 0x01)
          m_xiaomi_01();
        else if (((m_xiaomi_data(1, i) >> (j * 2 - 2)) & 0x02) == 0x02)
          m_xiaomi_10();
        else
          m_xiaomi_00();
      }
    }
    // 2024.3.4小米二段码需要加等待时间
    m_xiaomi_wait();
    // 第二段码8个字节
    for (i = 0; i < 8; i++)
    {
      // 先发送11 12 13 14 寄存器数据，8位发送
      for (j = 4; j > 0; j--)
      {
        if (((m_xiaomi_data(2, i) >> (j * 2 - 2)) & 0x03) == 0x03)
          m_xiaomi_11();
        else if (((m_xiaomi_data(2, i) >> (j * 2 - 2)) & 0x01) == 0x01)
          m_xiaomi_01();
        else if (((m_xiaomi_data(2, i) >> (j * 2 - 2)) & 0x02) == 0x02)
          m_xiaomi_10();
        else
          m_xiaomi_00();
      }
    }
  }
  // 结束码
  m_xiaomi_stop();
  return true;
}

/**
 * @brief (私有函数) IR 海信红外遥控 输出
 *
 * @param  len 指令长度
 * @return bool 成功返回true，失败返回false
 */
bool IR::m_hisense(uint8_t len)
{
  // 长度判断
  if (21 != len && 23 != len)
    return false;

  uint8_t i = 0;
  uint8_t j = 0;
  // 头码
  m_hisense_start();
  if (21 == len)
  {
    // 海信科龙遥控协议 V3.0
    for (i = 0; i < 6; i++)
    {
      for (j = 0; j < 8; j++)
      {
        if (m_hisense_data(1, i) & (0x01 << j))
          m_hisense_1();
        else
          m_hisense_0();
      }
    }
    m_hisense_wait();
    // 海信科龙遥控协议
    for (i = 0; i < 8; i++)
    {
      for (j = 0; j < 8; j++)
      {
        if (m_hisense_data(2, i) & (0x01 << j))
          m_hisense_1();
        else
          m_hisense_0();
      }
    }
    m_hisense_wait();
    // 海信科龙遥控协议
    for (i = 0; i < 7; i++)
    {
      for (j = 0; j < 8; j++)
      {
        if (m_hisense_data(3, i) & (0x01 << j))
          m_hisense_1();
        else
          m_hisense_0();
      }
    }
  }
  else if (23 == len)
  {
    // 海信科龙遥控协议 V4.0
    for (i = 0; i < 23; i++)
    {
      for (j = 0; j < 8; j++)
      {
        if (m_hisense_data(1, i) & (0x01 << j))
          m_hisense_1();
        else
          m_hisense_0();
      }
    }
  }
  // 结束码
  m_hisense_stop();
  return true;
}

/**
 * @brief IR 构造函数
 *
 * @param name    对象名称
 * @param parent  父对象(指针)
 */
IR::IR(const std::string& name, Object* parent) : Object(name, parent)
{
  /* 资源分配 */
  m_data_tmp    = nullptr;
  m_gpio        = new Class::io(name + "_gpio", this);
  m_pulse_width = 32;
}

/**
 * @brief IR 红外遥控发送
 *
 * @param  type         红外遥控品牌类型
 * @param  data         指令数据
 * @param  len          指令长度
 * @param  count        发送次数
 * @param  delay_times  循环时间间隔(ms)
 * @return bool         成功返回true，失败返回false
 */
bool IR::send(ir_type type, const char* data, uint32_t len, uint8_t count, uint32_t delay_times)
{
  if (!is_open())
    return false;

  Mutex_Guard locker(m_mutex);

  /* 数据隔离 */
  m_data_tmp = new char[len];
  std::memcpy(m_data_tmp, data, len);

  bool ret;

  while (count)
  {
    /* 协议类型选择 */
    switch (type)
    {
      case ir_type::AUX :
        ret = m_aux(len);
        break;
      case ir_type::TCL :
        ret = m_tcl(len);
        break;
      case ir_type::GREE :
        ret = m_gree(len);
        break;
      case ir_type::OUTES :
        ret = m_outes(len);
        break;
      case ir_type::MIDEA :
        ret = m_midea(len);
        break;
      case ir_type::XIAOMI :
        ret = m_xiaomi(len);
        break;
      case ir_type::HISENSE :
        ret = m_hisense(len);
        break;
      default :
        ret = false;
        break;
    }

    if (!ret)
      break;
    else
    {
      Thread::msleep(delay_times);
      count--;
    }
  }

  /* 资源释放 */
  delete[] m_data_tmp;
  m_data_tmp = nullptr;
  return ret;
}

/**
 * @brief IR 析构函数
 */
IR::~IR()
{
  close();

  /* 资源释放 */
  if (m_data_tmp)
    delete[] m_data_tmp;
  if (m_gpio)
    delete m_gpio;
}
