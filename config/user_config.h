/**
 * @file      user_config.h
 * @author    Sea-Of-Quantum
 * @brief     User configuration file (用户配置文件)
 * @version   version 1.0.0
 * @date      2024-12-31
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* 内存分配相关定义 */
#ifndef STM32F103 || STM32F429 || STM32H743
  #warning "Please define the chip you are using!"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __USER_CONFIG_H */