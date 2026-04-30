#ifndef INC_DHT_H_
#define INC_DHT_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


#define DHT_OK         0
#define DHT_NO_RESP    1
#define DHT_TIMEOUT    2
#define DHT_CHECKSUM   3


typedef struct {
    float temp;
    uint8_t hum;
} DHT_Data_t;


uint8_t DHT_Read(DHT_Data_t *data);


#ifdef __cplusplus
}
#endif

#endif /* INC_DHT_H_ */