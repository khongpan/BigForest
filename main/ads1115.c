#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "i2c_device.h"


#define ADS1115_ADDR 0x48

static char* TAG = "[ADS1115]";


esp_err_t ads1115_reset(void)
{
	int ret=ESP_FAIL;
	
	ret=ESP_OK;
	
	return ret;
}


esp_err_t ads1115_read(int ch,float *val)
{
	uint8_t data[2];
	i2c_cmd_handle_t cmd;
	int ret;
		
	*val = -1;
	
	//start conversion
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADS1115_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x01, ACK_CHECK_EN); 
	i2c_master_write_byte(cmd, 0x83 | (ch*16), ACK_CHECK_EN);
	i2c_master_write_byte(cmd, 0x03, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL) {
		ESP_LOGW(TAG,"No Response")
        return ret;
    }
	
	//waiting for conversion complete
	do {	
		vTaskDelay(150/ portTICK_RATE_MS);
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, ADS1115_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
		i2c_master_read_byte(cmd, &data[0], ACK_VAL);
		i2c_master_read_byte(cmd, &data[1], NACK_VAL);
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
		if (ret == ESP_FAIL) {
			ESP_LOGW(TAG,"Conversion fail");
			return ESP_FAIL;
		}
	} while ((data[0]&0x80)==0);
		
	
	// Set pointer register to 0 to read from the conversion register
	cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADS1115_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN); 
	i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL) {
		ESP_LOGW(TAG,"Read fail");
        return ret;
    }
		
	//read 16 bit results
	
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADS1115_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, &data[0],1, ACK_VAL);
	i2c_master_read_byte(cmd, &data[1], NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL) {
		ESP_LOGW(TAG,"Read fail");
        return ESP_FAIL;
    }
	
	*val=(float)(data[0])*256+data[1];
	*val=(*val*4.096)/32767.0;

    return ESP_OK;
}	


void ads1115_task(void *pvParameter)
{
	float val[2];
	
    while(1) {
		
		ads1115_read(0,val);
		ads1115_read(3,val+1);
		ESP_LOGI(TAG,"%1.3f,%1.3f",val[0],val[1]);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
	

}
	

void ads1115_init(void)
{
    //xTaskCreate(&ads1115_task, "ads1115_task", 2048, NULL, 5, NULL);
}

