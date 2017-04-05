#include "freertos/FreeRTOS.h"
//#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
//#include "nvs_flash.h"
#include "driver/gpio.h"

#include "sht31dis.h"
#include "wifi.h"

#define BLINK_GPIO 16

int i=1;



float Temp,Humid;

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	
	int tick_period = portTICK_PERIOD_MS;
	printf("tick period %d ms\n",tick_period);
	
    while(1) {
		
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
		
		//Temp=0;Humid=0;
		
		//ESP_ERROR_CHECK(sht31_read(&Temp,&Humid));
		//sht31_read(&Temp,&Humid);
		//printf("Alive %d T=%2.1f H%2.1f \n",i,Temp,Humid);i++;
		
		//printf("alive %d\n",i);
		i++;
		if (wifi_connection_check())
		{
			vTaskDelay(900 / portTICK_PERIOD_MS);
		}else {
			vTaskDelay(4900 / portTICK_PERIOD_MS);	
		}
        
    }
}

void heartbeat_init(void)
{
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}