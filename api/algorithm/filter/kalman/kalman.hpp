/**
 * @file      kalman.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Kalman Filter Algorithm implementation （一阶卡尔曼滤波器算法)
 * @version   version 3.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __KALMAN_HPP__
#define __KALMAN_HPP__

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
/// @brief 类 卡尔曼滤波器
class Kalman
{
  O_MEMORY
  NO_COPY(Kalman)
  NO_MOVE(Kalman)
private:
  /// @brief 卡尔曼滤波器 突变阈值
  float    m_threshold;
  /// @brief 卡尔曼滤波器 k-1时刻的预测值
  float    m_last_predict_value;
  /// @brief 卡尔曼滤波器 k时刻的输出值
  float    m_output_value;
  /// @brief 卡尔曼滤波器 过程噪声
  float    m_Q;
  /// @brief 卡尔曼滤波器 观测噪声
  float    m_R;
  /// @brief 卡尔曼滤波器 卡尔曼增益
  float    m_K;
  /// @brief 卡尔曼滤波器 突变计数器
  uint32_t m_error_couter;
  /// @brief 卡尔曼滤波器 突变计数最大值
  uint32_t m_error_max;
  /// @brief 卡尔曼滤波器 是否第一次采集数据
  bool     m_is_first;

public:
  /**
   * @brief 卡尔曼滤波器 构造函数
   *
   * @param Q         过程噪声
   * @param R         观测噪声
   * @param threshold 突变阈值
   * @param error_max 最大误差次数
   */
  Kalman(float Q, float R, float threshold = 0, uint32_t error_max = 0)
  {
    m_threshold    = threshold;
    m_Q            = Q;
    m_R            = R;
    m_is_first     = true;
    m_error_max    = error_max;
    m_error_couter = 0;
  }

  /**
   * @brief  卡尔曼滤波器 数据输入
   *
   * @param  new_input  新数据
   * @return float      输出值
   */
  float input(float new_input);

  /**
   * @brief  卡尔曼滤波器 数据输出
   *
   * @return float      输出值
   */
  float output()
  {
    return m_output_value;
  }

  /**
   * @brief  卡尔曼滤波器 重置
   *
   * @param  new_input  新数据
   * @return Kalman&    自身引用
   */
  Kalman& reset(float new_input)
  {
    m_last_predict_value = new_input;
    m_output_value       = new_input;
    m_K                  = 0.0f;
    m_is_first           = false;
    m_error_couter       = 0;
    return *this;
  }

  /**
   * @brief  卡尔曼滤波器 设置过程噪声
   *
   * @param  Q          过程噪声
   * @return Kalman&    自身引用
   */
  Kalman& set_q(float Q)
  {
    m_Q        = Q;
    m_is_first = true;

    return *this;
  }

  /**
   * @brief  卡尔曼滤波器 设置观测噪声
   *
   * @param  R          观测噪声
   * @return Kalman&    自身引用
   */
  Kalman& set_r(float R)
  {
    m_R        = R;
    m_is_first = true;

    return *this;
  }

  /**
   * @brief  卡尔曼滤波器 设置突变阈值
   *
   * @param  threshold  突变阈值
   * @return Kalman&    自身引用
   */
  Kalman& set_threshold(float threshold)
  {
    m_threshold = threshold;
    return *this;
  }

  /**
   * @brief  卡尔曼滤波器 设置最大误差次数
   *
   * @param  error_max  最大误差次数
   * @return Kalman&    自身引用
   * @note   当最大误差次数为0时，将不进行突变检测
   */
  Kalman& set_error_max(uint32_t error_max)
  {
    m_error_max = error_max;
    return *this;
  }

  /**
   * @brief  卡尔曼滤波器 获取过程噪声
   *
   * @return float 过程噪声
   */
  float get_q()
  {
    return m_Q;
  }

  /**
   * @brief  卡尔曼滤波器 获取观测噪声
   *
   * @return float 观测噪声
   */
  float get_r()
  {
    return m_R;
  }

  /**
   * @brief  卡尔曼滤波器 获取突变阈值
   *
   * @return float 突变阈值
   */
  float get_threshold()
  {
    return m_threshold;
  }

  /**
   * @brief  卡尔曼滤波器 获取最大误差次数
   *
   * @return uint32_t 最大误差次数
   */
  uint32_t get_error_max()
  {
    return m_error_max;
  }

  /**
   * @brief  卡尔曼滤波器 析构函数
   *
   */
  ~Kalman() {}
};
} /* namespace filter */
} /* namespace algorithm */
} /* namespace OwO */

#endif /* __KALMAN_HPP__ */
