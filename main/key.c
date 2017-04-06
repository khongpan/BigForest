#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "config.h"
static const char *TAG = "[KEY]";
#define KEY0_IO     0
#define GPIO_INPUT_PIN_SEL  ((1<<KEY0_IO))
#define ESP_INTR_FLAG_DEFAULT 0

//static xQueueHandle key_evt_queue = NULL;


static xQueueHandle key_evt_queue = NULL;
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(key_evt_queue, &gpio_num, NULL);
}

static void key_task(void* arg)
{
	int press_cnt=0;
	
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(key_evt_queue, &io_num, 1000)) {
			press_cnt++;
            ESP_LOGI(TAG,"key %d pressed", io_num);
        }else {
			if(press_cnt>0)	{
				ESP_LOGI(TAG,"continuous key pressed %d times", press_cnt);
				if(press_cnt==8) {
					ESP_LOGI(TAG,"Enter secret setting");
					config_restore_myhome();
				} else if (press_cnt==5) {
					ESP_LOGI(TAG,"Restore default setting");
					config_restore_default();
				}
				press_cnt=0;
			}
		}
    }
}

void key_init(void)
{
	gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
	//disable pull-down mode
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);


    //create a queue to handle gpio event from isr
    key_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(key_task, "key_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(KEY0_IO, gpio_isr_handler, (void*) 0);
 

}

