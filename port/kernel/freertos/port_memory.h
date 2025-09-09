/**
 * @file      user_malloc.h
 * @author    Sea-Of-Quantum
 * @brief     FreeRTOS port for memory (动态内存管理接口)
 * @version   version 2.0.0
 * @date      2025-03-18
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __USER_MALLOC_H__
#define __USER_MALLOC_H__

#if __cplusplus
extern "C"
{
#endif

#include <stddef.h>

  typedef struct PORT_OS_MEMORY_STATS_T
  {
    size_t total_allocated;    // 当前已分配内存
    size_t peak_usage;         // 峰值内存使用量
    size_t alloc_count;        // 总分配次数
    size_t free_count;         // 总释放次数
    size_t expand_count;       // 内存池扩展次数
    size_t error_count;        // 检测到的错误数
    size_t pool_fragment[4];   // 各内存池碎片率
  } port_os_memory_stats_t;

  extern void* Malloc(size_t size);
  extern void  Free(void* p);
  extern float ul_port_os_get_space();
  extern void  v_port_os_memory_get_stats(port_os_memory_stats_t* stats);

#if __cplusplus
}
#endif /* __cplusplus */

#define O_MEMORY                           \
public:                                    \
  static void* operator new(size_t size)   \
  {                                        \
    return Malloc(size);                   \
  }                                        \
  static void operator delete(void* p)     \
  {                                        \
    Free(p);                               \
  }                                        \
  static void* operator new[](size_t size) \
  {                                        \
    return Malloc(size);                   \
  }                                        \
  static void operator delete[](void* p)   \
  {                                        \
    Free(p);                               \
  }

#endif /* __USER_MALLOC_H__ */
