#ifndef __PORT_SYSTEM_H__
#define __PORT_SYSTEM_H__

#if __cplusplus
extern "C"
{
#endif

#include "error_handle.h"

  typedef struct PORT_SYSTEM_WORK_TIME_T
  {
    uint16_t days;
    uint16_t hours;
    uint16_t minutes;
    uint16_t seconds;
    uint64_t ticks;
  } port_system_work_time_t;

  extern error_code_e             e_port_system_init();
  extern void                     v_port_system_reset();
  extern port_system_work_time_t* p_port_system_get_work_time();
  extern void                     v_port_system_delay_us(uint32_t us);

#if __cplusplus
}
#endif

#endif /* __PORT_SYSTEM_H__ */
