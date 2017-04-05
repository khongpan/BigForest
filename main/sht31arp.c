/*
 * MaiKe Labs (2016 - 2026)
 *
 * Written by Jack Tan <jiankemeng@gmail.com>
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"

//static const char *SHT31_TAG = "SHT31ARP";




/* 
 * Read the amp adc, IO36 as the ADC_PRE_AMP
 * make sure connecting a 270pF cap from
 * esp32_pin5 to esp32_pin6
 */
//uint32_t adc1_amp_read();

#define ADC_TEMP_CH ADC1_CHANNEL_0
#define ADC_HUMID_CH ADC1_CHANNEL_3
#define V_SUPPLY 3.3


float sht31arp_read_temp(void)
{
	uint16_t adc_val;
	float v_adc;
	float temp;
	
	adc_val = adc1_get_voltage(ADC1_CHANNEL_0);
	v_adc = adc_val/2048.0;
	temp = -66.875+218.75*v_adc/V_SUPPLY;
	
	return temp;
}

float sht31arp_read_humid(void)
{
	uint16_t adc_val;
	float v_adc;
	float humid;
	
	adc_val = adc1_get_voltage(ADC1_CHANNEL_0);
	v_adc = adc_val/2048.0;
	humid = -12.5+125*v_adc/V_SUPPLY;
	
	return humid;

}


void read_sht31arp_task(void *pvParameters)
{
	while (1) {
		float vch0,vch3;
		float temp,humid;
		
		vch0=adc1_get_voltage(ADC1_CHANNEL_0)/2048.0;
		vch3=adc1_get_voltage(ADC1_CHANNEL_3)/2048.0;

		
		temp = -66.875+218.75*vch0/3.3;
		humid = -12.5+125*vch3/3.3;
		
		printf("----------------------------------------------\n");
		printf("ESP32 ADC1_CH0 (IO36) = %1.3f %1.3f\n",vch0, temp);
		printf("ESP32 ADC1_CH3 (IO39) = %1.3f %1.3f\n",vch3, humid);
		
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void sht31arp_init()
{
	/*
	gpio_set_direction(GPIO_NUM_34,GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_NUM_36,GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_NUM_39,GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_NUM_34,GPIO_MODE_INPUT);
	gpio_set_pull_mode(GPIO_NUM_36,GPIO_FLOATING);
	gpio_set_pull_mode(GPIO_NUM_39,GPIO_FLOATING);
	myadc_pad_init(ADC1_CHANNEL_0);
    myadc_pad_init(ADC1_CHANNEL_3);
	myadc_pad_init(ADC1_CHANNEL_6);
	*/
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_6db);
	adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_6db);
	
	//xTaskCreate(&read_sht31arp_task, "read_sht31_task", 2048, NULL, 5,NULL);

	
}
/*
void app_main()
{
	nvs_flash_init();
	adc1_init();

	printf("Welcome to Noduino Quantum\r\n");
	printf("Try to test the ADC1 pin of the ESP32 ... \r\n");
	xTaskCreatePinnedToCore(&read_adc_task, "read_adc_task", 2048, NULL, 5,
				NULL, 0);
}
*/