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
#ifndef __ERROR_HANDLE_H_
#define __ERROR_HANDLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

  /// @brief 枚举 错误代码
  typedef enum
  {
    SUCESS = 0,             // 无错误
    MEMORY_ERROR,           // 内存错误
    STACK_OVERFLOW_ERROR,   // 栈溢出错误
    INIT_ERROR,             // 初始化错误
    DEINIT_ERROR,           // 解除初始化错误
    NO_INIT_ERROR,          // 未初始化错误
    ALREADY_INIT_ERROR,     // 已初始化错误
    NULL_POINTER_ERROR,     // 空指针错误
    UNDEFINED_ERROR,        // 未定义错误
    SETUP_ERROR,            // 设置错误
    TRANSFER_ERROR,         // 传输错误
    NET_CONFIG_ERROR,       // 网络配置错误
    SOCKET_ERROR,           // 套接字错误
  } error_code_e;

  /// @brief 全局变量 错误代码
  extern error_code_e g_e_error_code;

  /// @brief 中断向量定义错误处理函数
#define IRQN_ERROR 91

  extern void v_error_handler(const char* file, const char* function, uint16_t line, const char* msg);

#define ERROR_HANDLE(msg) v_error_handler(__FILE__, __FUNCTION__, __LINE__, msg);

#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // __ERROR_HANDLE_H_
