#include <stdio.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "string.h"
#include "config.h"

#define MAX_KEY_VALUE_LEN   20

size_t key_val_len = MAX_KEY_VALUE_LEN;

nvs_handle cfg_handle;

/*
typedef struct cfg_t {
char key[10];
char val[20];
}cfg_t;

cfg_t cfg_list[] = 
{
	{"node_name","NODE_000"},
	{"ssid"		,"BigForest"},
	{"password" ,"88888888"},
	{""			,""},
};
*/

char wifi_ssid[20];
char wifi_pass[20];
char node_name[20];


void config_restore_default(void)
{
	esp_err_t err;
	err = nvs_set_str(cfg_handle, "wifi_ssid", "BigForest");
	err = nvs_set_str(cfg_handle, "wifi_pass", "88888888");
}

void config_restore_myhome(void)
{
	esp_err_t err;
	err = nvs_set_str(cfg_handle, "wifi_ssid", "TP-LINK_B48E7E");
	err = nvs_set_str(cfg_handle, "wifi_pass", "opensesami");
	
}

void config_init(void)
{
	esp_err_t err;
	
	err =  nvs_open("config",NVS_READWRITE, &cfg_handle);
	
	config_get_key_str("wifi_ssid",wifi_ssid,"BigForest");
	config_get_key_str("wifi_pass",wifi_pass,"88888888");
	config_get_key_str("node_name",node_name,"NODE_000");
	
	printf("node_name %s\n",node_name);
	printf("wifi_ssid %s\n",wifi_pass);
	printf("wifi_pass %s\n",wifi_ssid);
	
	if (err!=ESP_OK) {
		printf("Error (%d) opening NVS!\n", err);
	}
}

esp_err_t config_get_key_str(char *key, char * ret_str, char *default_str)
{
	esp_err_t err;
	
	strcpy(ret_str,default_str);
	
	err = nvs_get_str(cfg_handle, key, ret_str, &key_val_len);
	
	if (err!=ESP_OK) {
		//sprintf(ret_str,"err %d",err);
	}
	
	return err;
	
}

esp_err_t config_set_key_str(char *key, char *set_str)
{
	esp_err_t err;
	
	err = nvs_set_str(cfg_handle, key, set_str);
	
	return err;
}

