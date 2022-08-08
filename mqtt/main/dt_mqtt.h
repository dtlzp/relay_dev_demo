/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_mqtt.h
 * function: mqtt demo source code
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

#include "mqtt_client.h"

#ifndef _DT_MQTT_H_
#define _DT_MQTT_H_


#define MQTT_CLIENTID_LEN   (32)

#define MQTT_IN_TOPIC_LEN   (48)
#define MQTT_OUT_TOPIC_LEN  (48)

#define MQTT_Manufacturer   "Dingtian"

#define MQTT_CLIENTID           "dingtian-relay"
#define MQTT_IN_R_TOPIC         "/dingtian/relay/in/r%d"
#define MQTT_OUT_I_TOPIC        "/dingtian/relay/out/i%d"
#define MQTT_OUT_R_TOPIC        "/dingtian/relay/out/r%d"
#define MQTT_OUT_I_TOPIC        "/dingtian/relay/out/i%d"
#define MQTT_OUT_ICNT_TOPIC     "/dingtian/relay/out/input_cnt"
#define MQTT_OUT_RCNT_TOPIC     "/dingtian/relay/out/relay_cnt"
#define MQTT_OUT_LWT_AVAILABLITY_TOPIC     "/dingtian/relay/out/lwt_availability"

#define MQTT_ONLINE  "online"
#define MQTT_OFFLINE "offline"

enum
{
    MQTT_SP_START,

    MQTT_SUB_R_START = MQTT_SP_START,
    MQTT_SUB_R_END=MQTT_SUB_R_START+DT_R_CNT,

    MQTT_PUB_R_START = MQTT_SUB_R_END,
    MQTT_PUB_R_END = MQTT_PUB_R_START + DT_R_CNT,

    MQTT_PUB_I_START = MQTT_PUB_R_END,
    MQTT_PUB_I_END = MQTT_PUB_I_START + DT_I_CNT,

    MQTT_PUB_INPUT_CNT = MQTT_PUB_I_END,
    MQTT_PUB_RELAY_CNT,

    MQTT_PUB_LWT_AVAILABILITY,

    MQTT_SP_END,
    MQTT_SP_CNT = MQTT_SP_END,
};

#define MQTT_SP_BIT2BYTE   BIT_ALIGN_BYTE(u32_t, MQTT_SP_CNT)

#define MQTT_SP_RETRY_MAX (3)

typedef struct _dt_mqtt_t
{
    esp_mqtt_client_handle_t client;
    u8_t r_bits[DT_R_BIT2BYTE];
    u8_t i_bits[DT_I_BIT2BYTE];

    u8_t hw_r_bits[DT_R_BIT2BYTE];
    u8_t hw_i_bits[DT_I_BIT2BYTE];
    u8 sp_bit[MQTT_SP_BIT2BYTE];

    xQueueHandle mqtt_queue;

    b32_t connected;
    s32_t try_times;
}dt_mqtt_t;

void mqtt_notify(void);

void dt_mqtt_init(void);

void dt_mqtt_set_hw_r(u8_t r, b8_t on);
b8_t dt_mqtt_get_hw_r(u8_t r);
void dt_mqtt_set_hw_i(u8_t i, b8_t on);
b8_t dt_mqtt_get_hw_i(u8_t i);

#endif /* end _DT_MQTT_H_ */

