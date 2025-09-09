#ifndef __PORT_OS_ISR_H__
#define __PORT_OS_ISR_H__

#if __cplusplus
extern "C"
{
#endif

#include "FreeRTOS.h"
#include "task.h"

#define port_os_enter_critical_isr()        taskENTER_CRITICAL_FROM_ISR()
#define port_os_exit_critical_isr(isr_mask) taskEXIT_CRITICAL_FROM_ISR(isr_mask)
#define port_os_yield()                     taskYIELD()

#if __cplusplus
}
#endif

#endif /* __PORT_OS_ISR_H__ */
