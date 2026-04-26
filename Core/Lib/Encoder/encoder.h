#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


void Enc_Check(void);
void Enc_SetScrolFunc(void (*Enc_ScrolFunc)(int8_t dur));
void Enc_SetClickFunc(void (*Enc_ClickFunc)(void));


#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_H_ */