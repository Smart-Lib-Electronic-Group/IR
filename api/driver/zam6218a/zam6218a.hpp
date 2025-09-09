#ifndef __ZAM6218A_HPP__
#define __ZAM6218A_HPP__

#include "virtual_iic.hpp"
#include "thread.hpp"

#define ZAM6218A_DEF_ERR_COUNT   10 /* 故障计数最大值 */
#define ZAM6218A_DEF_BREAK_COUNT 3  /* 断偶计数最大值 */

/// @brief 名称空间 库名
namespace OwO
{
namespace driver
{
enum Zam6218a_Type : uint8_t
{
  Type_T,
  Type_K,
};

class Zam6218a : public system::Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(Zam6218a)
  NO_MOVE(Zam6218a)

private:
  /// @brief ZAM6218A 校准偏移量
  float                m_calibration_offset[8] = { 0 };
  /// @brief ZAM6218A 源数据
  float                m_data[8]               = { 0 };
  /// @brief ZAM6218A 读取缓存区
  uint8_t              m_tmp[24]               = { 0 };
  /// @brief ZAM6218A 配置信息缓存区
  uint8_t              m_info[8]               = { 0 };
  /// @brief ZAM6218A 错误计数
  uint8_t              m_error_count;
  /// @brief ZAM6218A 断偶计数
  uint8_t              m_break_count[8];
  /// @brief ZAM6218A 总线
  virtual_class::VIIC* m_bus;
  /// @brief ZAM6218A 模式
  Zam6218a_Type        m_type[8] = { Type_T };

protected:
  void m_data_analysis();
  bool m_read_temperature();
  bool m_write_mode();
  bool m_read_mode();
  void m_iic_setup();

public:
  static constexpr inline uint8_t ALL_CHANNEL           = 0x00;
  static constexpr inline float   Thermocouple_breakage = 0x8000;
  static constexpr inline float   Chip_error            = 0xC000;
  static constexpr inline float   Disable               = 0xFFFF;

  Zam6218a(const std::string& name = "ZAM6218A", Object* parent = nullptr);
  bool                 open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin);
  bool                 open(virtual_class::VIIC* bus);
  bool                 close();
  void                 calibration(float calibrated_point, uint8_t channel);
  void                 calibration(float calibration_point, float current, uint8_t channel);
  bool                 set_type(Zam6218a_Type type, uint8_t channel = ALL_CHANNEL, bool update = true);
  Zam6218a_Type        get_type(uint8_t channel);
  void                 set_calibration_offset(float calibration_offset, uint8_t channel);
  float                get_calibration_offset(uint8_t channel) const;
  virtual_class::VIIC* bus() const;
  bool                 update();
  float                data(uint8_t channel, bool update = true);
  bool                 data(float& data, uint8_t channel, bool update = true);
  bool                 data(float* data, bool update = true);

  virtual ~Zam6218a()
  {
    close();
  }
};
} /* namespace driver */
} /* namespace OwO */
#endif /* __ZAM6218A_HPP__ */
