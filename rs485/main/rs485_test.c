


/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: rs485_test.c
 * function:RS485 demo source code
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"

#define TAG "rs485_test"

ddt_t g_dt = DDT_DEF_VALUE();

#define BUF_SIZE (128)

static QueueHandle_t rs485_queue;
static char g_rs485_buf[BUF_SIZE*2] = {0};
static void rs485_task(void* arg)
{
    u32_t n_ret;
    uart_event_t event;

    DT_XLOGW(" start ");

    do{
        n_ret = xQueueReceive(rs485_queue, (void *)&event, HW_MS(200));
        if( FALSE == n_ret )
        {
            continue;
        }
        switch (event.type)
        {
        case UART_DATA:
            DT_XLOGD("[UART DATA]: %d", event.size);
            n_ret = uart_read_bytes((uart_port_t)UART_NUM_0, g_rs485_buf, event.size, HW_MS(0));
            if( n_ret != event.size )
            {
                DT_XLOGE("[UART DATA]: uart_read_bytes fail, %d!=%d fifo_free", n_ret, event.size);
                break;
            }
            n_ret = uart_write_bytes((uart_port_t)UART_NUM_0, g_rs485_buf, n_ret);
            break;

        case UART_FIFO_OVF:
            DT_XLOGE("hw fifo overflow");
            uart_flush_input((uart_port_t)UART_NUM_0);
            xQueueReset(rs485_queue);
            break;

        case UART_BUFFER_FULL:
            DT_XLOGE("ring buffer full");
            uart_flush_input((uart_port_t)UART_NUM_0);
            xQueueReset(rs485_queue);
            break;

        case UART_PARITY_ERR:
            DT_XLOGE("uart parity error");
            break;

        case UART_FRAME_ERR:
            DT_XLOGE("uart frame error");
            break;

        case UART_BREAK:
            DT_XLOGD("uart break event");
            break;

        default:
            DT_XLOGE("uart event type: %d", event.type);
            break;
        }
    }while(1);

    DT_XLOGW(" end<<<<<<<<<<<<<<<<");

    vTaskDelete(NULL);
}

void app_main(void)
{
    s32_t ret;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };
    s32_t rs485_de = g_dt.rs485_de;

    DT_XLOGE("uartx_init >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    ret = uart_driver_install((uart_port_t)UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 8, &rs485_queue, 0);
    if( ESP_OK != ret )
    {
        DT_XLOGE("uartx_init uart_driver_install %d", ret);
    }
    ret = uart_param_config((uart_port_t)UART_NUM_0, &uart_config);
    if( ESP_OK != ret )
    {
        DT_XLOGE("uartx_init uart_param_config %d", ret);
    }
    ret = uart_set_pin(UART_NUM_0, GPIO_TXD0, GPIO_RXD0, rs485_de, UART_PIN_NO_CHANGE);
    if( ESP_OK != ret )
    {
        DT_XLOGE("uartx_init uart_set_pin %d", ret);
    }
    ret = uart_set_mode(UART_NUM_0, UART_MODE_RS485_HALF_DUPLEX);
    if( ESP_OK != ret )
    {
        DT_XLOGE("uartx_init uart_set_mode %d", ret);
    }

    xTaskCreate(rs485_task, "rs485_task", 4096, (void*)NULL, 10, NULL);
}


