/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_deep_sleep.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "apps/sntp/sntp.h"

#include "config.h"
#include "wifi.h"
#include "mqtt_client.h"

#define WIFI_SSID "TP-LINK_B48E7E"
#define WIFI_PASS "opensesami"



/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;

static const char *TAG = "[WIFI]";


static void initialise_wifi(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);


esp_err_t wifi_wait_for_connection(void)
{
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
						
	return ESP_OK;
	
}

int wifi_connection_check(void)
{
	if ( xEventGroupGetBits( wifi_event_group ) & CONNECTED_BIT ) {
		return 1;
	} else {
		return 0;
	}
		
		
}


void wifi_start(void)
{
	initialise_wifi();
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
	
}	




static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
	memcpy(wifi_config.sta.ssid,wifi_ssid,20);
	memcpy(wifi_config.sta.password,wifi_pass,20);
	
	
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		#ifdef MQTT_CLIENT_H
		mqtt_client_start();
		#endif
		ESP_LOGI(TAG,"got ip\n");
        ESP_LOGI(TAG,"ip: " IPSTR "\n", IP2STR(&event->event_info.got_ip.ip_info.ip));
        ESP_LOGI(TAG,"netmask: " IPSTR "\n", IP2STR(&event->event_info.got_ip.ip_info.netmask));
        ESP_LOGI(TAG,"gw: " IPSTR "\n", IP2STR(&event->event_info.got_ip.ip_info.gw));
        ESP_LOGI(TAG,"\n");
        //fflush(stdout);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
		#ifdef MQTT_CLIENT_H   
		mqtt_client_stop();
		#endif
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}
