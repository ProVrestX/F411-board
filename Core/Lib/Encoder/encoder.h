#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


void Encoder_Check(void);
void Encoder_SetScrolFunc(void (*Enc_ScrolFunc)(int8_t dur));
void Encoder_SetClickFunc(void (*Enc_ClickFunc)(void));


#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_H_ */