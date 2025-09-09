#include "Zam6218a.hpp"

using namespace OwO;
using namespace driver;
using namespace system;
using namespace kernel;
using namespace virtual_class;

O_METAOBJECT(Zam6218a, Object)

/* ZAM6218A 通道配置 */
#define ZAM6218A_CHAN_DEN         0 << 7 /* 通道禁止 */
#define ZAM6218A_CHAN_EN          1 << 7 /* 通道使能 */
/* ZAM6218A 热电偶类型 */
#define ZAM6218A_TYPE_T           0 << 6 /* T型热电偶 */
#define ZAM6218A_TYPE_K           1 << 6 /* K型热电偶 */
/* ZAM6218A 测温范围 */
#define ZAM6218A_TEMP_RANGE_0     0 << 5 /* (-100~300) 摄氏度 */
#define ZAM6218A_TEMP_RANGE_1     1 << 5 /* (-270~1300)摄氏度 */
/* ZAM6218A 冷端芯片 */
#define ZAM6218A_COLD_CHIP_TMP116 0 << 4 /* TMP116 */
#define ZAM6218A_COLD_CHIP_M117   1 << 4 /* M117 */
/* ZAM6218A 数据地址 */
#define ZAM6218A_ADDR             (0X49) /* 器件地址 */
#define ZAM6218A_CONFIG_ADDR      (0X01) /* 配置寄存器地址 */
#define ZAM6218A_CONFIG_LEN       (0X08) /* 配置寄存器长度 */
#define ZAM6218A_TEMP_ADDR        (0X00) /* 温度寄存器地址 */
#define ZAM6218A_TEMP_LEN         (0X18) /* 温度寄存器长度 */

void Zam6218a::m_data_analysis()
{
  static uint32_t ul_tmp;
  static float    f_tmp;

  for (uint8_t i = 0; i < 8; i++)
  {
    ul_tmp = 0;
    /* 拼合数据 */
    ul_tmp = m_tmp[i * 3] | m_tmp[i * 3 + 1] << 8 | m_tmp[i * 3 + 2] << 16;
    /* 滤除异常数据 */
    if (ul_tmp != 0x000000 && ul_tmp != 0xFFFFFF)
    {
      /* 当前测量温度为负温度值 */
      if (ul_tmp >= 8388608)
        f_tmp = -(float)((16777216 - ul_tmp) / (float)(0x1 << 13));
      /* 当前测量温度为正温度值 */
      else
        f_tmp = (float)(ul_tmp / (float)(0x1 << 13));

      if (m_type[i] == Type_T)
      {
        if (f_tmp >= -100.0f && f_tmp <= 350.0f)
        {
          m_break_count[i] = 0;
          m_data[i]        = f_tmp;
        }
        else
        {
          if (m_break_count[i] < ZAM6218A_DEF_ERR_COUNT)
            m_break_count[i]++;
          else
            m_data[i] = 400.0f;
        }
      }
      else if (m_type[i] == Type_K)
      {
        if (f_tmp >= -100.0f && f_tmp <= 400.0f)
        {
          m_break_count[i] = 0;
          m_data[i]        = f_tmp;
        }
        else
        {
          if (m_break_count[i] < ZAM6218A_DEF_ERR_COUNT)
            m_break_count[i]++;
          else
            m_data[i] = 400.0f;
        }
      }
    }
  }
}

bool Zam6218a::m_read_temperature()
{
  m_iic_setup();
  /* 读取数据 */
  if (ZAM6218A_TEMP_LEN == m_bus->read(m_tmp, ZAM6218A_TEMP_LEN, ZAM6218A_TEMP_ADDR))
  {
    /* 异常计数清零 */
    m_error_count = 0;
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
    {
      /* 异常计数达到最大值，重新设置模式 */
      m_write_mode();
    }
    else
    {
      /* 数据分析 */
      m_data_analysis();
    }
    return true;
  }
  else
  {
    /* 异常计数自加 */
    if (m_error_count < ZAM6218A_DEF_ERR_COUNT)
      m_error_count++;
    return false;
  }
}

bool Zam6218a::m_write_mode()
{
  m_iic_setup();
  /* 写入数据 */
  return (ZAM6218A_CONFIG_LEN == m_bus->write(m_info, ZAM6218A_CONFIG_LEN, ZAM6218A_CONFIG_ADDR));
}

bool Zam6218a::m_read_mode()
{
  m_iic_setup();
  /* 读取数据 */
  return (ZAM6218A_CONFIG_LEN == m_bus->read(m_info, ZAM6218A_CONFIG_LEN, ZAM6218A_CONFIG_ADDR));
}

void Zam6218a::m_iic_setup()
{
  m_bus->set_address(ZAM6218A_ADDR << 1);
  m_bus->set_address_type(1);
  m_bus->set_clk_time(2);
}

Zam6218a::Zam6218a(const std::string& name, Object* parent) : Object(name, parent)
{
  memset(m_calibration_offset, 0, sizeof(m_calibration_offset));
  memset(m_data, 0, sizeof(m_data));
  memset(m_tmp, 0, sizeof(m_tmp));

  for (uint8_t i = 0; i < 8; i++)
  {
    m_type[i] = Type_T;
    m_info[i] = ZAM6218A_CHAN_EN | ZAM6218A_TYPE_T | ZAM6218A_TEMP_RANGE_1 | ZAM6218A_COLD_CHIP_TMP116 | 0;
  }

  m_bus         = nullptr;
  m_error_count = 0;
}

bool Zam6218a::open(Gpio::Port scl_port, uint8_t scl_pin, Gpio::Port sda_port, uint8_t sda_pin)
{
  if (m_is_open)
    return false;

  m_bus = new virtual_class::VIIC(name() + "_iic", this);
  if (false == m_bus->open(ZAM6218A_ADDR << 1, scl_port, scl_pin, sda_port, sda_pin))
    return false;
  else
  {
    m_write_mode();
    m_is_open = true;
    return true;
  }
}

bool Zam6218a::open(virtual_class::VIIC* bus)
{
  if (m_is_open)
    return false;

  m_bus = bus;
  m_write_mode();
  m_is_open = true;
  return true;
}

bool Zam6218a::close()
{
  if (!m_is_open)
    return true;

  bool ret = true;

  if (m_bus->parent() == this)
    ret = m_bus->close();

  if (ret)
  {
    m_is_open = false;
    m_bus     = nullptr;
  }
  return ret;
}

void Zam6218a::calibration(float calibrated_point, uint8_t channel)
{
  if (!m_is_open)
    return;

  if (channel == ALL_CHANNEL)
  {
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
      return;

    for (uint8_t i = 0; i < 8; i++)
    {
      m_calibration_offset[i] = calibrated_point - m_data[i];
    }
  }
  else if (channel < 1 || channel > 8)
    return;
  else
  {
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
      return;

    m_calibration_offset[channel - 1] = calibrated_point - m_data[channel - 1];
  }
}

void Zam6218a::calibration(float calibration_point, float current, uint8_t channel)
{
  if (channel == ALL_CHANNEL)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      m_calibration_offset[i] = calibration_point - current;
    }
  }
  else if (channel < 1 || channel > 8)
    return;
  else
    m_calibration_offset[channel - 1] = calibration_point - current;
}

bool Zam6218a::set_type(Zam6218a_Type type, uint8_t channel, bool update)
{
  if (!m_is_open && update)
    return false;

  if (channel == ALL_CHANNEL)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (type == Type_T)
        m_info[i] = ZAM6218A_CHAN_EN | ZAM6218A_TYPE_T | ZAM6218A_TEMP_RANGE_1 | ZAM6218A_COLD_CHIP_TMP116 | 0;
      else
        m_info[i] = ZAM6218A_CHAN_EN | ZAM6218A_TYPE_K | ZAM6218A_TEMP_RANGE_1 | ZAM6218A_COLD_CHIP_TMP116 | 0;

      m_type[i] = type;
    }
  }
  else if (channel < 1 || channel > 8)
    return false;
  else
  {
    if (type == Type_T)
      m_info[channel - 1] = ZAM6218A_CHAN_EN | ZAM6218A_TYPE_T | ZAM6218A_TEMP_RANGE_1 | ZAM6218A_COLD_CHIP_TMP116 | 0;
    else
      m_info[channel - 1] = ZAM6218A_CHAN_EN | ZAM6218A_TYPE_K | ZAM6218A_TEMP_RANGE_1 | ZAM6218A_COLD_CHIP_TMP116 | 0;

    m_type[channel - 1] = type;
  }

  if (update)
    return m_write_mode();
  else
    return true;
}

void Zam6218a::set_calibration_offset(float calibration_offset, uint8_t channel)
{
  if (channel == ALL_CHANNEL)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      m_calibration_offset[i] = calibration_offset;
    }
  }
  else if (channel < 1 || channel > 8)
    return;
  else
    m_calibration_offset[channel - 1] = calibration_offset;
}

Zam6218a_Type Zam6218a::get_type(uint8_t channel)
{
  if (channel < 1 || channel > 8)
    return Type_T;
  else
  {
    uint8_t tmp[8] = { 0 };
    m_bus->read(tmp, ZAM6218A_CONFIG_LEN, ZAM6218A_CONFIG_ADDR);

    for (uint8_t i = 0; i < 8; i++)
    {
      m_type[i] = (tmp[i] & (0x01 << 6)) ? Type_K : Type_T;
    }

    return m_type[channel - 1];
  }
}

float Zam6218a::get_calibration_offset(uint8_t channel) const
{
  if (channel < 1 || channel > 8)
    return 0.0f;
  else
    return m_calibration_offset[channel - 1];
}

VIIC* Zam6218a::bus() const
{
  return m_bus;
}

/**
 * @brief  ZAM6218A 更新数据
 *
 * @return true   成功
 * @return false  失败
 */
bool Zam6218a::update()
{
  if (!m_is_open)
    return false;

  return m_read_temperature();
}

/**
 * @brief  ZAM6218A 获取数据
 *
 * @param  channel  通道编号
 * @param  update   是否更新数据
 * @return float    滤波后数据
 */
float Zam6218a::data(uint8_t channel, bool update)
{
  if (!m_is_open)
    return 0.0f;

  if (update)
    this->update();

  if (channel < 1 || channel > 8)
  {
    return 0.0f;
  }
  else
  {
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
      return Chip_error;
    else
    {
      if (400.0f == m_data[channel - 1])
        return Thermocouple_breakage;
      else
        return m_data[channel - 1] + m_calibration_offset[channel - 1];
    }
  }
}

bool Zam6218a::data(float& data, uint8_t channel, bool update)
{
  if (!m_is_open)
    return false;

  if (update)
    this->update();

  if (channel < 1 || channel > 8)
  {
    return false;
  }
  else
  {
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
    {
      data = Chip_error;
      return false;
    }
    else
    {
      if (400.0f == m_data[channel - 1])
      {
        data = Thermocouple_breakage;
        return true;
      }
      else
      {
        data = m_data[channel - 1] + m_calibration_offset[channel - 1];
        return true;
      }
    }
  }
}

bool Zam6218a::data(float* data, bool update)
{
  if (!m_is_open)
    return false;

  if (update)
    this->update();

  bool ret = true;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (m_error_count >= ZAM6218A_DEF_ERR_COUNT)
    {
      data[i] = Chip_error;
      ret     = false;
    }
    else
    {
      if (400.0f == m_data[i])
      {
        data[i] = Thermocouple_breakage;
      }
      else
      {
        data[i] = m_data[i] + m_calibration_offset[i];
      }
    }
  }

  return ret;
}
