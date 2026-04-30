#include "dht.h"
#include "dwt_delay.h"


#define DHT_LOW()  (DHT_GPIO_Port->BSRR = ((uint32_t)DHT_Pin << 16))
#define DHT_HIGH() (DHT_GPIO_Port->BSRR = DHT_Pin)
#define DHT_READ() ((DHT_GPIO_Port->IDR & DHT_Pin)? GPIO_PIN_SET : GPIO_PIN_RESET)


uint8_t DHT_Read(DHT_Data_t *data) {
    DHT_LOW();
    DWT_DelayMs(1);
    DHT_HIGH();
    DWT_DelayUs(30);

    if (DHT_READ() != GPIO_PIN_RESET) return DHT_NO_RESP;

    uint32_t timeout = 0;
    while (DHT_READ() == GPIO_PIN_RESET) {
        if (timeout++ > 10) return DHT_TIMEOUT; 
        DWT_DelayUs(10);
    }

    timeout = 0;
    while (DHT_READ() == GPIO_PIN_SET) {
        if (timeout++ > 10) return DHT_TIMEOUT;
        DWT_DelayUs(10);
    }

    __disable_irq();
    uint8_t buffer[5] = {0};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            while (DHT_READ() == GPIO_PIN_RESET);
            
            DWT_DelayUs(40);
            
            if (DHT_READ() == GPIO_PIN_SET) {
                buffer[i] |= (1 << (7 - j));
                while (DHT_READ() == GPIO_PIN_SET);
            }
        }
    }
    __enable_irq();

    if ((uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3]) != buffer[4]) {
        return DHT_CHECKSUM;
    }

    int16_t temp_raw = ((int16_t)(buffer[2] & 0x7F) << 8) | buffer[3];
    if (buffer[2] & 0x80) temp_raw = -temp_raw;
    data->temp = temp_raw / 10.0f;

    data->hum = ((buffer[0] << 8) | buffer[1]) / 10;

    return DHT_OK;
}