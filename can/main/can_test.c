
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: can_test.c
 * function: CAN bus demo source code
 */
 
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"

#define TAG "DT-CANX"

static void can_task(void* arg)
{
    s32 n_ret;
    twai_message_t can_msg;

    DT_XLOGW(" start >>>>>>>>>>>>>>>>>>>> ");

    do{
        n_ret = twai_receive(&can_msg, HW_MS(200));
        if( ESP_OK == n_ret )
        {
            DT_XLOGW("Recv ID:%d ext:%d rtr:%d", can_msg.identifier, can_msg.extd, can_msg.rtr);
            DT_LOGI_BUFFER_HEX(can_msg.data, can_msg.data_length_code);

            n_ret = twai_transmit(&can_msg, pdMS_TO_TICKS(100));
            if( ESP_OK != n_ret )
            {
                DT_XLOGE("twai_transmit %d:%d", n_ret, can_msg.data_length_code);
            }
        }
    }while(1);

    DT_XLOGW(" end<<<<<<<<<<<<<<<<");

    vTaskDelete(NULL);
}

#define CAN_ID  1
#define CAN_EXT 1
void app_main(void)
{
    s32_t ret;
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_CAN_TX, GPIO_CAN_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t speed = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t f_config = {.acceptance_code = 0,
                                     .acceptance_mask = 0,
                                     .single_filter = true};

    f_config.acceptance_code = (CAN_ID << 21);
    f_config.acceptance_mask = ~(TWAI_STD_ID_MASK << 21);
    if( 1 == CAN_EXT )
    {
        f_config.acceptance_code = (CAN_ID << 3);
        f_config.acceptance_mask = ~(TWAI_EXTD_ID_MASK << 3);
    }
    DT_LOGE(" >>>>>>>>>>>>>>>>>>>> %d:%x", CAN_EXT, CAN_ID);

    ret = twai_driver_install(&g_config, &speed, &f_config);
    if( ESP_OK != ret )
    {
        DT_XLOGE(" twai_driver_install:%d failed", ret);
        return;
    }
    ret = twai_start();
    if( ESP_OK != ret )
    {
        DT_XLOGE(" twai_start:%d failed", ret);
        return;
    }

    xTaskCreate(can_task, "can_task", 4096, (void*)NULL, 10, NULL);
}
