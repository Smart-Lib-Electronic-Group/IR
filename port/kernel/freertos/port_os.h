#ifndef __PORT_OS_H__
#define __PORT_OS_H__

#if __cplusplus
extern "C"
{
#endif

#include "port_memory.h"
#include <stdint.h>
#include <stdbool.h>

#define WAIT_FOREVER               0xffffffffUL
#define PORT_OS_INTERRUPT_PRIORITY 5

  typedef void (*port_os_thread_function_t)(void*);
  typedef void (*port_os_timer_callback_t)(void*);
  typedef void* port_os_thread_t;
  typedef void* port_os_timer_t;
  typedef void* port_os_mutex_t;
  typedef void* port_os_semaphore_t;
  typedef void* port_os_event_t;
  typedef void* port_os_message_t;
  typedef void* port_os_stream_t;

  extern void                v_port_os_init(uint32_t start_thread_stack_size);
  extern void                v_port_os_start();
  extern uint32_t            ul_port_os_get_tick_count();
  extern void                v_port_os_enter_critical();
  extern void                v_port_os_exit_critical();
  extern port_os_thread_t    pt_port_os_thread_create(const char* const name, const uint32_t stack_depth, uint16_t priority, port_os_thread_function_t function, void* arg);
  extern port_os_thread_t    pt_port_os_thread_get_current();
  extern void                v_port_os_thread_yield();
  extern void                v_port_os_thread_abort(port_os_thread_t thread);
  extern void                v_port_os_thread_resume(port_os_thread_t thread);
  extern void                v_port_os_thread_suspend(port_os_thread_t thread);
  extern void                v_port_os_thread_set_priority(port_os_thread_t thread, uint8_t priority);
  extern uint8_t             us_port_os_thread_get_priority(port_os_thread_t thread);
  extern void                v_port_os_thread_set_tls_pointer(port_os_thread_t thread, uint8_t pos, void* tls_ptr);
  extern void*               pv_port_os_thread_get_tls_pointer(port_os_thread_t thread, uint8_t pos);
  extern void                v_port_os_thread_delete(port_os_thread_t thread);
  extern void                v_port_os_delay_ms(uint32_t ms);
  extern void                v_port_os_delay_s(uint32_t s);
  extern port_os_timer_t     pt_port_os_timer_create(const char* const name, const uint32_t period_ms, const bool auto_reload, port_os_timer_callback_t callback, void* arg);
  extern void*               pv_port_os_timer_get_arg(port_os_timer_t timer);
  extern bool                b_port_os_timer_start(port_os_timer_t timer, const uint32_t period_ms);
  extern bool                b_port_os_timer_stop(port_os_timer_t timer);
  extern bool                b_port_os_timer_delete(port_os_timer_t timer);
  extern port_os_mutex_t     pt_port_os_mutex_create();
  extern bool                b_port_os_mutex_wait(port_os_mutex_t mutex, uint32_t timeout_ms);
  extern bool                b_port_os_mutex_release(port_os_mutex_t mutex);
  extern port_os_thread_t    pt_port_os_get_mutex_holder(port_os_mutex_t mutex);
  extern void                v_port_os_mutex_delete(port_os_mutex_t mutex);
  extern port_os_semaphore_t pt_port_os_semaphore_create(const uint32_t max_count, const uint32_t initial_count);
  extern bool                b_port_os_semaphore_take(port_os_semaphore_t sem, uint32_t timeout_ms);
  extern bool                b_port_os_semaphore_give(port_os_semaphore_t sem);
  extern uint32_t            ul_port_os_get_semaphore_count(port_os_semaphore_t sem);
  extern void                v_port_os_semaphore_delete(port_os_semaphore_t sem);
  extern port_os_message_t   pt_port_os_message_create(const uint32_t size, const uint32_t item_size);
  extern bool                b_port_os_message_send(port_os_message_t message, const void* data, uint32_t timeout_ms);
  extern bool                b_port_os_message_receive(port_os_message_t message, void* data, uint32_t timeout_ms);
  extern uint32_t            ul_port_os_message_available(port_os_message_t message);
  extern uint32_t            ul_port_os_message_spaces_available(port_os_message_t message);
  extern void                v_port_os_message_reset(port_os_message_t message);
  extern bool                b_port_os_message_peek(port_os_message_t message, void* data, uint32_t timeout_ms);
  extern void                v_port_os_message_delete(port_os_message_t message);
  extern port_os_stream_t    pt_port_os_stream_create(uint32_t size);
  extern uint32_t            ul_port_os_stream_send(port_os_stream_t stream, const void* data, uint32_t size);
  extern uint32_t            ul_port_os_stream_receive(port_os_stream_t stream, void* data, uint32_t size);
  extern uint32_t            ul_port_os_stream_available(port_os_stream_t stream);
  extern uint32_t            ul_port_os_stream_spaces_available(port_os_stream_t stream);
  extern bool                b_port_os_stream_is_empty(port_os_stream_t stream);
  extern bool                b_port_os_stream_is_full(port_os_stream_t stream);
  extern bool                b_port_os_stream_reset(port_os_stream_t stream);
  extern void                v_port_os_stream_delete(port_os_stream_t stream);
  extern port_os_event_t     pt_port_os_event_create();
  extern uint32_t            ul_port_os_event_set(port_os_event_t event, uint32_t event_bit);
  extern uint32_t            ul_port_os_event_clear(port_os_event_t event, uint32_t event_bit);
  extern uint32_t            ul_port_os_event_wait(port_os_event_t event, uint32_t event_bit, uint32_t timeout_ms, bool is_wait_all);
  extern void                v_port_os_event_delete(port_os_event_t event);

#if __cplusplus
}
#endif

#endif /* __PORT_OS_H__ */
