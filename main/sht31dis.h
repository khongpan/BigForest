#ifndef SHT31DIS_H

#define SHT31DIS_H


esp_err_t sht31_read(float *pTemp,float *pHumid);
void sht31dis_init();
//void read_sht31arp_task(void *pvParameters);

#endif
