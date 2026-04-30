#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"

#define ENC_TIM htim2 

void Encoder_Check(void);
void Encoder_SetScrolFunc(void (*Enc_ScrolFunc)(int8_t dir));
void Encoder_SetClickFunc(void (*Enc_ClickFunc)(void));
int16_t Encoder_GetCount(void);
int8_t Encoder_GetScrol(void);
uint8_t Encoder_GetClick(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_H_ */