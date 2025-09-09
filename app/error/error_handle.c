/**
 * @file      error_handle.c
 * @author    Sea-Of-Quantum
 * @brief     Error handling (错误处理)
 * @version   version 1.0.0
 * @date      2024-12-31
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "error_handle.h"
#include "port_system.h"
#include "port_os.h"

error_code_e g_e_error_code = SUCESS;

/**
 * @brief 系统异常处理函数
 *
 */
void v_error_handler(const char* file, const char* function, uint16_t line, const char* msg)
{
  while (1)
  {
    msg = msg;
    v_port_system_reset();
    // 系统异常处理代码
    // 例如: 关闭所有任务，跳转到系统错误界面等
  }
}
