/**
 * @file      class_thread.h
 * @author    Sea-Of-Quantum
 * @brief     Star library Base class for Thread (线程)
 * @version   version 1.0.0
 * @date      2025-02-08
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#ifndef __CLASS_THREAD_H__
#define __CLASS_THREAD_H__

#include "port_os.h"
#include "port_system.h"
#include "object.hpp"
#include "message_queue.hpp"
#include "event_flags.hpp"

/// @brief Thread 默认优先级
#define THREAD_DEF_PRIORITY   3
/// @brief Thread 默认栈大小
#define THREAD_DEF_STACK_SIZE 256
/// @brief Thread 默认队列大小
#define THREAD_DEF_QUEUE_SIZE 5
/// @brief Thread 默认等待时间
#define THREAD_DEF_WAIT_TIME  1000

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
/// @brief 名称空间 系统接口
namespace kernel
{
/// @brief 类 Thread (线程)
class Thread : public Object
{
  O_MEMORY
  O_OBJECT

protected:
  friend class Object;

  enum thread_memssage_type
  {
    slot_message,
    later_delete_message,
  };

  struct thread_memssage
  {
    thread_memssage_type type;
    Base_Signal*         signal;
    void*                memssage;
  };

private:
  bool                            m_running;
  bool                            m_exit_flag;
  int                             m_ret_code;
  uint16_t                        m_priority;
  uint32_t                        m_wait_time;
  Mutex                           m_start_semaphore;
  port_os_thread_t                m_thread_handle;
  Message_Queue<thread_memssage>* m_message;

private:
  using Object::move_to_thread;
  static void thread_entry(void* arg);

protected:
  bool         wait_event(uint32_t timeout = WAIT_FOREVER);
  void         event();
  void         wait_exit();
  int          exec();
  void         cleanup();
  virtual void event_loop() = 0;
  virtual void run()
  {
    exec();
  }
  void suspend()
  {
    v_port_os_thread_suspend(m_thread_handle);
  }
  void resume()
  {
    v_port_os_thread_resume(m_thread_handle);
  }
  void set_wait_time(uint32_t timeout = THREAD_DEF_WAIT_TIME)
  {
    m_wait_time = timeout;
  }

public:
  explicit Thread(const std::string& name, Object* parent = nullptr) : Object(name, parent)
  {
    m_wait_time     = THREAD_DEF_WAIT_TIME;
    m_running       = false;
    m_exit_flag     = false;
    m_thread_handle = nullptr;
    m_message       = nullptr;
  }

  virtual void start(uint8_t priority = THREAD_DEF_PRIORITY, uint16_t stack_size = THREAD_DEF_STACK_SIZE, uint32_t queue_size = THREAD_DEF_QUEUE_SIZE);

  void exit(int ret_code = 0)
  {
    m_exit_flag = true;
    m_ret_code  = ret_code;
  }

  int join()
  {
    while (!m_exit_flag)
    {
      v_port_os_thread_abort(m_thread_handle);
      msleep(500);
    }
    return m_ret_code;
  }

  int quit()
  {
    exit(m_ret_code);
    return join();
  }

  static Thread* current_thread()
  {
    return static_cast<Thread*>(pv_port_os_thread_get_tls_pointer(nullptr, 0));
  }

  static void sleep(uint32_t s)
  {
    v_port_os_delay_s(s);
  }

  static void msleep(uint32_t ms)
  {
    v_port_os_delay_ms(ms);
  }

  static void usleep(uint32_t us)
  {
    v_port_system_delay_us(us);
  }

  static void yield()
  {
    v_port_os_thread_yield();
  }

  bool is_running() const
  {
    return m_running;
  }

  bool is_finished() const
  {
    return m_exit_flag;
  }

  uint16_t priority() const
  {
    return m_priority;
  }

  void set_priority(uint16_t priority)
  {
    m_priority = priority;
    if (m_thread_handle)
      v_port_os_thread_set_priority(m_thread_handle, priority);
  }

  virtual ~Thread()
  {
    if (m_running)
      quit();

    if (m_message)
      delete m_message;
  }
};

} /* namespace kernel */
} /* namespace system */
} /* namespace OwO */

#endif /* __CLASS_THREAD_H__ */
