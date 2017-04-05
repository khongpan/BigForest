#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c_device.h"


#define SHT31_ADDR 0x44
#define SHT31_CMD_MEAS_TH_H 0x24
#define SHT31_CMD_MEAS_TH_L 0x00

#define SHT31_MEAS_DATA_LEN 6

#define SHT31_CMD_RESET_H 0x30
#define SHT31_CMD_RESET_L 0xA2

#define SHT31_VDD_GPIO 21

static const char *TAG = "[SHT3x]";

esp_err_t sht31_soft_reset(void)
{
	
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT31_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, SHT31_CMD_RESET_H, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, SHT31_CMD_RESET_L, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

	return ret;
}


esp_err_t sht31_read(float *pTemp,float *pHumid)
{
	uint8_t data[SHT31_MEAS_DATA_LEN];
		
	float temp,humid;
	
	
	for (int j=0;j<SHT31_MEAS_DATA_LEN;j++) 
	{
		data[j]=0xff;
	}
	
	*pTemp=-1;
	*pHumid=-1;
	
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHT31_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, SHT31_CMD_MEAS_TH_H, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, SHT31_CMD_MEAS_TH_L, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL) {
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHT31_ADDR << 1) | READ_BIT, ACK_CHECK_DIS);
    i2c_master_read(cmd, &data[0],SHT31_MEAS_DATA_LEN-1, ACK_VAL);
	i2c_master_read_byte(cmd, &data[SHT31_MEAS_DATA_LEN-1], NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL) {
        return ESP_FAIL;
    }
/*	
	for(int i=0;i<MEAS_DATA_LEN;i++)
	{
		printf("%02x ",data[i]);
	}
	printf("\n");
*/	
	temp = data[0]*256.0+data[1];
	*pTemp = -45+(temp*175.0)/65535;
	humid = data[3]*256.0+data[4];
	*pHumid = (humid*100)/65535;
	
    return ESP_OK;
}	
	




void sht31dis_task(void *pvParameter)
{
    
	float Temp,Humid;
	
	sht31_soft_reset();
	vTaskDelay(1000 / portTICK_RATE_MS);
	
    while(1) {
		
		Temp=0;Humid=0;
		
		//ESP_ERROR_CHECK(sht31_read(&Temp,&Humid));
		sht31_read(&Temp,&Humid);
		ESP_LOGI(TAG,"%2.2f,%3.2f",Temp,Humid);

        vTaskDelay(10000 / portTICK_RATE_MS);
    }
	

}


void sht31dis_init()
{
	
	gpio_pad_select_gpio(SHT31_VDD_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(SHT31_VDD_GPIO, GPIO_MODE_OUTPUT);
    //gpio_set_level(SHT31_VDD_GPIO, 0);
	//vTaskDelay(100/portTICK_RATE_MS);
	gpio_set_level(SHT31_VDD_GPIO, 1);
	//vTaskDelay(500/portTICK_RATE_MS);
	
	//xTaskCreate(&sht31dis_task, "sht31dis_task", 2048, NULL, 5, NULL);
	
	//sht31_reset();
    //i2c_master_init();
}
