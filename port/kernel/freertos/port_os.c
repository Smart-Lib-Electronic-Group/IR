#include "port_os.h"
#include "error_handle.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "stream_buffer.h"
#include "event_groups.h"
#include "cmsis_gcc.h"

/* ------------------------------------------------ ISR 中断 ------------------------------------------------ */

static inline bool sl_b_port_os_is_in_isr(void)
{
  return __get_IPSR() != 0;
}

/* ------------------------------------------------ Core 核心 ------------------------------------------------ */

extern void start_app(void* arg);
extern void v_port_os_memory_init(void);
void        v_port_os_init(uint32_t start_thread_stack_size)
{
  v_port_os_memory_init();
  pt_port_os_thread_create("start_app", start_thread_stack_size, 1, start_app, NULL);
}

void v_port_os_start()
{
  vTaskStartScheduler();
}

extern void xPortSysTickHandler(void);
void        v_port_os_irq_function(void)
{
  /* FreeRTOS时钟源 */
#if (1 == INCLUDE_xTaskGetSchedulerState)
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) /* 系统已经运行 */
  {
#endif /* INCLUDE_xTaskGetSchedulerState */
    xPortSysTickHandler();
#if (1 == INCLUDE_xTaskGetSchedulerState)
  }
#endif /* INCLUDE_xTaskGetSchedulerState */
}

uint32_t ul_port_os_get_tick_count()
{
  if (sl_b_port_os_is_in_isr())
    return xTaskGetTickCountFromISR();
  else
    return xTaskGetTickCount();
}

/* ------------------------------------------------ Critical 临界区 ------------------------------------------------ */

void v_port_os_enter_critical()
{
  taskENTER_CRITICAL();
}

void v_port_os_exit_critical()
{
  taskEXIT_CRITICAL();
}

/* ------------------------------------------------ Error 异常处理 ------------------------------------------------ */

void vApplicationMallocFailedHook(void)
{
  g_e_error_code = MEMORY_ERROR;
  ERROR_HANDLE("port os malloc failed error!\n");
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
  g_e_error_code = STACK_OVERFLOW_ERROR;
  ERROR_HANDLE("port os stack overflow error!\n");
}

/* ------------------------------------------------ Thread 线程 ------------------------------------------------ */

port_os_thread_t pt_port_os_thread_create(const char* const name, const uint32_t stack_depth, uint16_t priority, port_os_thread_function_t function, void* arg)
{
  TaskHandle_t task_handle;
  if (pdPASS != xTaskCreate(function, name, stack_depth, arg, priority, &task_handle))
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create thread error!\n");
    return NULL;
  }
  else
    return (port_os_thread_t)task_handle;
}

port_os_thread_t pt_port_os_thread_get_current()
{
  return (port_os_thread_t)xTaskGetCurrentTaskHandle();
}

void v_port_os_thread_yield()
{
  taskYIELD();
}

void v_port_os_thread_abort(port_os_thread_t thread)
{
  xTaskAbortDelay((TaskHandle_t)thread);
}

void v_port_os_thread_resume(port_os_thread_t thread)
{
  vTaskResume((TaskHandle_t)thread);
}

void v_port_os_thread_suspend(port_os_thread_t thread)
{
  vTaskSuspend((TaskHandle_t)thread);
}

void v_port_os_thread_set_priority(port_os_thread_t thread, uint8_t priority)
{
  vTaskPrioritySet((TaskHandle_t)thread, priority);
}

uint8_t us_port_os_thread_get_priority(port_os_thread_t thread)
{
  if (sl_b_port_os_is_in_isr())
    return uxTaskPriorityGetFromISR((TaskHandle_t)thread);
  else
    return uxTaskPriorityGet((TaskHandle_t)thread);
}

void v_port_os_thread_set_tls_pointer(port_os_thread_t thread, uint8_t pos, void* tls_ptr)
{
  vTaskSetThreadLocalStoragePointer((TaskHandle_t)thread, 0, tls_ptr);
}

void* pv_port_os_thread_get_tls_pointer(port_os_thread_t thread, uint8_t pos)
{
  return pvTaskGetThreadLocalStoragePointer((TaskHandle_t)thread, 0);
}

void v_port_os_thread_delete(port_os_thread_t thread)
{
  vTaskDelete((TaskHandle_t)thread);
}

/* ------------------------------------------------ Delay 延时 ------------------------------------------------ */

void v_port_os_delay_ms(uint32_t ms)
{
  if (sl_b_port_os_is_in_isr())
    return;
  else
  {
    if (ms > 0)
      vTaskDelay(pdMS_TO_TICKS(ms));
  }
}

void v_port_os_delay_s(uint32_t s)
{
  if (sl_b_port_os_is_in_isr())
    return;
  else
  {
    if (s > 0)
      vTaskDelay(pdMS_TO_TICKS(s * 1000));
  }
}

/* ------------------------------------------------ Timer 定时器 ------------------------------------------------ */

port_os_timer_t pt_port_os_timer_create(const char* const name, const uint32_t period_ms, const bool auto_reload, port_os_timer_callback_t callback, void* arg)
{
  TimerHandle_t timer_handle = xTimerCreate(name, period_ms, auto_reload, arg, (TimerCallbackFunction_t)callback);
  if (NULL == timer_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create timer error!\n");
    return NULL;
  }
  else
    return (port_os_timer_t)timer_handle;
}

void* pv_port_os_timer_get_arg(port_os_timer_t timer)
{
  if (NULL == timer)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os get timer arg error!\n");
    return NULL;
  }
  else
    return pvTimerGetTimerID((TimerHandle_t)timer);
}

bool b_port_os_timer_start(port_os_timer_t timer, const uint32_t period_ms)
{
  if (NULL == timer)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os start timer error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdPASS != xTimerChangePeriodFromISR((TimerHandle_t)timer, pdMS_TO_TICKS(period_ms), &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      return true;
    }
    else
      return xTimerStart((TimerHandle_t)timer, pdMS_TO_TICKS(period_ms));
  }
}

bool b_port_os_timer_stop(port_os_timer_t timer)
{
  if (NULL == timer)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os stop timer error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdPASS != xTimerStopFromISR((TimerHandle_t)timer, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
      return xTimerStop((TimerHandle_t)timer, 0U);
  }
}

bool b_port_os_timer_delete(port_os_timer_t timer)
{
  if (NULL == timer)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete timer error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
      return false;
    else
      return pdPASS == xTimerDelete((TimerHandle_t)timer, portMAX_DELAY);
  }
}

/* ------------------------------------------------ Mutex 互斥锁 ------------------------------------------------ */

port_os_mutex_t pt_port_os_mutex_create()
{
  SemaphoreHandle_t mutex_handle = xSemaphoreCreateMutex();
  if (NULL == mutex_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create mutex error!\n");
    return NULL;
  }
  else
    return (port_os_semaphore_t)mutex_handle;
}

bool b_port_os_mutex_wait(port_os_mutex_t mutex, uint32_t timeout_ms)
{
  if (NULL == mutex)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os take mutex error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xSemaphoreTakeFromISR((SemaphoreHandle_t)mutex, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
    {
      if (WAIT_FOREVER == timeout_ms)
        return pdTRUE == xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY);
      else
        return pdTRUE == xSemaphoreTake((SemaphoreHandle_t)mutex, pdMS_TO_TICKS(timeout_ms));
    }
  }
}

bool b_port_os_mutex_release(port_os_mutex_t mutex)
{
  if (NULL == mutex)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os give mutex error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xSemaphoreGiveFromISR((SemaphoreHandle_t)mutex, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
      return pdTRUE == xSemaphoreGive((SemaphoreHandle_t)mutex);
  }
}

port_os_thread_t pt_port_os_get_mutex_holder(port_os_mutex_t mutex)
{
  if (NULL == mutex)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os get mutex holder error!\n");
    return NULL;
  }
  else
    return (port_os_thread_t)xSemaphoreGetMutexHolder((SemaphoreHandle_t)mutex);
}

void v_port_os_mutex_delete(port_os_mutex_t mutex)
{
  if (NULL == mutex)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete mutex error!\n");
  }
  else
    vSemaphoreDelete((SemaphoreHandle_t)mutex);
}

/* ------------------------------------------------ Semaphore 信号量 ------------------------------------------------ */

port_os_semaphore_t pt_port_os_semaphore_create(const uint32_t max_count, const uint32_t initial_count)
{
  SemaphoreHandle_t sem_handle = NULL;
  if (max_count == 1)
    sem_handle = xSemaphoreCreateBinary();
  else
    sem_handle = xSemaphoreCreateCounting(max_count, initial_count);

  if (NULL == sem_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create semaphore error!\n");
    return NULL;
  }
  else
    return (port_os_semaphore_t)sem_handle;
}

bool b_port_os_semaphore_take(port_os_semaphore_t sem, uint32_t timeout_ms)
{
  if (NULL == sem)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os wait semaphore error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xSemaphoreTakeFromISR((SemaphoreHandle_t)sem, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
    {
      if (WAIT_FOREVER == timeout_ms)
        return pdTRUE == xSemaphoreTake((SemaphoreHandle_t)sem, portMAX_DELAY);
      else
        return pdTRUE == xSemaphoreTake((SemaphoreHandle_t)sem, pdMS_TO_TICKS(timeout_ms));
    }
  }
}

bool b_port_os_semaphore_give(port_os_semaphore_t sem)
{
  if (NULL == sem)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os post semaphore error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
      return pdTRUE == xSemaphoreGive((SemaphoreHandle_t)sem);
  }
}

uint32_t ul_port_os_get_semaphore_count(port_os_semaphore_t sem)
{
  if (NULL == sem)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os get semaphore count error!\n");
    return 0;
  }
  else
    return uxSemaphoreGetCount((SemaphoreHandle_t)sem);
}

void v_port_os_semaphore_delete(port_os_semaphore_t sem)
{
  if (NULL == sem)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete semaphore error!\n");
  }
  else
    vSemaphoreDelete((SemaphoreHandle_t)sem);
}

/* ------------------------------------------------ Message 消息队列 ------------------------------------------------ */

port_os_message_t pt_port_os_message_create(const uint32_t size, const uint32_t item_size)
{
  QueueHandle_t message_handle = xQueueCreate(size, item_size);
  if (NULL == message_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create message error!\n");
    return NULL;
  }
  else
    return (port_os_message_t)message_handle;
}

bool b_port_os_message_send(port_os_message_t message, const void* data, uint32_t timeout_ms)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os send message error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xQueueSendFromISR((QueueHandle_t)message, data, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
    {
      if (WAIT_FOREVER == timeout_ms)
        return pdTRUE == xQueueSend((QueueHandle_t)message, data, portMAX_DELAY);
      else
        return pdTRUE == xQueueSend((QueueHandle_t)message, data, pdMS_TO_TICKS(timeout_ms));
    }
  }
}

bool b_port_os_message_receive(port_os_message_t message, void* data, uint32_t timeout_ms)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os receive message error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xQueueReceiveFromISR((QueueHandle_t)message, data, &xHigherPriorityTaskWoken))
        return false;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return true;
    }
    else
    {
      if (WAIT_FOREVER == timeout_ms)
        return pdTRUE == xQueueReceive((QueueHandle_t)message, data, portMAX_DELAY);
      else
        return pdTRUE == xQueueReceive((QueueHandle_t)message, data, pdMS_TO_TICKS(timeout_ms));
    }
  }
}

uint32_t ul_port_os_message_available(port_os_message_t message)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os available message error!\n");
    return 0;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
      return uxQueueMessagesWaitingFromISR((QueueHandle_t)message);
    else
      return uxQueueMessagesWaiting((QueueHandle_t)message);
  }
}

uint32_t ul_port_os_message_spaces_available(port_os_message_t message)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os spaces available message error!\n");
    return 0;
  }
  else
    return uxQueueSpacesAvailable((QueueHandle_t)message);
}

void v_port_os_message_reset(port_os_message_t message)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os reset message error!\n");
  }
  else
    xQueueReset((QueueHandle_t)message);
}

bool b_port_os_message_peek(port_os_message_t message, void* data, uint32_t timeout_ms)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os peek message error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      if (pdTRUE != xQueuePeekFromISR((QueueHandle_t)message, data))
        return false;
      else
        return true;
    }
    else
    {
      if (WAIT_FOREVER == timeout_ms)
        return pdTRUE == xQueuePeek((QueueHandle_t)message, data, portMAX_DELAY);
      else
        return pdTRUE == xQueuePeek((QueueHandle_t)message, data, pdMS_TO_TICKS(timeout_ms));
    }
  }
}

void v_port_os_message_delete(port_os_message_t message)
{
  if (NULL == message)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete message error!\n");
  }
  else
    vQueueDelete((QueueHandle_t)message);
}

/* ------------------------------------------------ Stream 字节流 ------------------------------------------------ */

port_os_stream_t pt_port_os_stream_create(uint32_t size)
{
  StreamBufferHandle_t stream_buffer_handle = xStreamBufferCreate(size, 1);
  if (NULL == stream_buffer_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create stream error!\n");
    return NULL;
  }
  else
    return (port_os_stream_t)stream_buffer_handle;
}

uint32_t ul_port_os_stream_send(port_os_stream_t stream, const void* data, uint32_t size)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os send stream error!\n");
    return 0;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xStreamBufferSendFromISR((StreamBufferHandle_t)stream, data, size, &xHigherPriorityTaskWoken))
        return 0;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return size;
    }
    else
      return xStreamBufferSend((StreamBufferHandle_t)stream, data, size, 0);
  }
}

uint32_t ul_port_os_stream_receive(port_os_stream_t stream, void* data, uint32_t size)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os receive stream error!\n");
    return 0;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xStreamBufferReceiveFromISR((StreamBufferHandle_t)stream, data, size, &xHigherPriorityTaskWoken))
        return 0;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return size;
    }
    else
      return xStreamBufferReceive((StreamBufferHandle_t)stream, data, size, 0);
  }
}

uint32_t ul_port_os_stream_available(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os available stream error!\n");
    return 0;
  }
  else
    return xStreamBufferBytesAvailable((StreamBufferHandle_t)stream);
}

uint32_t ul_port_os_stream_spaces_available(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os spaces available stream error!\n");
    return 0;
  }
  else
    return xStreamBufferSpacesAvailable((StreamBufferHandle_t)stream);
}

bool b_port_os_stream_is_empty(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os is empty stream error!\n");
    return false;
  }
  else
    return xStreamBufferIsEmpty((StreamBufferHandle_t)stream);
}

bool b_port_os_stream_is_full(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os is full stream error!\n");
    return false;
  }
  else
    return xStreamBufferIsFull((StreamBufferHandle_t)stream);
}

bool b_port_os_stream_reset(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os reset stream error!\n");
    return false;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      if (pdTRUE != xStreamBufferResetFromISR((StreamBufferHandle_t)stream))
        return false;
      else
        return true;
    }
    else
      return xStreamBufferReset((StreamBufferHandle_t)stream) == pdPASS;
  }
}

void v_port_os_stream_delete(port_os_stream_t stream)
{
  if (NULL == stream)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete stream error!\n");
  }
  else
    vStreamBufferDelete((StreamBufferHandle_t)stream);
}

/* ------------------------------------------------ Event 事件 ------------------------------------------------ */

port_os_event_t pt_port_os_event_create()
{
  EventGroupHandle_t event_group_handle = xEventGroupCreate();
  if (NULL == event_group_handle)
  {
    g_e_error_code = INIT_ERROR;
    ERROR_HANDLE("port os create event error!\n");
    return NULL;
  }
  else
    return (port_os_event_t)event_group_handle;
}

uint32_t ul_port_os_event_set(port_os_event_t event, uint32_t event_bit)
{
  if (NULL == event)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os set event error!\n");
    return 0;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      if (pdTRUE != xEventGroupSetBitsFromISR((EventGroupHandle_t)event, event_bit, &xHigherPriorityTaskWoken))
        return 0;
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      ;
      return event_bit;
    }
    else
      return xEventGroupSetBits((EventGroupHandle_t)event, event_bit);
  }
}

uint32_t ul_port_os_event_clear(port_os_event_t event, uint32_t event_bit)
{
  if (NULL == event)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os clear event error!\n");
    return 0;
  }
  else
  {
    if (sl_b_port_os_is_in_isr())
    {
      if (pdTRUE != xEventGroupClearBitsFromISR((EventGroupHandle_t)event, event_bit))
        return 0;
      else
        return event_bit;
    }
    else
      return xEventGroupClearBits((EventGroupHandle_t)event, event_bit);
  }
}

uint32_t ul_port_os_event_wait(port_os_event_t event, uint32_t event_bit, uint32_t timeout_ms, bool is_wait_all)
{
  if (NULL == event)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os wait event error!\n");
    return 0;
  }
  else
    return xEventGroupWaitBits((EventGroupHandle_t)event, event_bit, pdFALSE, is_wait_all, pdMS_TO_TICKS(timeout_ms));
}

void v_port_os_event_delete(port_os_event_t event)
{
  if (NULL == event)
  {
    g_e_error_code = NO_INIT_ERROR;
    ERROR_HANDLE("port os delete event error!\n");
  }
  else
    vEventGroupDelete((EventGroupHandle_t)event);
}
