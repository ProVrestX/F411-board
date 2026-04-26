#include "main.h"

extern volatile int8_t enc_dur;
extern volatile uint8_t enc_clk;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if(GPIO_Pin == Accel_INT_Pin) {

    }
    else if(GPIO_Pin == Enc_CLK_Pin) {
        enc_dur = (Enc_DT_GPIO_Port->IDR & Enc_DT_Pin)? 1: -1;
    }
    else if(GPIO_Pin == Enc_SW_Pin) {
        enc_clk = 1;
    }
    else if(GPIO_Pin == Joy_SW_Pin) {

    }
}