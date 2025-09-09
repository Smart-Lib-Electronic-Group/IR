/**
 * @file      ir.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library API for IR device (红外遥控)
 * @version   v1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __IR_HPP__
#define __IR_HPP__

#include "virtual_gpio.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 枚举 IR 机型
enum class ir_type
{
  AUX,     /* 奥克斯 */
  TCL,     /* TCL */
  GREE,    /* 格力 */
  OUTES,   /* 中广欧斯特 */
  MIDEA,   /* 美的 */
  XIAOMI,  /* 小米 */
  HISENSE, /* 海信 */
};

/// @brief 类 IR -- 红外遥控
class IR : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(IR)
  NO_MOVE(IR)

private:
  enum
  {
    _0_32MS  = 320,
    _0_371MS = 371,
    _0_44MS  = 440,
    _0_5MS   = 500,
    _0_56MS  = 560,
    _0_67MS  = 670,
    _0_588MS = 588,
    _0_882MS = 882,
    _1MS     = 1000,
    _1_08MS  = 1080,
    _1_47MS  = 1470,
    _1_6MS   = 1600,
    _1_68MS  = 1680,
    _1_84MS  = 1840,
    _20MS    = 2000,
    _2_21MS  = 2210,
    _26_37MS = 2637,
    _3_08MS  = 3080,
    _3_64MS  = 3640,
    _40MS    = 4000,
    _4_5MS   = 4500,
    _5_22MS  = 5220,
    _71MS    = 7100,
    _8MS     = 8000,
    _9MS     = 9000,
  };

  /// @brief IR 互斥锁
  mutable system::kernel::Mutex m_mutex;
  /// @brief IR GPIO 端口数据
  Class::io*                    m_gpio;
  /// @brief IR 载波周期
  uint8_t                       m_pulse_width;
  /// @brief IR 指令数据缓存区
  char*                         m_data_tmp;

  /**
   * @brief (私有函数) IR 输出接口
   *
   * @param high_time 高电平周期
   * @param low_time  低电平周期
   */
  void m_ir_flash(uint32_t high_time, uint32_t low_time);

#if 1 /* AUX 奥克斯 */
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 头码
   *
   */
  void m_aux_start()
  {
    m_ir_flash(_9MS, _4_5MS);
  }
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 二进制0
   *
   */
  void m_aux_0()
  {
    m_ir_flash(_0_56MS, _0_56MS);
  }
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 二进制1
   *
   */
  void m_aux_1()
  {
    m_ir_flash(_0_56MS, _1_68MS);
  }
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 尾码
   *
   */
  void m_aux_stop()
  {
    m_ir_flash(_0_56MS, 0);
  }
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 获取数据
   *
   */
  char& m_aux_data(uint8_t pos)
  {
    return m_data_tmp[pos];
  }
  /**
   * @brief (私有内联函数) IR 奥克斯红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_aux(uint8_t len);
#endif

#if 1 /* TCL */
  /**
   * @brief (私有内联函数) IR TCL红外遥控 头码
   *
   */
  void m_tcl_start()
  {
    m_ir_flash(_3_08MS, _1_6MS);
  }
  /**
   * @brief (私有内联函数) IR TCL红外遥控 二进制0
   *
   */
  void m_tcl_0()
  {
    m_ir_flash(_0_5MS, _0_32MS);
  }
  /**
   * @brief (私有内联函数) IR TCL红外遥控 二进制1
   *
   */
  void m_tcl_1()
  {
    m_ir_flash(_0_5MS, _1_08MS);
  }
  /**
   * @brief (私有内联函数) IR TCL红外遥控 延时
   *
   */
  void m_tcl_wait()
  {
    m_ir_flash(0, _71MS);
  }
  /**
   * @brief (私有内联函数) IR TCL红外遥控 尾码
   *
   */
  void m_tcl_stop()
  {
    m_ir_flash(_0_5MS, 0);
  }
  /**
   * @brief (私有内联函数) IR TCL红外遥控 获取数据
   *
   */
  char& m_tcl_data(uint8_t loaction, uint8_t pos)
  {
    return m_data_tmp[pos + (loaction - 1) * 14];
  }
  /**
   * @brief (私有函数) IR TCL红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_tcl(uint8_t len);
#endif

#if 1 /* GREE 格力 */
  /**
   * @brief (私有内联函数) IR 格力红外遥控 头码
   *
   */
  void m_gree_start()
  {
    m_ir_flash(_9MS, _4_5MS);
  }
  /**
   * @brief (私有内联函数) IR 格力红外遥控 二进制0
   *
   */
  void m_gree_0()
  {
    m_ir_flash(_0_56MS, _0_67MS);
  }
  /**
   * @brief (私有内联函数) IR 格力红外遥控 二进制1
   *
   */
  void m_gree_1()
  {
    m_ir_flash(_0_56MS, _1_6MS);
  }
  /**
   * @brief (私有内联函数) IR 格力红外遥控 延时
   *
   * @param time 延时
   */
  void m_gree_wait(uint8_t time)
  {
    if (20 == time)
      m_ir_flash(_0_67MS, _20MS);
    else if (40 == time)
      m_ir_flash(_0_67MS, _40MS);
  }
  /**
   * @brief (私有内联函数) IR 格力红外遥控 尾码
   *
   */
  void m_gree_stop()
  {
    m_ir_flash(_0_67MS, _0_56MS);
  }
  /**
   * @brief (私有内联函数) IR 格力红外遥控 获取数据
   *
   */
  char& m_gree_data(uint8_t loaction, uint8_t num, uint8_t pos = 0);
  /**
   * @brief (私有函数) IR 格力红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool  m_gree(uint8_t len);
#endif

#if 1 /* OUTES 中广欧斯特 */
  /**
   * @brief (私有内联函数) IR 中广欧斯特红外遥控 头码
   *
   */
  void m_outes_start()
  {
    m_ir_flash(_3_64MS, _1_84MS);
  }
  /**
   * @brief (私有内联函数) IR 中广欧斯特红外遥控 二进制0
   *
   */
  void m_outes_0()
  {
    m_ir_flash(_0_44MS, _0_44MS);
  }
  /**
   * @brief (私有内联函数) IR 中广欧斯特红外遥控 二进制1
   *
   */
  void m_outes_1()
  {
    m_ir_flash(_0_44MS, _1_84MS);
  }
  /**
   * @brief (私有内联函数) IR 中广欧斯特红外遥控 尾码
   *
   */
  void m_outes_stop()
  {
    m_ir_flash(_0_44MS, 0);
  }
  /**
   * @brief (私有内联函数) IR 中广欧斯特红外遥控 获取数据
   *
   */
  char& m_outes_data(uint8_t pos)
  {
    return m_data_tmp[pos];
  }
  /**
   * @brief (私有函数) IR 中广欧斯特红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_outes(uint8_t len);
#endif

#if 1 /* MIDEA 美的 */
  /**
   * @brief (私有内联函数) IR 美的红外遥控 头码
   *
   */
  void m_midea_start()
  {
    m_ir_flash(_4_5MS, _4_5MS);
  }
  /**
   * @brief (私有内联函数) IR 美的红外遥控 二进制0
   *
   */
  void m_midea_0()
  {
    m_ir_flash(_0_56MS, _0_56MS);
  }
  /**
   * @brief (私有内联函数) IR 美的红外遥控 二进制1
   *
   */
  void m_midea_1()
  {
    m_ir_flash(_0_56MS, _1_6MS);
  }
  /**
   * @brief (私有内联函数) IR 美的红外遥控 延时
   *
   */
  void m_midea_wait()
  {
    m_ir_flash(_0_56MS, _5_22MS);
  }
  /**
   * @brief (私有内联函数) IR 美的红外遥控 尾码
   *
   */
  void m_midea_stop()
  {
    m_ir_flash(_0_56MS, _0_56MS);
  }
  /**
   * @brief (私有内联函数) IR 美的红外遥控 获取数据
   *
   */
  char& m_midea_data(uint8_t loaction, uint8_t pos)
  {
    return m_data_tmp[pos + (loaction - 1) * 6];
  }
  /**
   * @brief (私有函数) IR 美的红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_midea(uint8_t len);
#endif

#if 1 /* XIAOMI 小米 */
  /**
   * @brief (私有内联函数) IR 小米红外遥控 头码
   *
   */
  void m_xiaomi_start()
  {
    m_ir_flash(_1MS, _0_588MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 二进制00
   *
   */
  void m_xiaomi_00()
  {
    m_ir_flash(_0_588MS, _0_371MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 二进制01
   *
   */
  void m_xiaomi_01()
  {
    m_ir_flash(_0_588MS, _0_882MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 二进制10
   *
   */
  void m_xiaomi_10()
  {
    m_ir_flash(_0_588MS, _2_21MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 二进制11
   *
   */
  void m_xiaomi_11()
  {
    m_ir_flash(_0_588MS, _1_47MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 延时
   *
   */
  void m_xiaomi_wait()
  {
    m_ir_flash(_0_588MS, _26_37MS);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 尾码
   *
   */
  void m_xiaomi_stop()
  {
    m_ir_flash(_0_588MS, 0);
  }
  /**
   * @brief (私有内联函数) IR 小米红外遥控 获取数据
   *
   */
  char& m_xiaomi_data(uint8_t loaction, uint8_t pos)
  {
    return m_data_tmp[pos + (loaction - 1) * 11];
  }
  /**
   * @brief (私有函数) IR 小米红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_xiaomi(uint8_t len);
#endif

#if 1 /* HISENSE 海信 */
  /**
   * @brief (私有内联函数) IR 海信红外遥控 头码
   *
   */
  void m_hisense_start()
  {
    m_ir_flash(_9MS, _4_5MS);
  }
  /**
   * @brief (私有内联函数) IR 海信红外遥控 二进制0
   *
   */
  void m_hisense_0()
  {
    m_ir_flash(_0_56MS, _0_56MS);
  }
  /**
   * @brief (私有内联函数) IR 海信红外遥控 二进制1
   *
   */
  void m_hisense_1()
  {
    m_ir_flash(_0_56MS, _1_68MS);
  }
  /**
   * @brief (私有内联函数) IR 海信红外遥控 延时
   *
   */
  void m_hisense_wait()
  {
    m_ir_flash(_0_56MS, _8MS);
  }
  /**
   * @brief (私有内联函数) IR 海信红外遥控 尾码
   *
   */
  void m_hisense_stop()
  {
    m_ir_flash(_0_56MS, 0);
  }
  /**
   * @brief (私有内联函数) IR 海信红外遥控 获取数据
   *
   */
  char& m_hisense_data(uint8_t loaction, uint8_t pos)
  {
    if (1 == loaction)
      return m_data_tmp[pos];
    else if (2 == loaction)
      return m_data_tmp[pos + 6];
    else
      return m_data_tmp[pos + 14];
  }
  /**
   * @brief (私有函数) IR 海信红外遥控 输出
   *
   * @param  len 指令长度
   * @return bool 成功返回true，失败返回false
   */
  bool m_hisense(uint8_t len);
#endif

public:
  /**
   * @brief IR 构造函数
   *
   * @param name    对象名称
   * @param parent  父对象(指针)
   */
  IR(const std::string& name = "IR", Object* parent = nullptr);

  /**
   * @brief IR 开启端口
   *
   * @param  port 端口编号
   * @param  pin  引脚编号
   * @return bool 成功返回true，失败返回false
   */
  virtual bool open(Gpio::Port port, uint8_t pin, uint8_t pulse_width = 32)
  {
    m_pulse_width = pulse_width;
    m_is_open     = m_gpio->open(port, pin);
    return m_is_open;
  }

  /**
   * @brief  IR 关闭端口
   *
   * @return bool 成功返回true，失败返回false
   */
  virtual bool close()
  {
    m_is_open = false;
    return m_gpio->close();
  }

  /**
   * @brief IR 设置载波周期
   *
   * @param width 载波周期
   */
  void set_pulse_width(uint8_t width)
  {
    m_pulse_width = width;
  }

  /**
   * @brief IR 获取载波周期
   *
   * @return uint16_t 载波周期
   */
  uint8_t get_pulse_width() const
  {
    return m_pulse_width;
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
  bool send(ir_type type, const char* data, uint32_t len, uint8_t count = 1, uint32_t delay_times = 500);

  /**
   * @brief IR 析构函数
   */
  virtual ~IR();
};

} /* namespace device */
} /* namespace OwO */

#endif /* __IR_HPP__ */
