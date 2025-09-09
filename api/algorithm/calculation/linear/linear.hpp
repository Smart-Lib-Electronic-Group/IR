/**
 * @file      linear.hpp
 * @author    Sea-Of-Quantum
 * @brief     OwO Library Linear Calculation Class (线性计算)
 * @version   version 1.0.0
 * @date      2025-06-24
 *
 * @copyright Copyright (c) 2025 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __LINEAR_HPP__
#define __LINEAR_HPP__

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
/// @brief 类 线性计算
class Linear
{
  O_MEMORY
  NO_COPY(Linear)
  NO_MOVE(Linear)
private:
  /// @brief 成员变量 斜率
  float m_k;
  /// @brief 成员变量 截距
  float m_b;

public:
  /**
   * @brief 线性计算 构造函数
   *
   * @param k 斜率
   * @param b 截距
   */
  Linear(float k, float b) : m_k(k), m_b(b) {}

  /**
   * @brief  线性计算 计算函数
   *
   * @param  x       输入值
   * @return float   计算结果
   */
  float calculate(float x) const
  {
    return m_k * x + m_b;
  }

  /**
   * @brief   线性计算 运算符重载
   *
   * @param   x       输入值
   * @return  float   计算结果
   */
  float operator()(float x) const
  {
    return m_k * x + m_b;
  }

  /**
   * @brief   线性计算 获得斜率
   *
   * @return  float   斜率
   */
  float get_k() const
  {
    return m_k;
  }

  /**
   * @brief  线性计算 获得截距
   *
   * @return float   截距
   */
  float get_b() const
  {
    return m_b;
  }

  /**
   * @brief  线性计算 设置斜率
   *
   * @param  k       斜率
   */
  void set_k(float k)
  {
    m_k = k;
  }

  /**
   * @brief  线性计算 设置截距
   *
   * @param  b       截距
   */
  void set_b(float b)
  {
    m_b = b;
  }

  /**
   * @brief  线性计算 析构函数
   *
   */
  ~Linear() {}
};
} /* namespace calculation */
} /* namespace algorithm */
} /* namespace OwO */

#endif /* __LINEAR_HPP__ */
