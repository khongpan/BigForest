#ifndef CONFIG_H

#define CONFIG_H

extern char wifi_ssid[20];
extern char wifi_pass[20];
extern char node_name[20];

void config_init(void);
esp_err_t config_get_key_str(char *key, char * ret_str, char *default_str);
esp_err_t config_set_key_str(char *key, char *set_str);

#endif

