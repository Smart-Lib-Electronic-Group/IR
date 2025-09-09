/**
 * @file      kalman.cpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Kalman Filter Algorithm implementation （一阶卡尔曼滤波器算法)
 * @version   version 3.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "Kalman.hpp"

using namespace OwO::algorithm::filter;

float Kalman::input(float new_input)
{
  // 第一次输入快速路径
  if (m_is_first)
  {
    reset(new_input);
    return m_output_value;
  }

  // 突变检测优化（单条件判断+位操作）
  const float diff = new_input - m_output_value;
  uint32_t    diff_bits;
  memcpy(&diff_bits, &diff, sizeof(float));

  // 位操作快速检查绝对值比较（避免分支）
  const bool is_over_threshold = (diff_bits & 0x80000000 ? (-diff >= m_threshold) : (diff >= m_threshold));

  // 突变处理流程优化
  if (m_error_max && is_over_threshold)
  {
    if (m_error_couter >= m_error_max)
    {
      reset(new_input);
    }
    else
    {
      m_error_couter++;
    }
    return m_output_value;
  }

  // 核心计算优化（减少中间变量）
  const float predict           = m_last_predict_value + m_Q;
  const float K                 = predict / (predict + m_R);

  // 合并计算步骤（减少一次浮点运算）
  m_output_value               += K * (new_input - m_output_value);

  // 并行计算优化（避免依赖链）
  const float last_predict_tmp  = (1.0f - K) * predict;
  m_last_predict_value          = last_predict_tmp;

  // 重置计数器（无分支）
  m_error_couter                = 0;

  return m_output_value;
}
