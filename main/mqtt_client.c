#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "soc/rtc_cntl_reg.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

//#include "user_config.h"
//#include "debug.h"
#include "config.h"
#include "mqtt.h"
#include "sht31arp.h"
#include "cmd.h"


static const char *TAG = "[MQTT_CLIENT]";

//#define INFO(...) printf(__VA_ARGS__)

#define INFO(...) ESP_LOGI(TAG,##__VA_ARGS__)

void connected_cb(void *self, void *params)
{
	INFO("Connected");
    mqtt_client *client = (mqtt_client *)self;
    mqtt_subscribe(client, "todevice", 0);
    //mqtt_publish(client, "fromdevice", "connected_cb", 12, 0, 0);
}
void disconnected_cb(void *self, void *params)
{

}
void reconnect_cb(void *self, void *params)
{

}
void subscribe_cb(void *self, void *params)
{
    INFO("Subscribe ok");
    //mqtt_client *client = (mqtt_client *)self;
    //mqtt_publish(client, "fromdevice", "subscribe_cb", 12, 0, 0);
}

void publish_cb(void *self, void *params)
{
	INFO("Publish Message\n");
    //mqtt_client *client = (mqtt_client *)self;
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;

    if (event_data->data_offset == 0) {
        char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        INFO("Publish topic: %s", topic);
        free(topic);
    }
	
	//if (event_data->data_length==0) return; //protect empty message;

    char *data = malloc(event_data->data_length + 1);
    memcpy(data, event_data->data, event_data->data_length);
    data[event_data->data_length] = 0;
    INFO("Publish data[%d/%d bytes] %s",
         event_data->data_length + event_data->data_offset,
         event_data->data_total_length,data);
         // data);
    free(data);	
}

void data_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;

    if (event_data->data_offset == 0) {

        char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        INFO("Received topic: %s", topic);
        free(topic);
    }
	
	//if (event_data->data_length==0) return; //protect empty message;

    char *data = malloc(event_data->data_length + 1);
    memcpy(data, event_data->data, event_data->data_length);
    data[event_data->data_length] = 0;
    INFO("Received data[%d/%d bytes] %s",
         event_data->data_length + event_data->data_offset,
         event_data->data_total_length,data);
         // data);
	char *ret_str = malloc(80);
	cmd_process_str(data,ret_str);
	int len;
	len = strlen(ret_str);
	
	
	if (len>0) {
		INFO("Publish topic: %s", "fromdevice");
		INFO("Publish data: %s", ret_str );
		mqtt_publish(client, "fromdevice", ret_str, len, 0, 0);	 
	}
	free(ret_str);

    free(data);

}


/*
mqtt_settings settings = {
    .host = "test.mosquitto.org",
#if defined(CONFIG_MQTT_SECURITY_ON)         
    .port = 8883, // encrypted
#else
    .port = 1883, // unencrypted
#endif    
    .client_id = "mqtt_client_id",
    .username = "user",
    .password = "pass",
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    .reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};
*/

mqtt_settings settings = {
    .host = "m10.cloudmqtt.com",
#if defined(CONFIG_MQTT_SECURITY_ON)         
    .port = 29178, // encrypted
#else
    .port = 19178, // unencrypted
#endif    
    .client_id ="node_004",
    .username = "esp32",
    .password = "esp32",
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    .reconnect_cb = reconnect_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb
};

void mqtt_client_start(void)
{
	mqtt_start(&settings);
}

void mqtt_client_stop(void)
{
	mqtt_stop();
}



/* 
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;

    case SYSTEM_EVENT_STA_GOT_IP:

        mqtt_start(&settings);
        // Notice that, all callback will called in mqtt_task
        // All function publish, subscribe
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate.
        
        mqtt_stop();
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    default:
        break;
    }
    return ESP_OK;


} */



/* 
void wifi_conn_init(void)
{
    INFO("[APP] Start, connect to Wifi network: %s ..\n", WIFI_SSID);

    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );

    wifi_init_config_t icfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&icfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK( esp_wifi_start());
}
*/

/* 
void app_main()
{
    INFO("[APP] Startup..\n");
    INFO("[APP] Free memory: %d bytes\n", system_get_free_heap_size());
    INFO("[APP] SDK version: %s, Build time: %s\n", system_get_sdk_version(), BUID_TIME);

#ifdef CPU_FREQ_160MHZ
    INFO("[APP] Setup CPU run as 160MHz\n");
    SET_PERI_REG_BITS(RTC_CLK_CONF, RTC_CNTL_SOC_CLK_SEL, 0x1, RTC_CNTL_SOC_CLK_SEL_S);
    WRITE_PERI_REG(CPU_PER_CONF_REG, 0x01);
    INFO("[APP] Setup CPU run as 160MHz - Done\n");
#endif
 
    nvs_flash_init();
    wifi_conn_init();
} 
*/