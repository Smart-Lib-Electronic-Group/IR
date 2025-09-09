#include "port_ui_init.h"
#include "port_gpio.h"
#include "port_spi.h"
#include "u8g2.h"

#define U8G2_PORT_SPI_PORT  1
#define U8G2_PORT_DC_PORT   'A'
#define U8G2_PORT_DC_PIN    4
#define U8G2_PORT_SCLK_PORT 'A'
#define U8G2_PORT_SCLK_PIN  5
#define U8G2_PORT_CS_PORT   'A'
#define U8G2_PORT_CS_PIN    6
#define U8G2_PORT_MOSI_PORT 'A'
#define U8G2_PORT_MOSI_PIN  7
#define U8G2_PORT_RESE_PORT 'B'
#define U8G2_PORT_RESE_PIN  1

static port_os_event_t s_u8g2_port_event_group;

static inline void sl_u8g2_port_gpio_init()
{
  v_port_gpio_init(U8G2_PORT_DC_PORT, U8G2_PORT_DC_PIN, PORT_GPIO_OUT_PP, PORT_GPIO_EVENT_BOTH, PORT_GPIO_PULL_UP, PORT_GPIO_SPEED_HIGH, NULL);
  v_port_gpio_init(U8G2_PORT_CS_PORT, U8G2_PORT_CS_PIN, PORT_GPIO_OUT_PP, PORT_GPIO_EVENT_BOTH, PORT_GPIO_PULL_UP, PORT_GPIO_SPEED_HIGH, NULL);
  v_port_gpio_init(U8G2_PORT_RESE_PORT, U8G2_PORT_RESE_PIN, PORT_GPIO_OUT_PP, PORT_GPIO_EVENT_BOTH, PORT_GPIO_PULL_UP, PORT_GPIO_SPEED_HIGH, NULL);

  v_port_gpio_write(U8G2_PORT_DC_PORT, U8G2_PORT_DC_PIN, 0);
  v_port_gpio_write(U8G2_PORT_CS_PORT, U8G2_PORT_CS_PIN, 1);
  v_port_gpio_write(U8G2_PORT_RESE_PORT, U8G2_PORT_RESE_PIN, 1);
}

static inline error_code_e sl_u8g2_port_spi_init(port_os_event_t event_group)
{
  return e_port_spi_init(U8G2_PORT_SPI_PORT, PORT_SPI_FULL_DUPLEX_MASTER, 8, 2, PORT_SPI_MSB, PORT_SPI_TX, PORT_SPI_DMA, PORT_SPI_DMA, event_group);
}

static inline error_code_e sl_u8g2_port_send(void* send_data, uint16_t send_length)
{
  return e_port_spi_send(U8G2_PORT_SPI_PORT, send_data, send_length);
}

static inline bool sl_u8g2_port_wait_send_complete(uint32_t wait_time)
{
  if (0 != (ul_port_os_event_wait(s_u8g2_port_event_group, SPI_SEND_CPLT_BIT, wait_time, false) & SPI_SEND_CPLT_BIT))
  {
    ul_port_os_event_clear(s_u8g2_port_event_group, SPI_SEND_CPLT_BIT);
    return true;
  }
  else
    return false;
}

static inline void sl_u8g2_port_cs_write(uint8_t val)
{
  v_port_gpio_write(U8G2_PORT_CS_PORT, U8G2_PORT_CS_PIN, val);
}

static inline void sl_u8g2_port_dc_write(uint8_t val)
{
  v_port_gpio_write(U8G2_PORT_DC_PORT, U8G2_PORT_DC_PIN, val);
}

static uint8_t s_u8g2_port_spi_message_callback(U8X8_UNUSED u8x8_t* u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void* arg_ptr)
{
  return 1;
}

static uint8_t s_u8g2_port_spi_event_callback(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr)
{
  switch (msg)
  {
    /* 初始化函数 */
    case U8X8_MSG_BYTE_INIT :
      s_u8g2_port_event_group = pt_port_os_event_create();
      sl_u8g2_port_spi_init(s_u8g2_port_event_group);
      sl_u8g2_port_gpio_init();
      break;

    /* 设置DC引脚，DC引脚控制发送的是数据还是命令 */
    case U8X8_MSG_BYTE_SET_DC :
      sl_u8g2_port_dc_write(arg_int);
      break;

    /* 开始传输前会进行的操作 */
    case U8X8_MSG_BYTE_START_TRANSFER :
      sl_u8g2_port_cs_write(0);
      break;

    /* 传输后进行的操作 */
    case U8X8_MSG_BYTE_END_TRANSFER :
      sl_u8g2_port_cs_write(1);
      break;

    /* 发送数据 */
    case U8X8_MSG_BYTE_SEND :
      sl_u8g2_port_send(arg_ptr, arg_int);
      sl_u8g2_port_wait_send_complete(WAIT_FOREVER);
      break;

    default :
      return 0;
  }

  return 1;
}

void v_u8g2_port_init(u8g2_t* u8g2)
{
  u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R0, s_u8g2_port_spi_event_callback, s_u8g2_port_spi_message_callback);
  u8g2_InitDisplay(u8g2);
  u8g2_SetPowerSave(u8g2, 0);
}

void v_demo(u8g2_t* u8g2)
{
  static uint8_t local = 1;
  u8g2_SetFontMode(u8g2, 1);
  u8g2_SetFontDirection(u8g2, 0);
  u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
  u8g2_DrawStr(u8g2, 0 + local, 20, "U");

  u8g2_SetFontDirection(u8g2, 1);
  u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
  u8g2_DrawStr(u8g2, 21 + local, 8, "8");

  u8g2_SetFontDirection(u8g2, 0);
  u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
  u8g2_DrawStr(u8g2, 51 + local, 30, "g");
  u8g2_DrawStr(u8g2, 67 + local, 30, "\xb2");

  u8g2_DrawHLine(u8g2, 2 + local, 35, 47);
  u8g2_DrawHLine(u8g2, 3 + local, 36, 47);
  u8g2_DrawVLine(u8g2, 45 + local, 32, 12);
  u8g2_DrawVLine(u8g2, 46 + local, 33, 12);

  u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
  u8g2_DrawStr(u8g2, 1 + local, 54, "github.com/olikraus/u8g2");
  local = (local + 1) % 128;
}