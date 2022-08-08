
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_gpio.h
 * function: Input,Relay,Factory button/led macro define
 */
 
#ifndef _DT_GPIO_H_
#define _DT_GPIO_H_

#define VGPIO_ENTER_CRITICAL()  portENTER_CRITICAL(&g_io.gpio_spinlock)
#define VGPIO_EXIT_CRITICAL()   portEXIT_CRITICAL(&g_io.gpio_spinlock)

#define VGPIO_REFRESH(igpio, ogpio, i_bytes, o_bytes) \
do \
{ \
    VGPIO_ENTER_CRITICAL(); \
    vpio_refresh((igpio), (ogpio), (i_bytes), (o_bytes)); \
    HC595165_PL(DT_LOW);\
    VGPIO_EXIT_CRITICAL(); \
}while(0)

enum
{
    GPIO_START,

    GPIO_I_START = GPIO_START,
    GPIO_I_END = GPIO_I_START+(VGPIO_I_MAX-1),

    GPIO_O_START,
    GPIO_O_END = GPIO_O_START+(VGPIO_I_MAX-1),

    GPIO_I_FACTORY,

    GPIO_END,
    GPIO_CNT = GPIO_END,
};
#define GPIO_CNT_ALIGN4  BYTE_ALIGN(sizeof(u32_t), GPIO_CNT)
#define GPIO_BIT2BYTE4   BIT_ALIGN_BYTE(u32_t, GPIO_CNT)

#define GPIO_I_BIT2BYTE4   BIT_ALIGN_BYTE(u32_t, VGPIO_I_MAX)

#define VPIO_I_BYTE(_i_cnt_)  (((_i_cnt_)+7)/8)
#define VPIO_O_BYTE(_o_cnt_)  VPIO_I_BYTE(_o_cnt_)
#define VPIO_IB8    VPIO_I_BYTE(8)
#define VPIO_OB8    VPIO_O_BYTE(8)
#define VPIO_IB16   VPIO_I_BYTE(16)
#define VPIO_OB16   VPIO_O_BYTE(16)
#define VPIO_IB24   VPIO_I_BYTE(24)
#define VPIO_OB24   VPIO_O_BYTE(24)
#define VPIO_IB32   VPIO_I_BYTE(32)
#define VPIO_OB32   VPIO_O_BYTE(32)

#define VPIO_I_BYTE_START   BIT_ALIGN_BYTE(u8_t, GPIO_I_START)
#define VPIO_O_BYTE_START   BIT_ALIGN_BYTE(u8_t, GPIO_O_START)

#define VPIO_I_BIT2BYTE    BIT_ALIGN_BYTE(u8_t, VGPIO_I_MAX)
#define VPIO_O_BIT2BYTE    BIT_ALIGN_BYTE(u8_t, VGPIO_O_MAX)

#define GPIO_I_BIT2BYTE    BIT_ALIGN_BYTE(u8_t, DT_I_CNT)
#define GPIO_O_BIT2BYTE    BIT_ALIGN_BYTE(u8_t, DT_R_CNT)

#define INPUT_DEBOUNCE_MS   100
#define FACTORY_TIMEOUT     INPUT_DEBOUNCE_MS
typedef struct _dt_io_t
{
    u8_t xpio_map[GPIO_CNT_ALIGN4];

    u8_t t_xpio_bits[GPIO_BIT2BYTE4];
    u8_t xpio_bits[GPIO_BIT2BYTE4];

    u32_t input_debounce_ms[VGPIO_I_MAX];
    u8 at_debounce[GPIO_I_BIT2BYTE4];

    xQueueHandle gpio_queue;
    portMUX_TYPE gpio_spinlock;

    b8_t started;
    u8_t factory_btn;
    u8_t factory_led;
    u8_t res[1];
    u32 factory_press_ms;
}dt_io_t;

enum{
    QUEUE_GPIO_INPUT_START = 0,
    QUEUE_GPIO_INPUT_END = QUEUE_GPIO_INPUT_START + (GPIO_MAX-1),
    QUEUE_VPIO_INPUT_START,
    QUEUE_VPIO_INPUT_END = QUEUE_VPIO_INPUT_START + (VGPIO_I_MAX-1),
    QUEUE_VPIO_OUTPUT_START,
    QUEUE_VPIO_OUTPUT_END = QUEUE_VPIO_OUTPUT_START + (VGPIO_O_MAX-1),

    QUEUE_GPIO_SYNC,
    QUEUE_GPIO_START,
    QUEUE_GPIO_EXIT,
};

b8_t relay_r(u8_t i);
void relay_w(u8_t i, b8_t on);
void iox_sync(void);
void iox_start(void);
void iox_exit(void);

#define ESP_INTR_FLAG_DEFAULT 0
void dt_gpio_init(void);

#endif /* end _DT_GPIO_H_ */

