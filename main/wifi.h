#ifndef WIFI_H
#define WIFI_H

int wifi_connection_check(void);
esp_err_t wifi_wait_for_connection(void);
void wifi_start(void);


#endif