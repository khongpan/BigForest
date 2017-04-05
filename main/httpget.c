/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "wifi.h"
#include <time.h>
#include <string.h>
#include "config.h"
#include "sht31arp.h"
#include "sht31dis.h"


//http://agritronics.nstda.or.th/webpost0606/log.php?data1=NODEMCU_002,1000,A,17/03/06,11:36:00,7,10,1,2,3,4




/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

/* FreeRTOS event group to signal when we are connected & ready to make a request */
//static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
//const int CONNECTED_BIT = BIT0;

/* Constants that aren't configurable in menuconfig */
//#define WEB_SERVER "example.com"
//#define WEB_PORT 80
//#define WEB_URL "http://example.com/"
#define WEB_SERVER "agritronics.nstda.or.th"
#define WEB_PORT 80
//#define WEB_URL "http://agritronics.nstda.or.th/webpost0606/log.php?data1=NODEMCU_002,1000,A,"
//#define WEB_URL "http://agritronics.nstda.or.th/webpost0606/log.php?data1=NODEMCU_002,1000,A,17/03/06,11:50:00,7,10,1,2,3,4"

#define WEB_URL1 "GET http://agritronics.nstda.or.th/webpost0606/log.php?data1=NODEMCU_002,1000,A,"
#define WEB_URL2 "17/03/06,11:50:00,7,10,"

#define WEB_URL  "GET http://agritronics.nstda.or.th/webpost0606/log.php?data1="



static const char *TAG = "[HTTP_GET]";

/*
static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\n"
    "Host: "WEB_SERVER"\n"
    "User-Agent: esp-idf/1.0 esp32\n"
    "\n";
*/
	
static const char *close_str = " HTTP/1.1\n"
    "Host: "WEB_SERVER"\n"
    "User-Agent: esp-idf/1.0 esp32\n"
    "\n";	

	
char *get_current_time_str(char *time_str)
{

    time_t now;
    struct tm ti;
	
	time(&now);

    // Set timezone to Bangkok Standard Time
    setenv("TZ", "CST-7CDT-9,M4.2.0/2,M9.2.0/3", 1);
    tzset();
    localtime_r(&now, &ti);
	
	sprintf(time_str,"%02d/%02d/%02d,%02d:%02d:%02d",ti.tm_year-100,ti.tm_mon+1,ti.tm_mday,ti.tm_hour,ti.tm_min,ti.tm_sec);
	
	return time_str;
}


void delay_modulo(int hh, int mm, int ss)
{
	
    time_t now;
    struct tm ti;
	int rh,rm,rs;

	if(hh==0) hh=24;
	if(mm==0) mm=60;
	if(ss==0) ss=60;
	
	for(;;) {
		
		time(&now);

		// Set timezone to Bangkok Standard Time
		//setenv("TZ", "CST-7CDT-9,M4.2.0/2,M9.2.0/3", 1);
		//tzset();
		localtime_r(&now, &ti);

		rh=ti.tm_hour%hh;
		rm=ti.tm_min%mm;
		rs=ti.tm_sec%ss;
		//printf("%d:%d:%d \n",rh,rm,rs);
		//printf("%d:%d:%d \n",ti.tm_hour,ti.tm_min,ti.tm_sec);
		if ((rh==0)&&(rm==0)&&(rs==0))
		{
			return;
		}
		vTaskDelay(950/portTICK_PERIOD_MS);
	}
}	


char time_stamp_str[20];
char sensor_val_str[20];
char node_name_str[20];

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    //char recv_buf[64];
	
	char *recv_buf;

    while(1) {
		
		get_current_time_str(time_stamp_str);
		float temp,humid;
		//temp = sht31arp_read_temp();
		//humid = sht31arp_read_humid();
		sht31_read(&temp,&humid);
		sprintf(sensor_val_str,"%2.1f,%2.1f",temp,humid);

	
		wifi_wait_for_connection();
        ESP_LOGI(TAG, "Connected to AP ");

        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

        Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\r\n");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

		if (write(s, WEB_URL, strlen(WEB_URL)) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		
		sprintf(node_name_str,"%s,1000,A,",node_name);
		if (write(s, node_name_str, strlen(node_name_str)) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		
		if (write(s, time_stamp_str, strlen(time_stamp_str)) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		if (write(s, ",7,10,", strlen(",7,10,")) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		
		if (write(s, sensor_val_str, strlen(sensor_val_str)) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		
		if (write(s, close_str, strlen(close_str)) < 0) 
		{	
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
		}
		

		
        ESP_LOGI(TAG, "... socket send success");
		
		#define BUFF_SIZE 256
		recv_buf = malloc(BUFF_SIZE);

        /* Read HTTP response */
        //do {
            bzero(recv_buf, BUFF_SIZE);
            r = read(s, recv_buf, BUFF_SIZE-1);
            //for(int i = 0; i < r; i++) {
                //putchar(recv_buf[i]);
            //}
			ESP_LOGI(TAG,"http response\n%s", recv_buf);
        //} while(r > 0);
		
		free(recv_buf);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
		
        //for(int countdown = 300; countdown >= 0; countdown--) {
        //    //ESP_LOGI(TAG, "%d... ", countdown);
		//	//ESP_LOGI(TAG,"Current time %s", get_current_time_str(recv_buf));
        //    vTaskDelay(1000 / portTICK_PERIOD_MS);
        //}
		delay_modulo(1,60,60);

        //ESP_LOGI(TAG, "Starting again!");
    }
}

void httpget_init(void)
{
    //nvs_flash_init();
    //initialise_wifi();
    xTaskCreate(&http_get_task, "http_get_task", 2048, NULL, 5, NULL);
}


void http_post(char *post_str)
{
	
	
}




