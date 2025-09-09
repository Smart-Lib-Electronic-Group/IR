#include "port_net_init.h"
#include "port_os.h"
#include "port_eth.h"

#include "FreeRTOS_IP.h"

typedef struct PORT_NET_INFO_T
{
  uint8_t mac[6];
  uint8_t ip[4];
  uint8_t netmask[4];
  uint8_t gateway[4];
  uint8_t dns[4];
  uint8_t link_status;
} port_net_info_t;

static port_net_info_t*   s_pt_port_net_info;
static NetworkInterface_t s_t_port_net_interface = { 0 };
static NetworkEndPoint_t  s_t_port_net_end_point = { 0 };

char* pc_port_net_get_ip_address()
{
  static char ip_str[16];
  sprintf(ip_str, "%d.%d.%d.%d", s_pt_port_net_info->ip[0], s_pt_port_net_info->ip[1], s_pt_port_net_info->ip[2], s_pt_port_net_info->ip[3]);
  return ip_str;
}

char* pc_port_net_get_netmask()
{
  static char netmask_str[16];
  sprintf(netmask_str, "%d.%d.%d.%d", s_pt_port_net_info->netmask[0], s_pt_port_net_info->netmask[1], s_pt_port_net_info->netmask[2], s_pt_port_net_info->netmask[3]);
  return netmask_str;
}

char* pc_port_net_get_gateway()
{
  static char gateway_str[16];
  sprintf(gateway_str, "%d.%d.%d.%d", s_pt_port_net_info->gateway[0], s_pt_port_net_info->gateway[1], s_pt_port_net_info->gateway[2], s_pt_port_net_info->gateway[3]);
  return gateway_str;
}

static bool s_b_port_net_parse(const char* str, uint8_t result[4])
{
  if (NULL == str)
    return false;

  int octets[4];
  // 使用sscanf解析四个十进制数，分隔符为点
  int parsed = sscanf(str, "%d.%d.%d.%d", &octets[0], &octets[1], &octets[2], &octets[3]);

  // 确保成功解析四个部分
  if (parsed != 4)
    return false;

  // 检查每个部分是否在有效范围内
  for (int i = 0; i < 4; i++)
  {
    if (octets[i] < 0 || octets[i] > 255)
      return false;

    result[i] = (uint8_t)octets[i];   // 转换为uint8_t
  }

  return true;
}

static void s_v_port_net_load_mac_address(uint8_t addr[6])
{
  // 获取STM32的唯一ID的前48位作为MAC地址
  uint32_t stm32_id = *(volatile uint32_t*)(0x1FFF7A10 + 4);
  addr[0]           = 00;
  addr[1]           = (stm32_id >> 8) & 0XFF;
  addr[2]           = stm32_id & 0XFF;

  stm32_id          = *(volatile uint32_t*)(0x1FFF7A10);
  addr[3]           = (stm32_id >> 16) & 0XFF;
  addr[4]           = (stm32_id >> 8) & 0XFF;
  addr[5]           = stm32_id & 0XFF;
}

static void s_v_port_net_load_default(port_net_info_t* net_info)
{
  s_v_port_net_load_mac_address(net_info->mac);
  net_info->ip[0]      = 192;
  net_info->ip[1]      = 168;
  net_info->ip[2]      = 1;
  net_info->ip[3]      = 10;

  net_info->netmask[0] = 255;
  net_info->netmask[1] = 255;
  net_info->netmask[2] = 255;
  net_info->netmask[3] = 0;

  net_info->gateway[0] = 192;
  net_info->gateway[1] = 168;
  net_info->gateway[2] = 1;
  net_info->gateway[3] = 10;
}

__attribute__((weak)) void v_port_net_link_up_callback()
{
  (void)0;
}

__attribute__((weak)) void v_port_net_link_down_callback()
{
  (void)0;
}

bool b_port_net_link_status()
{
  return s_pt_port_net_info->link_status;
}

void v_port_net_deinit()
{
  if (s_pt_port_net_info)
  {
    Free(s_pt_port_net_info);
    s_pt_port_net_info = NULL;
  }
}

#include "port_include.h"
error_code_e e_port_net_init_arr(const uint8_t* ip, const uint8_t* mask, const uint8_t* gate_way)
{
  if (s_pt_port_net_info)
  {
    v_port_net_reset_address_arr(ip, mask, gate_way);
    return SUCESS;
  }

  s_pt_port_net_info = (port_net_info_t*)Malloc(sizeof(port_net_info_t));
  if (NULL == s_pt_port_net_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port eth info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(s_pt_port_net_info, 0, sizeof(port_net_info_t));

  if (ip == NULL || mask == NULL || gate_way == NULL)
    s_v_port_net_load_default(s_pt_port_net_info);
  else
  {
    for (int i = 0; i < 4; i++)
    {
      s_pt_port_net_info->ip[i]      = ip[i];
      s_pt_port_net_info->netmask[i] = mask[i];
      s_pt_port_net_info->gateway[i] = gate_way[i];
    }

    s_v_port_net_load_mac_address(s_pt_port_net_info->mac);
  }

  pxFillInterfaceDescriptor(0, &s_t_port_net_interface);
  FreeRTOS_FillEndPoint(&s_t_port_net_interface, &s_t_port_net_end_point, s_pt_port_net_info->ip, s_pt_port_net_info->netmask, s_pt_port_net_info->gateway, s_pt_port_net_info->dns, s_pt_port_net_info->mac);
  FreeRTOS_IPInit_Multi();

  v_port_eth_reset();

  return SUCESS;
}

error_code_e e_port_net_init(const char* ip, const char* mask, const char* gate_way)
{
  if (s_pt_port_net_info)
  {
    v_port_net_reset_address(ip, mask, gate_way);
    return SUCESS;
  }

  s_pt_port_net_info = (port_net_info_t*)Malloc(sizeof(port_net_info_t));
  if (NULL == s_pt_port_net_info)
  {
    g_e_error_code = MEMORY_ERROR;
    ERROR_HANDLE("port eth info Malloc failed!\n");
    return g_e_error_code;
  }

  memset(s_pt_port_net_info, 0, sizeof(port_net_info_t));

  if (ip == NULL || mask == NULL || gate_way == NULL)
    s_v_port_net_load_default(s_pt_port_net_info);
  else
  {
    if (!s_b_port_net_parse(ip, s_pt_port_net_info->ip))
    {
      g_e_error_code = NET_CONFIG_ERROR;
      ERROR_HANDLE("port eth ip config error!\n");
      return g_e_error_code;
    }

    if (!s_b_port_net_parse(mask, s_pt_port_net_info->netmask))
    {
      g_e_error_code = NET_CONFIG_ERROR;
      ERROR_HANDLE("port eth netmask config error!\n");
      return g_e_error_code;
    }

    if (!s_b_port_net_parse(gate_way, s_pt_port_net_info->gateway))
    {
      g_e_error_code = NET_CONFIG_ERROR;
      ERROR_HANDLE("port eth gateway config error!\n");
      return g_e_error_code;
    }

    s_v_port_net_load_mac_address(s_pt_port_net_info->mac);
  }

  pxFillInterfaceDescriptor(0, &s_t_port_net_interface);
  FreeRTOS_FillEndPoint(&s_t_port_net_interface, &s_t_port_net_end_point, s_pt_port_net_info->ip, s_pt_port_net_info->netmask, s_pt_port_net_info->gateway, s_pt_port_net_info->dns, s_pt_port_net_info->mac);
  FreeRTOS_IPInit_Multi();

  v_port_eth_reset();

  return SUCESS;
}

void v_port_net_reset_address_arr(const uint8_t* ip, const uint8_t* mask, const uint8_t* gate_way)
{
  s_pt_port_net_info->link_status = 0;
  v_port_net_link_down_callback();

  HAL_NVIC_DisableIRQ(ETH_IRQn);

  if (NULL != ip)
    for (int i = 0; i < 4; i++)
      s_pt_port_net_info->ip[i] = ip[i];

  if (NULL != mask)
    for (int i = 0; i < 4; i++)
      s_pt_port_net_info->netmask[i] = mask[i];

  if (NULL != gate_way)
    for (int i = 0; i < 4; i++)
      s_pt_port_net_info->gateway[i] = gate_way[i];

  uint32_t __ip       = FreeRTOS_inet_addr_quick(s_pt_port_net_info->ip[0], s_pt_port_net_info->ip[1], s_pt_port_net_info->ip[2], s_pt_port_net_info->ip[3]);
  uint32_t __mask     = FreeRTOS_inet_addr_quick(s_pt_port_net_info->netmask[0], s_pt_port_net_info->netmask[1], s_pt_port_net_info->netmask[2], s_pt_port_net_info->netmask[3]);
  uint32_t __gate_way = FreeRTOS_inet_addr_quick(s_pt_port_net_info->gateway[0], s_pt_port_net_info->gateway[1], s_pt_port_net_info->gateway[2], s_pt_port_net_info->gateway[3]);

  taskENTER_CRITICAL();

  FreeRTOS_SetIPAddress(__ip);
  FreeRTOS_SetNetmask(__mask);
  FreeRTOS_SetGatewayAddress(__gate_way);

  taskEXIT_CRITICAL();

  v_port_eth_reset();
  HAL_NVIC_EnableIRQ(ETH_IRQn);

  s_pt_port_net_info->link_status = 1;
  v_port_net_link_up_callback();
}

void v_port_net_reset_address(const char* ip, const char* mask, const char* gate_way)
{
  if (NULL != ip)
    s_b_port_net_parse(ip, s_pt_port_net_info->ip);

  if (NULL != mask)
    s_b_port_net_parse(mask, s_pt_port_net_info->netmask);

  if (NULL != gate_way)
    s_b_port_net_parse(gate_way, s_pt_port_net_info->gateway);

  uint32_t __ip       = FreeRTOS_inet_addr_quick(s_pt_port_net_info->ip[0], s_pt_port_net_info->ip[1], s_pt_port_net_info->ip[2], s_pt_port_net_info->ip[3]);
  uint32_t __mask     = FreeRTOS_inet_addr_quick(s_pt_port_net_info->netmask[0], s_pt_port_net_info->netmask[1], s_pt_port_net_info->netmask[2], s_pt_port_net_info->netmask[3]);
  uint32_t __gate_way = FreeRTOS_inet_addr_quick(s_pt_port_net_info->gateway[0], s_pt_port_net_info->gateway[1], s_pt_port_net_info->gateway[2], s_pt_port_net_info->gateway[3]);
  uint32_t __dns      = FreeRTOS_inet_addr_quick(s_pt_port_net_info->dns[0], s_pt_port_net_info->dns[1], s_pt_port_net_info->dns[2], s_pt_port_net_info->dns[3]);

  HAL_NVIC_DisableIRQ(ETH_IRQn);

  FreeRTOS_SetEndPointConfiguration(&__ip, &__mask, &__gate_way, &__dns, &s_t_port_net_end_point);

  HAL_NVIC_EnableIRQ(ETH_IRQn);

  v_port_eth_reset();
}

void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier) {}
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
  /* Network is up */
  if (eNetworkEvent == eNetworkUp)
  {
    s_pt_port_net_info->link_status = 1;
    v_port_net_link_up_callback();
  }
  /* Network is down */
  else if (eNetworkEvent == eNetworkDown)
  {
    s_pt_port_net_info->link_status = 0;
    v_port_net_link_down_callback();
  }
}

#include <stdlib.h>
#include <time.h>
uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort)
{
  (void)ulSourceAddress;
  (void)usSourcePort;
  (void)ulDestinationAddress;
  (void)usDestinationPort;
  srand((unsigned int)time(NULL));
  return rand();
}

BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber)
{
  srand((unsigned int)time(NULL));
  return rand();
}