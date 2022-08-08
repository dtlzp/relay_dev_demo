/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: http_test.c
 * function: http server demo source code
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

#include "sdkconfig.h"


#include "../../common/dt_device_tree.h"

#define TAG "http_test"

ddt_t g_dt = DDT_DEF_VALUE();

//#define CONFIG_EXAMPLE_ETH_PHY_RTL8201  1 /* ethernet PHY RTL8201F */
//#define CONFIG_EXAMPLE_ETH_PHY_LAN8720  1 /* ethernet PHY LAN8720A */
#define CONFIG_EXAMPLE_ETH_PHY_JL1101   1   /* ethernet PHY JL1101 */

#define CONFIG_EXAMPLE_ETH_PHY_ADDR     (-1)
#define CONFIG_EXAMPLE_ETH_PHY_RST_GPIO g_dt.gpio_phyrst
#define CONFIG_EXAMPLE_ETH_MDC_GPIO     GPIO_RMII_MDC
#define CONFIG_EXAMPLE_ETH_MDIO_GPIO    GPIO_RMII_MDIO

//#define ETH_STATIC_IP 1
#define ETH_DHCP_IP 1

typedef struct _fs_item_t{
    const char *hex;
    u16_t length;
}fs_item_t;

typedef enum _FI_TYPE{
    FI_index_html,

    FI_END,

    FI_INDEX = FI_index_html,
}FI_TYPE;

#include "fsdata_dingtian.c"
fs_item_t g_fi[FI_END] = 
{
    {(const char*)index_html,         sizeof(index_html)},
};

char* search_suffix(char *url)
{
    char *suffix = NULL;
    s32 len = strlen(url);
    s32 i;

    ESP_LOGI(TAG, "search_suffix %s:%d", url, len);

    url += (len-1);
    for(i=len-1; i>=0; i--, url--)
    {
        if( '.' == *url )
        {
            suffix = url+1;
            break;
        }
    }
    return suffix;
}
esp_err_t default_get_handler(httpd_req_t *req)
{
    char *type[] = 
    {
        "html",
        "js",
        "css",
        "jpg",
        "png",
        "gif",
        "svg",
        "ico",
    };
    char *type_text[] = 
    {
        "text/html",
        "text/javascript",
        "text/css",
        "image/jpeg",
        "image/png",
        "image/gif",
        "image/svg",
        "image/ico",
    };
    u8 i,cnt;
    char *suffix;
    fs_item_t *fi = (fs_item_t*)req->user_ctx;

    if( ('/' == req->uri[0]) && (0 == req->uri[1]) )
    {
        i = 0;
        goto send;
    }
    suffix = search_suffix((char*)req->uri);
    if( NULL == suffix )
    {
        return ESP_FAIL;
    }
    ESP_LOGD(TAG, "default_get_handler suffix=%s", suffix);
    cnt = sizeof(type)/sizeof(char*);
    for(i=0; i<cnt; i++)
    {
        if( 0 == strcmp(suffix, type[i]) )
        {
            break;
        }
    }
    if( i >= cnt )
    {
        i = 0;
    }

send:
    ESP_LOGI(TAG, "Free Stack for server task: '%d'", uxTaskGetStackHighWaterMark(NULL));
//    httpd_resp_set_hdr(req, "Content-Encoding", "deflate");
    httpd_resp_set_type(req, type_text[i]);
    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, fi->hex, fi->length);
    return ESP_OK;
}

httpd_uri_t basic_handlers[] = {
    { .uri      = "/",
      .method   = HTTP_GET,
      .handler  = default_get_handler,
      .user_ctx = &(g_fi[FI_INDEX]),
    },
    { .uri      = "/index.html",
      .method   = HTTP_GET,
      .handler  = default_get_handler,
      .user_ctx = &(g_fi[FI_INDEX]),
    },
};

httpd_handle_t g_http = NULL;
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;

    if (httpd_start(&g_http, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(g_http, &basic_handlers[0]);
        httpd_register_uri_handler(g_http, &basic_handlers[1]);
        return g_http;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(g_http);
    g_http = NULL;
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
        stop_webserver();
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

        ip_info.ip.addr     = esp_ip4addr_aton("192.168.1.100");
        ip_info.netmask.addr= esp_ip4addr_aton("255.255.255.0");
        ip_info.gw.addr     = esp_ip4addr_aton("192.168.1.1");
        dns_info.ip.u_addr.ip4.addr    = esp_ip4addr_aton("192.168.1.1");
        
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

    if( NULL == g_http )
    {
        start_webserver();
    }
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
}


