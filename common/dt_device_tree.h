
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_device_tree.h
 * function: gpio define
 */
 
#ifndef _DT_DEVICE_TREE_H
#define _DT_DEVICE_TREE_H

#include "dt_type.h"
#include "dt_dbg.h"
#include "dt_macro.h"
#include "channel.h"

#define GPIO_MAX    (64)
#define HGPIO_MAX    (8)

#define GPIO_NULL   (U8MAX)

#define VGPIO_I_MAX (32)
#define VGPIO_O_MAX (32)

enum{
    DT_LOW,
    DT_HIGH,
};

/* UART0 */
#define GPIO_TXD0   1
#define GPIO_RXD0   3

/* RMII */
#define GPIO_RMII_RXD0  25
#define GPIO_RMII_RXD1  26
#define GPIO_RMII_CRS   27
#define GPIO_RMII_TXD1  22
#define GPIO_RMII_TXD0  19
#define GPIO_RMII_TXEN  21
#define GPIO_RMII_CLK   17
#define GPIO_RMII_MDC   23
#define GPIO_RMII_MDIO  18

/* CAN */
#define GPIO_CAN_RX   4
#define GPIO_CAN_TX   5

/* dingtian device tree(ddt)  */
typedef struct _ddt_t
{
    u8_t gpio_factory;
    u8_t factory_run_led_level;
    u8_t rs485_de;
    u8_t is_vpio;
    u8_t gpio_wiegand[HGPIO_MAX];
    u8_t gpio_input[HGPIO_MAX];
    u8_t gpio_output[HGPIO_MAX];

    u8_t gpio_factory_run_led;
    u8_t gpio_phyrst;

    u8_t gpio_595165_pl;
    u8_t gpio_595165_rck;
    u8_t gpio_595165_sdi;
    u8_t gpio_595165_q7;
    u8_t gpio_595165_clk;

    u8_t rtc_scl;
    u8_t rtc_sda;
}ddt_t;

#define HC595165_PL(x)     gpio_set_level(g_dt.gpio_595165_pl, (x))
#define HC595165_RCK(x)    gpio_set_level(g_dt.gpio_595165_rck, (x))
#define HC595165_CLK(x)    gpio_set_level(g_dt.gpio_595165_clk, (x))
#define HC595165_SDI(x)    gpio_set_level(g_dt.gpio_595165_sdi, (x))
#define HC595165_Q7()      gpio_get_level(g_dt.gpio_595165_q7)

#if (1 == DT_2CH)
#define DT_I_CNT   (2)
#define DT_R_CNT   DT_I_CNT
#define DDT_2I_2O_VALUE() \
{ \
    .gpio_factory = 34,\
    .rs485_de = 33,\
    .is_vpio = FALSE,\
    .gpio_wiegand= {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_input  = {36, 39, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_output = {16, 2,  GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_595165_pl     = GPIO_NULL,\
    .gpio_595165_rck    = GPIO_NULL,\
    .gpio_595165_sdi    = GPIO_NULL,\
    .gpio_595165_q7     = GPIO_NULL,\
    .gpio_595165_clk    = GPIO_NULL,\
    .gpio_factory_run_led = 32,\
    .factory_run_led_level = DT_LOW,\
    .gpio_phyrst = 0,\
    .rtc_scl = GPIO_NULL,\
    .rtc_sda = GPIO_NULL,\
}
#define DDT_DEF_VALUE()   DDT_2I_2O_VALUE()

#elif (1 == DT_4CH)
#define DT_I_CNT   (4)
#define DT_R_CNT   DT_I_CNT
#define DDT_4I_4O_VALUE() \
{ \
    .gpio_factory = 34,\
    .rs485_de = 13,\
    .is_vpio = FALSE,\
    .gpio_wiegand= {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_input  = {36, 39, 33, 35, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_output = {16, 2,  32, 12, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_595165_pl     = GPIO_NULL,\
    .gpio_595165_rck    = GPIO_NULL,\
    .gpio_595165_sdi    = GPIO_NULL,\
    .gpio_595165_q7     = GPIO_NULL,\
    .gpio_595165_clk    = GPIO_NULL,\
    .gpio_factory_run_led = 15,\
    .factory_run_led_level = DT_LOW,\
    .gpio_phyrst = 0,\
    .rtc_scl = GPIO_NULL,\
    .rtc_sda = GPIO_NULL,\
}
#define DDT_DEF_VALUE()   DDT_4I_4O_VALUE()

#elif (1 == DT_8CH)
#define DT_I_CNT   (8)
#define DT_R_CNT   DT_I_CNT
#define DDT_8I_8O_VALUE() \
{ \
    .gpio_factory = 34,\
    .rs485_de = 33,\
    .is_vpio = TRUE,\
    .gpio_wiegand= {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_input  = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_output = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_595165_pl     = 32,\
    .gpio_595165_rck    = 15,\
    .gpio_595165_sdi    = 13,\
    .gpio_595165_q7     = 16,\
    .gpio_595165_clk    = 14,\
    .gpio_factory_run_led = 2,\
    .factory_run_led_level = DT_HIGH,\
    .gpio_phyrst = 0,\
    .rtc_scl = GPIO_NULL,\
    .rtc_sda = GPIO_NULL,\
}
#define DDT_DEF_VALUE()   DDT_8I_8O_VALUE()

#elif (1 == DT_16CH)
#define DT_I_CNT   (16)
#define DT_R_CNT   DT_I_CNT
#define DDT_16I_16O_VALUE() \
{ \
    .gpio_factory = 34,\
    .rs485_de = 33,\
    .is_vpio = TRUE,\
    .gpio_wiegand= {36, 39, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_input  = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_output = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_595165_pl     = 0,\
    .gpio_595165_rck    = 15,\
    .gpio_595165_sdi    = 13,\
    .gpio_595165_q7     = 35,\
    .gpio_595165_clk    = 14,\
    .gpio_factory_run_led = 2,\
    .factory_run_led_level = DT_HIGH,\
    .gpio_phyrst = 12,\
    .rtc_scl = 32,\
    .rtc_sda = 16,\
}
#define DDT_DEF_VALUE()   DDT_16I_16O_VALUE()

#elif (1 == DT_32CH)
#define DT_I_CNT   (32)
#define DT_R_CNT   DT_I_CNT
#define DDT_32I_32O_VALUE() \
{ \
    .gpio_factory = 34,\
    .rs485_de = 33,\
    .is_vpio = TRUE,\
    .gpio_wiegand= {36, 39, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_input  = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_output = {GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL, GPIO_NULL},\
    .gpio_595165_pl     = 0,\
    .gpio_595165_rck    = 15,\
    .gpio_595165_sdi    = 13,\
    .gpio_595165_q7     = 35,\
    .gpio_595165_clk    = 14,\
    .gpio_factory_run_led = 2,\
    .factory_run_led_level = DT_HIGH,\
    .gpio_phyrst = 12,\
    .rtc_scl = 32,\
    .rtc_sda = 16,\
}
#define DDT_DEF_VALUE()   DDT_32I_32O_VALUE()

#else
#error "please define channel DT_2CH/DT_4CH/DT_8CH/DT_16CH/DT_32CH"
#endif

#define DT_I_BIT2BYTE  BIT_ALIGN_BYTE(u32_t, DT_I_CNT)
#define DT_R_BIT2BYTE  BIT_ALIGN_BYTE(u32_t, DT_R_CNT)

#endif /* end _DT_DEVICE_TREE_H */

