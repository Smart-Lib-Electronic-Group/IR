/**
 * @file      user_malloc.cpp
 * @author    Sea-Of-Quantum
 * @brief     FreeRTOS port for memory (动态内存管理接口)
 * @version   version 2.0.0
 * @date      2025-03-09
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_memory.h"

void* operator new(size_t size)
{
  return Malloc(size);
}

void operator delete(void* p)
{
  Free(p);
}

void* operator new[](size_t size)
{
  return Malloc(size);
}

void operator delete[](void* p)
{
  Free(p);
}
