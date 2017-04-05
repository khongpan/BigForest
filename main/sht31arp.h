#ifndef SHT31ARP_H

#define SHT31ARP_H

float sht31arp_read_temp(void);
float sht31arp_read_humid(void);
void sht31arp_init();
void read_sht31arp_task(void *pvParameters);

#endif
