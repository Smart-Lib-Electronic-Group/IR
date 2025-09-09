/**
 * @file      moving_average.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Moving Average Filter Algorithm implementation（滑动窗口均值滤波算法)
 * @version   version 1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __MOVING_AVERAGE_HPP__
#define __MOVING_AVERAGE_HPP__

#include <cstdint>
#include "object.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 算法
namespace algorithm
{
/// @brief 名称空间 过滤器
namespace filter
{
/// @brief 类 均值滤波器
class Moving_Average
{
  O_MEMORY
  NO_COPY(Moving_Average)
  NO_MOVE(Moving_Average)

private:
  /// @brief 动态分配的缓冲区
  float*   m_buffer;
  /// @brief 当前总和
  float    m_sum;
  /// @brief 窗口大小
  uint16_t m_window_size;
  /// @brief 当前写入位置
  uint16_t m_index;
  /// @brief 当前元素数量
  uint16_t m_count;

public:
  /**
   * @brief 构造函数
   *
   * @param window_size 滤波器窗口大小
   * @param initial_value 初始值（默认为0）
   */
  Moving_Average(uint16_t window_size)
  {
    // 分配缓冲区内存
    m_buffer      = static_cast<float*>(Malloc(window_size * sizeof(float)));
    m_sum         = 0.0f;
    m_window_size = window_size;
    m_index       = 0;
    m_count       = 0;

    // 初始化缓冲区
    reset();
  }

  /**
   * @brief 输入新数据并获取平均值
   *
   * @param new_value 新输入值
   * @return float 当前平均值
   */
  float input(float new_value)
  {
    if (m_count != m_window_size)
    {
      // 窗口未满：添加新元素
      m_buffer[m_index]  = new_value;
      m_sum             += new_value;
      m_count++;
    }
    else
    {
      // 窗口已满：替换旧元素
      float removed      = m_buffer[m_index];
      m_buffer[m_index]  = new_value;
      m_sum             -= removed;
      m_sum             += new_value;
    }

    // 更新索引（优化取模运算）
    m_index++;
    if (m_index == m_window_size)
    {
      m_index = 0;
    }

    return output();
  }

  /**
   * @brief 获取当前平均值
   *
   * @return float 当前平均值
   */
  float output() const
  {
    return (m_count != m_window_size) ? (m_sum / m_count) : (m_sum / m_window_size);
  }

  /**
   * @brief 重置滤波器
   *
   * @param new_value 新的初始值
   */
  void reset()
  {
    for (uint16_t i = 0; i < m_window_size; ++i)
    {
      m_buffer[i] = 0.0f;
    }

    m_sum   = 0.0f;
    m_index = 0;
    m_count = 0;
  }

  /**
   * @brief 获取当前窗口大小
   *
   * @return uint16_t 窗口大小
   */
  uint16_t window_size() const
  {
    return m_window_size;
  }

  /**
   * @brief 修改窗口大小
   *
   * @param new_size 新的窗口大小
   * @param preserve_data 是否保留现有数据（默认为false）
   */
  void resize(uint16_t new_size, bool preserve_data = false);

  /**
   * @brief 析构函数
   */
  ~Moving_Average()
  {
    if (m_buffer)
    {
      Free(m_buffer);
      m_buffer = nullptr;
    }
  }
};

} /* namespace filter */
} /* namespace algorithm */
} /* namespace OwO */

#endif /* __MOVING_AVERAGE_HPP__ */
