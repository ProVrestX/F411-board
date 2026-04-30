#include "encoder.h"
#include "tim.h"

volatile int8_t enc_dir = 0;
volatile uint8_t enc_clk = 0;
volatile int16_t enc_count = 0;

static void (*Enc_Scrol)(int8_t dir) = NULL;
static void (*Enc_Click)(void) = NULL;

void Encoder_Check(void) {
    #ifdef ENC_TIM
        static int16_t enc_count_prev = 0;
        int enc_d = (int16_t)__HAL_TIM_GET_COUNTER(&ENC_TIM) - enc_count_prev;
        enc_count_prev = (int16_t)__HAL_TIM_GET_COUNTER(&ENC_TIM);
        if(enc_d) {
            enc_dir = (enc_d > 0)? -1: 1;
        }
        else {
            enc_dir = 0;
        }
    #endif

    if(enc_dir) {
        if(Enc_Scrol != NULL)
            Enc_Scrol(enc_dir);
        enc_dir = 0;
    }
    
    if(enc_clk) {
        if(Enc_Click != NULL)
            Enc_Click();
        enc_clk = 0;
    }
}

void Encoder_SetScrolFunc(void (*Enc_ScrolFunc)(int8_t dir)) {
    Enc_Scrol = Enc_ScrolFunc;
}

void Encoder_SetClickFunc(void (*Enc_ClickFunc)(void)) {
    Enc_Click = Enc_ClickFunc;
}

int16_t Encoder_GetCount(void) {
    #ifdef ENC_TIM
        enc_count = (int16_t)__HAL_TIM_GET_COUNTER(&ENC_TIM) / 2;
    #endif

    return enc_count;
}

int8_t Encoder_GetScrol(void) {
    #ifdef ENC_TIM
        static int16_t enc_count_prev = 0;
        int enc_d = (int16_t)__HAL_TIM_GET_COUNTER(&ENC_TIM) - enc_count_prev;
        enc_count_prev = (int16_t)__HAL_TIM_GET_COUNTER(&ENC_TIM);
        if(enc_d) {
            enc_dir = (enc_d > 0)? -1: 1;
        }
        else {
            enc_dir = 0;
        }
    #endif

    int8_t enc_dir_tmp = enc_dir;
    enc_dir = 0;
    return enc_dir_tmp;
}

uint8_t Encoder_GetClick(void) {
    int8_t enc_clk_tmp = enc_clk;
    enc_clk = 0;
    return enc_clk_tmp;
}
