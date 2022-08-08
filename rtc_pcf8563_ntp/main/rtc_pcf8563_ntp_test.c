/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: rtc_pcf8563_ntp_test.c
 * function: rtc ntp demo source code
 */
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>

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
#include "esp_sntp.h"

#include <esp_http_server.h>

#include <pcf8563.h>

#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"


#define TAG "rtc_pcf8563_ntp_test"

ddt_t g_dt = DDT_DEF_VALUE();

//#define CONFIG_EXAMPLE_ETH_PHY_RTL8201  1 /* ethernet PHY RTL8201F */
//#define CONFIG_EXAMPLE_ETH_PHY_LAN8720  1 /* ethernet PHY LAN8720A */
#define CONFIG_EXAMPLE_ETH_PHY_JL1101   1   /* ethernet PHY JL1101 */

#define CONFIG_EXAMPLE_ETH_PHY_ADDR     (-1)
#define CONFIG_EXAMPLE_ETH_PHY_RST_GPIO g_dt.gpio_phyrst
#define CONFIG_EXAMPLE_ETH_MDC_GPIO     GPIO_RMII_MDC
#define CONFIG_EXAMPLE_ETH_MDIO_GPIO    GPIO_RMII_MDIO

#define ETH_STATIC_IP 1
//#define ETH_DHCP_IP 1

#define PCF8563_YEAR    1900

static i2c_dev_t g_pcf8563x_dev;

b8_t pcf8563x_set_time(struct tm *time)
{
    esp_err_t r;

    DT_XLOGI("Enter ");
    r = pcf8563_set_time(&g_pcf8563x_dev, time);
    if( ESP_OK == r )
    {
        r = TRUE;
    }else
    {
        DT_XLOGE("Error %d: %s", r, esp_err_to_name(r));
        r = FALSE;
    }
    DT_XLOGI("Exit ");

    return r;
}

b8_t pcf8563x_get_time(struct tm *time)
{
    esp_err_t r;
    bool valid;

    DT_XLOGI("Enter ");

    r = pcf8563_get_time(&g_pcf8563x_dev, time, &valid);
    if( ESP_OK == r )
    {
        DT_XLOGW("%04d/%02d/%02d %02d:%02d:%02d, %s", 
            time->tm_year + PCF8563_YEAR, time->tm_mon + 1, time->tm_mday, 
            time->tm_hour, time->tm_min, time->tm_sec, 
            valid ? "VALID" : "NOT VALID");
        if( TRUE != valid )
        {
            time->tm_year = 2020-PCF8563_YEAR;
            time->tm_mon = 0;/* 0~11*/
            time->tm_mday = 1;/* 1~31 */
            time->tm_hour = 0;
            time->tm_min = 0;
            time->tm_sec = 0;
        }
    }else
    {
        DT_XLOGE("Error %d: %s", r, esp_err_to_name(r));
        valid = FALSE;
    }

    DT_XLOGI("Exit ");

    return valid;
}

void pcf8563x_sync_time(struct tm *time)
{
    struct timeval tv;
    time_t t_sec;

    t_sec = mktime(time);
    tv.tv_sec = t_sec;
    tv.tv_usec = 0;

    DT_XLOGI(" %ld", tv.tv_sec);
    settimeofday(&tv, NULL);
}

void pcf8563x_cnys_time(time_t unix_stamp)
{
    b8_t ret;
    struct tm *t;

    DT_XLOGI("Enter ");

    t = gmtime(&unix_stamp);

    DT_XLOGW("%04d/%02d/%02d %02d:%02d:%02d", 
        t->tm_year+PCF8563_YEAR, t->tm_mon + 1, t->tm_mday, 
        t->tm_hour, t->tm_min, t->tm_sec);
    ret = pcf8563x_set_time(t);
    DT_XLOGI("Exit ret=%d ", ret);
}

void pcf8563x_init(void)
{
    struct tm time;
    gpio_num_t scl = g_dt.rtc_scl;
    gpio_num_t sda = g_dt.rtc_sda;

    DT_XLOGE(" PCF8563 SCL:%d SDA:%d ", scl, sda);

    memset(&g_pcf8563x_dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(i2cdev_init());

    if( ERR_OK != pcf8563_init_desc(&g_pcf8563x_dev, 0, sda, scl) )
    {
        return;
    }

    pcf8563x_get_time(&time);
    pcf8563x_sync_time(&time);
}

void sntpx_sync_notify(struct timeval *tv)
{
    DT_XLOGE(" time is ok %ld", tv->tv_sec);
    pcf8563x_cnys_time(tv->tv_sec);
}

void sntpx_start(char *sntp_server)
{
    DT_XLOGI(" %s", sntp_server);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, sntp_server);
    sntp_set_time_sync_notification_cb(sntpx_sync_notify);
    sntp_init();
}

void sntpx_stop(void)
{
    DT_LOGI("stop_sntp");

    sntp_stop();
}


esp_netif_t *g_eth_netif = NULL;
/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    esp_err_t err;
    uint8_t mac_addr[6] = {0};
    esp_netif_ip_info_t ip_info;
    esp_netif_dns_info_t dns_info;
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
    esp_netif_t *eth_netif = g_eth_netif;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        if( ESP_OK != (err = esp_netif_dhcps_stop(eth_netif)) )
        {
            ESP_LOGE(TAG, "eth_event_handler Stop dhcp server Err: %s", esp_err_to_name(err));
        }

#if (1 == ETH_STATIC_IP)
        ESP_LOGI(TAG, "eth_event_handler static ip");

        if( ESP_OK != (err = esp_netif_dhcpc_stop(eth_netif)) )
        {
            ESP_LOGE(TAG, "eth_event_handler Stop dhcp Err: %s", esp_err_to_name(err));
        }

        ip_info.ip.addr     = esp_ip4addr_aton("192.168.43.198");
        ip_info.netmask.addr= esp_ip4addr_aton("255.255.255.0");
        ip_info.gw.addr     = esp_ip4addr_aton("192.168.43.1");
        dns_info.ip.u_addr.ip4.addr    = esp_ip4addr_aton("192.168.43.1");
        if( ESP_OK != (err = esp_netif_set_ip_info(eth_netif, &ip_info)) )
        {
            ESP_LOGE(TAG, "eth_event_handler set ip Err: %s", esp_err_to_name(err));
        }
        if( ESP_OK != (err = esp_netif_set_dns_info(eth_netif, ESP_NETIF_DNS_MAIN, &dns_info)) )
        {
            ESP_LOGE(TAG, "eth_event_handler set dns Err: %s", esp_err_to_name(err));
        }
#endif
#if (1 == ETH_DHCP_IP)
        ESP_LOGI(TAG, "eth_event_handler dhcp ip");
        if( ESP_OK != (err = esp_netif_dhcpc_start(eth_netif)) )
        {
            ESP_LOGE(TAG, "eth_event_handler Start dhcp Err: %s", esp_err_to_name(err));
        }
#endif
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

//    sntpx_stop();
    sntpx_start("pool.ntp.org");
}

void app_main(void)
{
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    g_eth_netif = eth_netif;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;

    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_JL1101
    esp_eth_phy_t *phy = esp_eth_phy_new_jl1101(&phy_config);
#endif

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    pcf8563x_init();
}



