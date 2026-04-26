#ifndef INC_DWT_DELAY_H_
#define INC_DWT_DELAY_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


void DWT_Delay_Init(void);
void DWT_DelayMs(uint32_t ms);
void DWT_DelayUs(uint32_t us);


#ifdef __cplusplus
}
#endif

#endif /* INC_DWT_DELAY_H_ */