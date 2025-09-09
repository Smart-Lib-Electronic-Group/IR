#ifndef __PORT_NET_INIT_H__
#define __PORT_NET_INIT_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdbool.h>
#include "error_handle.h"

  extern char*        pc_port_net_get_ip_address();
  extern char*        pc_port_net_get_netmask();
  extern char*        pc_port_net_get_gateway();
  extern void         v_port_net_link_up_callback();
  extern void         v_port_net_link_down_callback();
  extern bool         b_port_net_link_status();
  extern error_code_e e_port_net_init_arr(const uint8_t* ip, const uint8_t* mask, const uint8_t* gate_way);
  extern error_code_e e_port_net_init(const char* ip, const char* mask, const char* gateway);
  extern void         v_port_net_reset_address_arr(const uint8_t* ip, const uint8_t* mask, const uint8_t* gate_way);
  extern void         v_port_net_reset_address(const char* ip, const char* mask, const char* gate_way);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PORT_NET_INIT_H__ */
