#ifndef ads1115_h
#define ads1115_h

void ads1115_init(void);
esp_err_t ads1115_read(int ch,float *val);

#endif