/**
 * @file      port_include.h
 * @author    Sea-Of-Quantum
 * @brief     STM32F429 HAL port Driver Include File (驱动接口——引用文件)
 * @version   version 2.0.0
 * @date      2025-03-15
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __PORT_INCLUDE_H__
#define __PORT_INCLUDE_H__

/* C语言头文件 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 错误处理 */
#include "error_handle.h"

/* HAL库头文件 */
#include "stm32f4xx_hal.h"

/* 系统头文件 */
#include "port_os.h"
#include "port_memory.h"

#endif /* __PORT_INCLUDE_H__ */
