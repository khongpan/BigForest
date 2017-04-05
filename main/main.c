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
#include "heartbeat.h"
#include "i2c_device.h"
#include "sht31arp.h"
#include "sht31dis.h"
#include "ads1115.h"

#include "wifi.h"
#include "timesync.h"
#include "httpget.h"
#include "httpserv.h"

static const char *TAG = "[MAIN]";

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;




void app_main()
{
	nvs_flash_init();
	config_init();
	wifi_start();
	
    ++boot_count;
    ESP_LOGI(TAG, "Boot count: %d", boot_count);
	
	heartbeat_init();
	
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        timesync_obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];

    // Set timezone to Bangkok Standard Time
    setenv("TZ", "CST-7CDT-9,M4.2.0/2,M9.2.0/3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Bangkok is: %s", strftime_buf);
	
	
	i2c_device_init();
	ads1115_init();
	sht31dis_init();	
	sht31arp_init();
	httpserv_start();
	httpget_init();
	

 //   const int deep_sleep_sec = 10;
 //   ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
 //   esp_deep_sleep(1000000LL * deep_sleep_sec);
}
