
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: wiegand_test.c
 * function: wiegand26 demo source code
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"

#define TAG "wiegand_test"

ddt_t g_dt = DDT_DEF_VALUE();

enum
{
    WG_D0,
    WG_D1,

    WG_DE,
    WG_Dx_NULL = U8MAX,
};

#define WG26_BITS       (26)

#define WG_BIT2BYTE4 BIT_ALIGN_BYTE(u32_t, 32)
typedef struct _dt_wiegand_t{
    u8_t data[8];
    u8_t data_bits;

    u8_t bits[WG_BIT2BYTE4];
    u8_t cnt;
    u8 io_dx[GPIO_MAX];
    u8 io_value[GPIO_MAX];

    xQueueHandle queue;

    u32_t tick;
    b8_t ok;
}dt_wiegand_t;

dt_wiegand_t g_wg;

s32_t get_card(u8_t data[], u8_t bit_cnt, u8_t bits[])
{
    s32_t i;
    u8_t data_bit_cnt;
    u8_t bit;

    data_bit_cnt = 0;
    for(i=(bit_cnt-2); i>=1; i--)
    {
        bit = !!isset(bits, i);
        mkbit(data, data_bit_cnt, bit);    data_bit_cnt ++;
    }

    return data_bit_cnt;
}

void wg_input(u8 dx)
{
    mkbit(g_wg.bits, g_wg.cnt, dx);
    g_wg.cnt ++;

    if( g_wg.cnt >= WG26_BITS )
    {
        g_wg.data_bits = get_card(g_wg.data, g_wg.cnt, g_wg.bits);
        if( g_wg.data_bits > 0 )
        {
            g_wg.ok = TRUE;
        }
        g_wg.cnt = 0;
        g_wg.tick = U32MAX;
    }
}

u32_t time_dec(u32_t now, u32_t before)
{
    u32_t tmp_now = now;
    u32_t tmp_before = before;
    u32_t result;

    if( tmp_now < tmp_before )
    {
        tmp_now += (U32MAX - tmp_before);
        tmp_before = 0;
    }
    result = (tmp_now - tmp_before);

    return result;
}
#define tick_ms()   (esp_timer_get_time()/MS2US)
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    u32_t io_num = (u32_t) arg;
    s32_t ret;

    ret = gpio_get_level(io_num);
    if( g_wg.io_value[io_num] == ret ){
        return;
    }
    g_wg.io_value[io_num] = ret;
    if( DT_LOW == ret ){
        g_wg.tick = tick_ms();
        wg_input(g_wg.io_dx[io_num]);
        io_num = 1;
        xQueueSendFromISR(g_wg.queue, &io_num, NULL);
    }
}

static void gpio_task(void* arg)
{
    u32_t ret_wg;
    u32_t ret;
    u32_t tim_cnt, tmp;

    DT_XLOGW(" start ");

    do{
        ret_wg = U32MAX;
        ret = xQueueReceive(g_wg.queue, &ret_wg, HW_MS(20));
        if( TRUE == ret )
        {
            ret = ret_wg;
        }
        if( TRUE == g_wg.ok )
        {
            DT_XLOGW("Recv Card:");
            DT_LOGI_BUFFER_HEX(g_wg.data, BIT_ALIGN_BYTE(u8_t, g_wg.data_bits));
            g_wg.ok = FALSE;
            memset(g_wg.data, 0, 8);
        }
        if( U32MAX == g_wg.tick )
        {
            continue;
        }
        tim_cnt = tick_ms();
        tmp = time_dec(tim_cnt, g_wg.tick);
        if( tmp > 40 )
        {
            DT_XLOGW(" wg recv timeout");
            g_wg.cnt = 0;
            g_wg.tick = U32MAX;
        }
    }while(1);

    DT_XLOGW(" end<<<<<<<<<<<<<<<<");

    vTaskDelete(NULL);
}

#define ESP_INTR_FLAG_DEFAULT 0
void app_main(void)
{
    dt_wiegand_t *wg = &g_wg;
    esp_err_t err;
    gpio_config_t io_conf = {0};
    int32_t i;
    uint64_t i_irq_pin_bit_mask = 0;
    s32_t end;
    u8_t gpio, bit, xpio_i;

    if( (GPIO_NULL == g_dt.gpio_wiegand[0]) || 
        (GPIO_NULL == g_dt.gpio_wiegand[1]) )
    {
        DT_XLOGI(" no wiegand exit");
        return;
    }

    memset(wg, 0, sizeof(dt_wiegand_t));
    g_wg.tick = U32MAX;
    g_wg.cnt = 0;
    g_wg.ok = FALSE;

    i_irq_pin_bit_mask = 0;

    /* wiegand init */
    DT_XLOGI(" wiegand input pin");

    /* D0 */
    gpio = g_dt.gpio_wiegand[0];
    i_irq_pin_bit_mask |= (1ull<<gpio);
    g_wg.io_dx[gpio]  = WG_D0;
    g_wg.io_value[gpio] = gpio_get_level(gpio);

    /* D1 */
    gpio = g_dt.gpio_wiegand[1];
    i_irq_pin_bit_mask |= (1ull<<gpio);
    g_wg.io_dx[gpio]  = WG_D1;
    g_wg.io_value[gpio] = gpio_get_level(gpio);

    DT_XLOGI(" irq pin config");
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.pin_bit_mask = i_irq_pin_bit_mask;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    for(i=0; i<2; i++)
    {
        gpio = g_dt.gpio_wiegand[i];
        if( GPIO_NULL != gpio )
        {
            gpio_isr_handler_add(gpio, gpio_isr_handler, (void*)gpio);
        }
    }

    g_wg.queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task, "wg_task", 4096, (void*)NULL, 10, NULL);
}

