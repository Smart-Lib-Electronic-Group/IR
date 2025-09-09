/**
 * @file      delta.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Delta Calculation Class (Delta运算)
 * @version   version 1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __DELTA_H__
#define __DELTA_H__

#include <cstdint>
#include "object.hpp"

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 算法
namespace algorithm
{
/// @brief 名称空间 计算
namespace calculation
{
/// @brief 类 Delta运算
class Delta
{
  O_MEMORY
  NO_COPY(Delta)
  NO_MOVE(Delta)

private:
  /// @brief 参考点
  float m_point;

public:
  /**
   * @brief Delta运算  构造函数
   *
   * @param point     参考点
   */
  explicit Delta(float point) : m_point(point) {}

  /**
   * @brief  Delta运算  计算函数
   * 
   * @param  value     输入值
   * @return float     输出值
   */
  float calculate(float value) const
  {
    return value - m_point;
  }

  /**
   * @brief Delta运算  运算符重载
   * 
   * @param  value     输入值
   * @return float     输出值
   */
  float operator()(float value) const
  {
    return value - m_point;
  }

  /**
   * @brief  Delta运算  获取参考点
   * 
   * @return float     参考点
   */
  float get_point() const
  {
    return m_point;
  }

  /**
   * @brief  Delta运算  设置参考点
   * 
   * @param  point     参考点
   */
  void set_point(float point)
  {
    m_point = point;
  }

  /**
   * @brief  Delta运算  析构函数
   * 
   */
  ~Delta() {}
};
} /* namespace calculation */
} /* namespace algorithm */
} /* namespace OwO */

#endif /* __DELTA_H__ */
