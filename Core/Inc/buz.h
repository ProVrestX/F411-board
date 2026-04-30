#ifndef INC_BUZ_H_
#define INC_BUZ_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


void Buz_Start(uint16_t dur_ms);
void Buz_UpdateTime(uint16_t time_ms);
void Buz_Stop(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_BUZ_H_ */