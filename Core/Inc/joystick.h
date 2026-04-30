#ifndef INC_JOYSTICK_H_
#define INC_JOYSTICK_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"

#define JOYSTICK_ADC hadc1

typedef struct {
    int8_t x;
    int8_t y;
} Joystick_Data_t;

void Joystick_Start(void);
uint8_t Joystick_GetData(Joystick_Data_t *data);
void Joystick_Stop(void);
uint8_t Joystick_GetClick(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_JOYSTICK_H_ */