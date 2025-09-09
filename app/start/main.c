#include "port_system.h"
#include "port_os.h"

#include "stm32f429xx.h"
#define APP_ADDRESS 0x8040000

int main(void)
{
  __set_PRIMASK(0);
  SCB->VTOR = APP_ADDRESS;
  __enable_irq();

  e_port_system_init();
  v_port_os_init(1024);
  v_port_os_start();
  return 0;
}