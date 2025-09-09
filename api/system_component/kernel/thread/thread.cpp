/**
 * @file      class_thread.cpp
 * @author    Sea-Of-Quantum
 * @brief     Star library Base class for Thread (线程)
 * @version   version 1.0.0
 * @date      2025-02-08
 *
 * @copyright Copyright (c) 2024 by Sea-Of-Quantum, All Rights Reserved.
 *
 */
#include "thread.hpp"
#include "signal.hpp"

using namespace OwO;
using namespace system;
using namespace kernel;

O_METAOBJECT(Thread, Object)

void Thread::start(uint8_t priority, uint16_t stack_size, uint32_t queue_size)
{
  if (m_running)
    return;

  {
    Atomic_Guard atomic;
    m_exit_flag = false;
  }

  if (queue_size)
    m_message = new Message_Queue<thread_memssage>(queue_size);

  m_priority      = priority;
  m_thread_handle = pt_port_os_thread_create(name().c_str(), stack_size, priority, thread_entry, this);
  m_start_semaphore.lock();
}

int Thread::exec()
{
  while (!m_exit_flag)
  {
    event_loop();
    if (wait_event(m_wait_time) && !m_exit_flag)
      event();
  }
  return m_ret_code;
}

void Thread::cleanup()
{
  event();
}

bool Thread::wait_event(uint32_t timeout)
{
  if (m_message)
  {
    thread_memssage message;
    return m_message->peek(message, timeout);
  }
  return false;
}

void Thread::event()
{
  if (m_message)
  {
    thread_memssage message;
    while (m_message->available())
    {
      if (m_message->receive(message, 0))
      {
        switch (message.type)
        {
          case slot_message :
            message_slot_handler(message.signal, message.memssage);
            break;
          case later_delete_message :
            message_later_delete_handler(message.memssage);
            break;
        }
      }
    }
  }
}

void Thread::wait_exit()
{
  m_start_semaphore.lock();
  m_start_semaphore.unlock();
}

void Thread::thread_entry(void* arg)
{
  Thread* thread = static_cast<Thread*>(arg);
  v_port_os_thread_set_tls_pointer(nullptr, 0, thread);
  thread->move_to_thread(thread);
  thread->m_start_semaphore.unlock();
  {
    Atomic_Guard atomic;
    thread->m_running = true;
  }
  thread->move_to_thread(thread);
  thread->run();
  thread->m_start_semaphore.lock();
  thread->cleanup();
  {
    Atomic_Guard atomic;
    thread->m_running = false;
  }
  v_port_os_thread_delete(nullptr);
  thread->m_start_semaphore.unlock();
}
