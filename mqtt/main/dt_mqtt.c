/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_mqtt.c
 * function: mqtt demo source code
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
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>
#include "mqtt_client.h"

#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"

#include "dt_gpio.h"
#include "dt_mqtt.h"

#define TAG "DT-MQTT"

dt_mqtt_t g_mqtt;

void dt_mqtt_set_hw_r(u8_t r, b8_t on)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    mkbit(mqtt->hw_r_bits, r, !!on);
}

b8_t dt_mqtt_get_hw_r(u8_t r)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    return (!!isset(mqtt->hw_r_bits, r));
}

void dt_mqtt_set_hw_i(u8_t i, b8_t on)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    mkbit(mqtt->hw_i_bits, i, !!on);
}

b8_t dt_mqtt_get_hw_i(u8_t i)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    return (!!isset(mqtt->hw_i_bits, i));
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    dt_mqtt_t *mqtt = &g_mqtt;
//    esp_mqtt_client_handle_t client = event->client;
//    int msg_id;
//    u8 *data = NULL;
    char *str;
    u32_t u32_val, u32_idx;
    u8 i;
//    char topic_tmp[MQTT_IN_TOPIC_LEN] = {0};

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        {
            DT_XLOGW("MQTT_EVENT_CONNECTED");
            mqtt->connected = TRUE;

            memset(mqtt->sp_bit, 0, MQTT_SP_BIT2BYTE);
            setbit(mqtt->sp_bit, MQTT_PUB_LWT_AVAILABILITY);
            for(i=0; i<DT_R_CNT; i++)
            {
                setbit(mqtt->sp_bit, MQTT_SUB_R_START+i);
            }
            for(i=0; i<DT_R_CNT; i++)
            {
                setbit(mqtt->sp_bit, MQTT_PUB_R_START+i);
            }
            for(i=0; i<DT_I_CNT; i++)
            {
                setbit(mqtt->sp_bit, MQTT_PUB_I_START+i);
            }
            setbit(mqtt->sp_bit, MQTT_PUB_INPUT_CNT);
            setbit(mqtt->sp_bit, MQTT_PUB_RELAY_CNT);
        }
            break;
        case MQTT_EVENT_DISCONNECTED:
        {
            DT_XLOGW("MQTT_EVENT_DISCONNECTED");
            mqtt->connected = FALSE;
        }
            break;

        case MQTT_EVENT_SUBSCRIBED:
            DT_XLOGW("MQTT_EVENT_SUBSCRIBED, msg_id=%d %x", event->msg_id, (u32)event->topic);/* event->topic==NULL,导致崩溃 */
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            DT_XLOGI("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            DT_LOGI("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
        {
            DT_XLOGI("MQTT_EVENT_DATA msg_id=%d", event->msg_id);
            DT_XLOGI("TOPIC=%.*s", event->topic_len, event->topic);
            DT_XLOGI("DATA=%.*s", event->data_len, event->data);
            //DT_LOGI("DATA=len:%d", event->data_len);

            sscanf(event->topic, MQTT_IN_R_TOPIC, &u32_idx);

            DT_XLOGI("MQTT_EVENT_DATA :MQTT_CTRL_R %d", u32_idx);
            u32_idx --;
            if( u32_idx >= DT_R_CNT )
            {
                DT_XLOGE("MQTT_EVENT_DATA %s overrange goto data_free", event->topic);
                break;
            }
            str = (char*)event->data;
            if( ('N' == str[1]) || ('n' == str[1]) || 
                ('T' == str[0]) || ('t' == str[0]) )
            {/* TRUE,true,ON,on */
                u32_val = TRUE;
            }else if( ('F' == str[1]) || ('f' == str[1]) || 
                      ('F' == str[0]) || ('f' == str[0]) )
            {/* FALSE,false,OFF,off */
                u32_val = FALSE;
            }else
            {
                DT_XLOGE("MQTT_EVENT_DATA :MQTT_CTRL_R unknow %s", str);
                break;
            }
            dt_mqtt_set_hw_r(u32_idx, u32_val);
            setbit(mqtt->sp_bit, MQTT_PUB_R_START+u32_idx);

            relay_w(u32_idx, u32_val);iox_sync();
        }
            break;
        case MQTT_EVENT_ERROR:
//        case MQTT_EVENT_ANY:
//        case MQTT_EVENT_BEFORE_CONNECT:
            DT_XLOGE("MQTT_EVENT_ERROR");
            break;
        default:
            DT_XLOGE("mqtt_event_handler %d", event->event_id);
            break;
    }
    return ESP_OK;
}


void mqtt_notify(void)
{
    dt_mqtt_t *mqtt = &g_mqtt;
    
    u32_t notify = 1;
    DT_XLOGW(">>>> %d", notify);
    xQueueSend(mqtt->mqtt_queue, &notify, 0);
}

b32 mqtt_check(void)
{
    dt_mqtt_t *mqtt = &g_mqtt;
//    esp_mqtt_client_handle_t client = mqtt->client;
    s32 i;
    u32 relay_cnt = DT_R_CNT;
    u32 input_cnt = DT_I_CNT;
    b32 ret = FALSE;

    if( FALSE == mqtt->connected )
    {
        DT_XLOGI(" mqtt connected -> FALSE ");
        return ret;
    }

    if( 0 != memcmp(mqtt->r_bits, mqtt->hw_r_bits, DT_R_BIT2BYTE) )
    {
        for(i=0; i<relay_cnt; i++)
        {
            if( (!!isset(mqtt->r_bits, i)) ^ (!!isset(mqtt->hw_r_bits, i)) )
            {
                setbit(mqtt->sp_bit, MQTT_PUB_R_START+i);

                ret = TRUE;
            }
        }
        memcpy(mqtt->r_bits, mqtt->hw_r_bits, DT_R_BIT2BYTE);
    }

    if( 0 != memcmp(mqtt->i_bits, mqtt->hw_i_bits, DT_I_BIT2BYTE) )
    {
        for(i=0; i<input_cnt; i++)
        {
            if( (!!isset(mqtt->i_bits, i)) ^ (!!isset(mqtt->hw_i_bits, i)) )
            {
                setbit(mqtt->sp_bit, MQTT_PUB_I_START+i);

                ret = TRUE;
            }
        }
        memcpy(mqtt->i_bits, mqtt->hw_i_bits, DT_I_BIT2BYTE);
    }

    for(i=0; i<MQTT_SP_BIT2BYTE; i++)
    {
        if( 0 != mqtt->sp_bit[i] )
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

void mqtt_do(void)
{
    dt_mqtt_t *mqtt = &g_mqtt;
    esp_mqtt_client_handle_t client = mqtt->client;
    u16 i, tmp;
    char topic[MQTT_OUT_TOPIC_LEN];
    char pub_payload[MQTT_OUT_TOPIC_LEN];
//    char datetime[32] = {0};
    b8 go_on = FALSE;
    s32 msg_id;
    u32 relay_cnt = DT_R_CNT;
    u32 input_cnt = DT_I_CNT;
//    s32_t ret;

    if( FALSE == mqtt->connected )
    {
        DT_XLOGD(" return ");
        return;
    }

    for(i=0; i<MQTT_SP_BIT2BYTE; i++)
    {
        if( 0 != mqtt->sp_bit[i] )
        {
            go_on = TRUE;
            break;
        }
    }
    if( FALSE == go_on )
    {
        return;
    }
////////////////////////
    DT_XLOGI(" start SP -------->>>>>>>>>>");
    if( isset(mqtt->sp_bit, MQTT_PUB_LWT_AVAILABILITY) )
    {
        DT_XLOGI(" LWT_AVAILABLITY");
        msg_id = esp_mqtt_client_publish(client, MQTT_OUT_LWT_AVAILABLITY_TOPIC, MQTT_ONLINE, strlen(MQTT_ONLINE), 1, 1);
        DT_LOGI("selectx_mqtt_proc publish LWT_AVAILABLITY=%s successful, msg_id=%d",  MQTT_ONLINE, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail LWT_AVAILABLITY,return ----");
            mqtt->try_times ++;
            go_on = FALSE;
        }else
        {
            mqtt->try_times = 0;
            clrbit(mqtt->sp_bit, MQTT_PUB_LWT_AVAILABILITY);
        }
    }
    if( FALSE == go_on )
    {
        goto error;
    }

    for(i=MQTT_SUB_R_START; i<MQTT_SUB_R_START+relay_cnt; i++)
    {
        if( isclr(mqtt->sp_bit, i) )
        {
            continue;
        }
        tmp = i-MQTT_SUB_R_START;
        DT_LOGI(" SUB-R%d ", tmp+1);
        sprintf(topic, MQTT_IN_R_TOPIC, tmp+1);
        msg_id = esp_mqtt_client_subscribe(client, topic, 1);
        DT_LOGI("selectx_mqtt_proc subscribe %s successful, msg_id=%d",  topic, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail SUB-R%d,return ----", tmp+1);
            mqtt->try_times ++;
            go_on = FALSE;
            break;
        }
        mqtt->try_times = 0;
        clrbit(mqtt->sp_bit, i);
    }
    if( FALSE == go_on )
    {
        goto error;
    }
////////////////////////
    for(i=MQTT_PUB_R_START; i<MQTT_PUB_R_START+relay_cnt; i++)
    {
        if( isclr(mqtt->sp_bit, i) )
        {
            continue;
        }
        tmp = i-MQTT_PUB_R_START;
        DT_LOGI(" PUB-R%d=%d", tmp+1, (!!isset(mqtt->hw_r_bits, tmp)));
        sprintf(topic, MQTT_OUT_R_TOPIC, tmp+1);
        sprintf(pub_payload, "%s", (!!isset(mqtt->hw_r_bits, tmp)) ? "ON":"OFF");
        DT_LOGD(" PUB-R%d=%d start", tmp+1, (!!isset(mqtt->hw_r_bits, tmp)));
        msg_id = esp_mqtt_client_publish(client, topic, pub_payload, strlen(pub_payload), 1, 1);
        DT_LOGI("selectx_mqtt_proc publish PUB-R%d=%s successful, msg_id=%d",  tmp+1, pub_payload, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail PUB-R%d,return ----", tmp+1);
            mqtt->try_times ++;
            go_on = FALSE;
            break;
        }
        mqtt->try_times = 0;
        clrbit(mqtt->sp_bit, i);
    }
    if( FALSE == go_on )
    {
        goto error;
    }

    for(i=MQTT_PUB_I_START; i<MQTT_PUB_I_START+input_cnt; i++)
    {
        if( isclr(mqtt->sp_bit, i) )
        {
            continue;
        }
        tmp = i-MQTT_PUB_I_START;
        DT_XLOGI(" PUB-I%d=%d", tmp+1, (!!isset(mqtt->hw_i_bits, tmp)));
        sprintf(topic, MQTT_OUT_I_TOPIC, tmp+1);
        sprintf(pub_payload, "%s", (!!isset(mqtt->hw_i_bits, tmp)) ? "OFF":"ON");
        msg_id = esp_mqtt_client_publish(client, topic, pub_payload, strlen(pub_payload), 1, 1);
        DT_LOGI("selectx_mqtt_proc publish PUB-I%d=%s successful, msg_id=%d",  tmp+1, pub_payload, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail PUB-I%d,return ----", tmp+1);
            mqtt->try_times ++;
            go_on = FALSE;
            break;
        }
        mqtt->try_times = 0;
        clrbit(mqtt->sp_bit, i);
    }
    if( FALSE == go_on )
    {
        goto error;
    }

////////////
    if( isset(mqtt->sp_bit, MQTT_PUB_INPUT_CNT) )
    {
        DT_LOGI(" PUB-ICNT");
        sprintf(topic, MQTT_OUT_ICNT_TOPIC);
        sprintf(pub_payload, "%u", input_cnt);
        msg_id = esp_mqtt_client_publish(client, topic, pub_payload, strlen(pub_payload), 1, 1);
        DT_LOGI("selectx_mqtt_proc publish PUB-ICNT=%s successful, msg_id=%d",  pub_payload, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail PUB-ICNT,return ----");
            mqtt->try_times ++;
            go_on = FALSE;
        }else
        {
            mqtt->try_times = 0;
            clrbit(mqtt->sp_bit, MQTT_PUB_INPUT_CNT);
        }
    }
    if( FALSE == go_on )
    {
        goto error;
    }
////////////
    if( isset(mqtt->sp_bit, MQTT_PUB_RELAY_CNT) )
    {
        DT_LOGI(" PUB-RCNT");
        sprintf(topic, MQTT_OUT_RCNT_TOPIC);
        sprintf(pub_payload, "%u", relay_cnt);
        msg_id = esp_mqtt_client_publish(client, topic, pub_payload, strlen(pub_payload), 1, 1);
        DT_LOGI("selectx_mqtt_proc publish PUB-RCNT=%s successful, msg_id=%d",  pub_payload, msg_id);
        if( ESP_FAIL == msg_id )
        {
            DT_LOGE(" fail PUB-RCNT,return ----");
            mqtt->try_times ++;
            go_on = FALSE;
        }else
        {
            mqtt->try_times = 0;
            clrbit(mqtt->sp_bit, MQTT_PUB_RELAY_CNT);
        }
    }
    if( FALSE == go_on )
    {
        goto error;
    }

    memset(mqtt->sp_bit, 0x00, MQTT_SP_BIT2BYTE);
    DT_LOGI(" selectx_mqtt_proc success --------<<<<<<<<<");
    return;

error:
    if( mqtt->try_times >= MQTT_SP_RETRY_MAX )
    {
        DT_XLOGE("sub retry 3 times wait relay connect <<<<<<<<<-------- ");
        mqtt->connected = FALSE;
    }
}

static void mqtt_task(void* arg)
{
    dt_mqtt_t *mqtt = (dt_mqtt_t*)arg;
    u32_t notify;
    u32_t ret;

    DT_XLOGW(" start ");

    do{
        notify = U32MAX;
        ret = xQueueReceive(mqtt->mqtt_queue, &notify, HW_MS(200));
        if( TRUE == ret )
        {
            DT_XLOGW(" ret:%d notify:%d", ret, notify);
            mqtt_check();
        }
        mqtt_do();
    }while(1);

    DT_XLOGW(" end");

    vTaskDelete(NULL);
}

void mqtt_start(void)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.1.9:1883",
        .event_handle = mqtt_event_handler,
        .user_context = NULL,
        .client_id = "dingtian-relay",
        .username = "mqtt",
        .password = "123",
        .keepalive = 120,
        .lwt_topic = MQTT_OUT_LWT_AVAILABLITY_TOPIC,
        .lwt_msg = MQTT_OFFLINE,
        .lwt_qos = 1,
        .lwt_retain = 1,
    };

    DT_XLOGI(" >>>>>");

    mqtt->client = esp_mqtt_client_init(&mqtt_cfg);

//    esp_mqtt_client_register_event(mqtt->client, MQTT_EVENT_ANY, mqtt_event_handler, mqtt)

    esp_mqtt_client_start(mqtt->client);
}

void dt_mqtt_init(void)
{
    dt_mqtt_t *mqtt = &g_mqtt;

    mqtt->connected = FALSE;
    memset(mqtt->hw_i_bits, 0xff, DT_I_BIT2BYTE);
    memset(mqtt->hw_r_bits, 0, DT_I_BIT2BYTE);
    memset(mqtt->i_bits, 0, DT_I_BIT2BYTE);
    memset(mqtt->r_bits, 0, DT_I_BIT2BYTE);

    memset(mqtt->sp_bit, 0, MQTT_SP_BIT2BYTE);

    mqtt_start();

    mqtt->mqtt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(mqtt_task, "mqtt", 4096, mqtt, 10, NULL);
}


