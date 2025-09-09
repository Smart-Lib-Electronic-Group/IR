/**
 * @file      user_malloc.c
 * @author    Sea-Of-Quantum
 * @brief     FreeRTOS port for memory (动态内存管理接口)
 * @version   version 2.0.0
 * @date      2025-03-18
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "port_memory.h"
#include "error_handle.h"
#include "port_os.h"
#include "FreeRTOS.h"
#include "portable.h"

#include <string.h>
// 内存块签名魔数
#define MEM_SIGNATURE_HEADER 0xDEADBEEF
#define MEM_SIGNATURE_FOOTER 0xCAFEBABE

// 内存池配置
enum
{
  POOL_008B,
  POOL_016B,
  POOL_032B,
  POOL_064B,
  MAX_POOLS
};

// 内存池结构
typedef struct PORT_OS_MEMORY_POOL_T
{
  size_t block_size;     // 块大小（不含头尾）
  size_t total_blocks;   // 总块数
  size_t free_blocks;    // 空闲块数
  void*  free_list;      // 空闲链表
  size_t expand_size;    // 下次扩展块数
} port_os_memory_pool_t;

// 内存块头结构（8字节对齐）
typedef struct __attribute__((aligned(8)))
{
  uint32_t                      signature;    // 头校验
  size_t                        alloc_size;   // 实际分配大小
  struct PORT_OS_MEMORY_POOL_T* pool;         // 所属内存池
} port_os_memory_block_header_t;

// 全局内存池数组
static port_os_memory_pool_t memory_pools[MAX_POOLS] = {
  { 8,  0, 0, NULL, 4 },
  { 16, 0, 0, NULL, 4 },
  { 32, 0, 0, NULL, 4 },
  { 64, 0, 0, NULL, 4 },
};

// 互斥锁
static port_os_mutex_t s_memory_mutex;
// 内存管理统计信息
struct
{
  size_t total_allocated;   // 当前已分配内存
  size_t peak_usage;        // 峰值内存使用量
  size_t alloc_count;       // 总分配次数
  size_t free_count;        // 总释放次数
  size_t expand_count;      // 内存池扩展次数
  size_t error_count;       // 检测到的错误数
  size_t direct_allocs;     // 直接分配内存次数
} s_memory_stats;

// 内存对齐计算
static inline size_t sl_port_os_memory_align_up(size_t size, size_t alignment)
{
  return (size + alignment - 1) & ~(alignment - 1);
}

// 选择合适的内存池
static port_os_memory_pool_t* s_port_os_memory_select_pool(size_t size)
{
  for (int i = 0; i < MAX_POOLS; ++i)
  {
    if (size <= memory_pools[i].block_size)
    {
      return &memory_pools[i];
    }
  }
  return NULL;
}

// 扩展指定内存池
static void s_port_os_memory_expand_pool(port_os_memory_pool_t* pool)
{
  const size_t header_size = sl_port_os_memory_align_up(sizeof(port_os_memory_block_header_t), 8);
  const size_t block_size  = header_size + pool->block_size + sizeof(uint32_t);

  // 分配内存块组
  void* new_blocks         = pvPortMalloc(block_size * pool->expand_size);
  if (!new_blocks)
  {
    s_memory_stats.error_count++;
    return;
  }

  // 初始化每个块
  for (size_t i = 0; i < pool->expand_size; ++i)
  {
    port_os_memory_block_header_t* header = (port_os_memory_block_header_t*)((uint8_t*)new_blocks + i * block_size);

    // 初始化头
    header->signature                     = MEM_SIGNATURE_HEADER;
    header->alloc_size                    = pool->block_size;
    header->pool                          = pool;

    // 初始化尾
    uint32_t* footer                      = (uint32_t*)((uint8_t*)header + header_size + pool->block_size);
    *footer                               = MEM_SIGNATURE_FOOTER;

    // 加入空闲链表
    void** next                           = (void**)((uint8_t*)header + header_size);
    *next                                 = pool->free_list;
    pool->free_list                       = header;

    pool->total_blocks++;
    pool->free_blocks++;
  }

  // 更新扩展参数
  pool->expand_size *= 2;
  s_memory_stats.expand_count++;
}

// 内存管理初始化
void v_port_os_memory_init(void)
{
  s_memory_mutex = pt_port_os_mutex_create();
  b_port_os_mutex_release(s_memory_mutex);
  memset(&s_memory_stats, 0, sizeof(s_memory_stats));
}

// 核心分配函数
void* Malloc(size_t size)
{
  if (size == 0 || size > (1024 * 1024))
  {   // 限制最大1MB
    s_memory_stats.error_count++;
    return NULL;
  }

  b_port_os_mutex_wait(s_memory_mutex, WAIT_FOREVER);

  const size_t                   aligned_size = sl_port_os_memory_align_up(size, 8);
  port_os_memory_pool_t*         pool         = s_port_os_memory_select_pool(aligned_size);
  port_os_memory_block_header_t* header       = NULL;

  if (pool)
  {
    // 从内存池分配
    if (!pool->free_list && pool->free_blocks == 0)
    {
      s_port_os_memory_expand_pool(pool);
    }

    if (pool->free_list)
    {
      header          = (port_os_memory_block_header_t*)pool->free_list;
      pool->free_list = *(void**)((uint8_t*)header + sizeof(port_os_memory_block_header_t));
      pool->free_blocks--;
    }
  }
  else
  {
    // 直接分配大块内存
    const size_t header_size = sl_port_os_memory_align_up(sizeof(port_os_memory_block_header_t), 8);
    const size_t total_size  = header_size + aligned_size + sizeof(uint32_t);

    header                   = (port_os_memory_block_header_t*)pvPortMalloc(total_size);
    if (header)
    {
      header->signature  = MEM_SIGNATURE_HEADER;
      header->alloc_size = aligned_size;
      header->pool       = NULL;
      s_memory_stats.direct_allocs++;

      // 设置尾部签名
      uint32_t* footer = (uint32_t*)((uint8_t*)header + header_size + aligned_size);
      *footer          = MEM_SIGNATURE_FOOTER;
    }
  }

  if (header)
  {
    // 更新统计信息
    s_memory_stats.total_allocated += aligned_size;
    s_memory_stats.alloc_count++;
    if (s_memory_stats.total_allocated > s_memory_stats.peak_usage)
    {
      s_memory_stats.peak_usage = s_memory_stats.total_allocated;
    }
  }
  else
  {
    s_memory_stats.error_count++;
  }

  b_port_os_mutex_release(s_memory_mutex);
  return header ? (uint8_t*)header + sl_port_os_memory_align_up(sizeof(port_os_memory_block_header_t), 8) : NULL;
}

// 核心释放函数
void Free(void* ptr)
{
  if (!ptr)
    return;

  b_port_os_mutex_wait(s_memory_mutex, WAIT_FOREVER);

  port_os_memory_block_header_t* header = (port_os_memory_block_header_t*)((uint8_t*)ptr - sl_port_os_memory_align_up(sizeof(port_os_memory_block_header_t), 8));

  // 校验内存完整性
  if (header->signature != MEM_SIGNATURE_HEADER)
  {
    s_memory_stats.error_count++;
    b_port_os_mutex_release(s_memory_mutex);
    return;
  }

  const size_t header_size = sl_port_os_memory_align_up(sizeof(port_os_memory_block_header_t), 8);
  uint32_t*    footer      = (uint32_t*)((uint8_t*)ptr + header->alloc_size);
  if (*footer != MEM_SIGNATURE_FOOTER)
  {
    s_memory_stats.error_count++;
    b_port_os_mutex_release(s_memory_mutex);
    return;
  }

  // 更新统计
  s_memory_stats.total_allocated -= header->alloc_size;
  s_memory_stats.free_count++;

  if (header->pool)
  {
    // 回收到内存池
    void** next             = (void**)((uint8_t*)header + header_size);
    *next                   = header->pool->free_list;
    header->pool->free_list = header;
    header->pool->free_blocks++;
  }
  else
  {
    // 直接释放
    vPortFree(header);
    s_memory_stats.direct_allocs--;
  }

  b_port_os_mutex_release(s_memory_mutex);
}

float ul_port_os_get_space()
{
  return (float)xPortGetFreeHeapSize() / (float)configTOTAL_HEAP_SIZE * 100.0f;
}

// 获取统计信息
void v_port_os_memory_get_stats(port_os_memory_stats_t* stats)
{
  if (!stats)
    return;

  b_port_os_mutex_wait(s_memory_mutex, WAIT_FOREVER);

  stats->total_allocated = s_memory_stats.total_allocated;
  stats->peak_usage      = s_memory_stats.peak_usage;
  stats->alloc_count     = s_memory_stats.alloc_count;
  stats->free_count      = s_memory_stats.free_count;
  stats->expand_count    = s_memory_stats.expand_count;
  stats->error_count     = s_memory_stats.error_count;

  for (int i = 0; i < MAX_POOLS; ++i)
  {
    const size_t used       = memory_pools[i].total_blocks - memory_pools[i].free_blocks;
    stats->pool_fragment[i] = (used * 100) / memory_pools[i].total_blocks;
  }

  b_port_os_mutex_release(s_memory_mutex);
}
