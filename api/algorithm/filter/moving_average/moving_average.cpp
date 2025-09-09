/**
 * @file      moving_average.cpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Moving Average Filter Algorithm implementation（滑动窗口均值滤波算法)
 * @version   version 1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "moving_average.hpp"

using namespace OwO::algorithm::filter;

void Moving_Average::resize(uint16_t new_size, bool preserve_data)
{
  if (new_size == m_window_size)
    return;

  // 分配新缓冲区
  float* new_buffer = static_cast<float*>(Malloc(new_size * sizeof(float)));

  if (preserve_data)
  {
    // 计算需要保留的元素数量
    uint16_t elements_to_keep = (m_count < new_size) ? m_count : new_size;

    // 复制最近的数据到新缓冲区
    uint16_t start_index      = (m_index >= elements_to_keep) ? (m_index - elements_to_keep) : (m_window_size + m_index - elements_to_keep);

    float new_sum             = 0.0f;
    for (uint16_t i = 0; i < elements_to_keep; ++i)
    {
      uint16_t src_index  = (start_index + i) % m_window_size;
      new_buffer[i]       = m_buffer[src_index];
      new_sum            += m_buffer[src_index];
    }

    // 填充剩余位置
    for (uint16_t i = elements_to_keep; i < new_size; ++i)
    {
      new_buffer[i] = 0.0f;
    }

    // 更新状态
    m_sum   = new_sum;
    m_count = elements_to_keep;
    m_index = elements_to_keep % new_size;
  }
  else
  {
    // 不保留数据，直接初始化
    for (uint16_t i = 0; i < new_size; ++i)
    {
      new_buffer[i] = 0.0f;
    }
    m_sum   = 0.0f;
    m_index = 0;
    m_count = 0;
  }

  // 更新窗口大小和倒数
  m_window_size     = new_size;

  // 释放旧缓冲区并更新指针
  Free(m_buffer);
  m_buffer = new_buffer;
}
