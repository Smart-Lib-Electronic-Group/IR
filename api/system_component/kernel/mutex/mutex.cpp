#include "mutex.hpp"
#include "thread.hpp"

using namespace OwO::system::kernel;

void Mutex::lock()
{
  Thread* current = Thread::current_thread();

  if (current == m_owner)
  {
    m_lock_counter++;
    return;
  }

  b_port_os_mutex_wait(m_mutex, WAIT_FOREVER);
  m_owner        = current;
  m_lock_counter = 1;
}

bool Mutex::try_lock(uint32_t timeout)
{
  Thread* current = Thread::current_thread();

  if (current == m_owner)
  {
    m_lock_counter++;
    return true;
  }

  if (b_port_os_mutex_wait(m_mutex, timeout))
  {
    m_owner        = current;
    m_lock_counter = 1;
    return true;
  }
  else
    return false;
}

Read_Write_Lock::Read_Write_Lock()
{
  m_mutex             = pt_port_os_mutex_create();
  m_write_wait_binary = pt_port_os_semaphore_create(1, 1);

  m_read_counter      = 0;
  m_write_owner       = nullptr;
  m_read_recursion    = 0;
  m_write_recursion   = 0;
}

bool Read_Write_Lock::lock_read(uint32_t timeout)
{
  if (m_write_owner == Thread::current_thread())
  {
    m_read_recursion++;
    return true;
  }

  if (false == b_port_os_mutex_wait(m_mutex, timeout))
    return false;

  if (++m_read_counter == 1)
  {
    if (false == b_port_os_semaphore_take(m_write_wait_binary, 0))
    {
      m_read_counter--;
      b_port_os_mutex_release(m_mutex);
      return false;
    }
  }

  b_port_os_mutex_release(m_mutex);
  return true;
}

bool Read_Write_Lock::lock_write(uint32_t timeout)
{
  if (m_write_owner == Thread::current_thread())
  {
    m_write_recursion++;
    return true;
  }

  if (false == b_port_os_semaphore_take(m_write_wait_binary, timeout))
    return false;

  m_write_owner     = Thread::current_thread();
  m_write_recursion = 1;
  return true;
}

void Read_Write_Lock::unlock()
{
  if (m_write_owner == Thread::current_thread())
  {
    if (--m_write_recursion == 0)
    {
      m_write_owner = nullptr;
      b_port_os_semaphore_give(m_write_wait_binary);
    }
  }
  else
  {
    b_port_os_mutex_wait(m_mutex, WAIT_FOREVER);
    if (--m_read_counter == 0)
    {
      b_port_os_semaphore_give(m_write_wait_binary);
    }
    b_port_os_mutex_release(m_mutex);
  }
}

Read_Write_Lock::~Read_Write_Lock()
{
  v_port_os_mutex_delete(m_mutex);
  v_port_os_semaphore_delete(m_write_wait_binary);
}
