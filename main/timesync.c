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

#include "wifi.h"
#include "timesync.h"


static const char *TAG = "[TSYNC]";

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */


static void initialize_sntp(void);

/*
void timesync_obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 20;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}
*/

void timesync_obtain_time(void)
{
    //

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
	const int retry_count=4;
	int wait_sec;

	while(retry++ < retry_count)
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);

		wifi_wait_for_connection();
		initialize_sntp();
		wait_sec=15;
		while(timeinfo.tm_year < (2016 - 1900) && wait_sec-- ) {
			//ESP_LOGI(TAG, "Waiting for system time to be set... (%d)",wait_sec );
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			time(&now);
			localtime_r(&now, &timeinfo);
		}
		if (wait_sec>0)  {
			ESP_LOGI(TAG,"SYNC SUCCESS");
			return;
		}
	}
	ESP_LOGW(TAG,"SYNC FAIL");
}

char *(sntp_server[])= {"pool.ntp.org","th.pool.ntp.org"};
int sntp_index=0;

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP %s",sntp_server[sntp_index]);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, sntp_server[sntp_index]);
    sntp_init();
	//change sntp server every time
	if (++sntp_index>1) sntp_index=0;
}