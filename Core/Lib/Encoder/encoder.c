#include "encoder.h"

volatile int8_t enc_dur = 0;
volatile uint8_t enc_clk = 0;

static void (*Enc_Scrol)(int8_t dur) = NULL;
static void (*Enc_Click)(void) = NULL;

void Enc_Check(void) {
    if(enc_dur) {
        if(Enc_Scrol != NULL)
            Enc_Scrol(enc_dur);
        enc_dur = 0;
    }
    
    if(enc_clk) {
        if(Enc_Click != NULL)
            Enc_Click();
        enc_clk = 0;
    }
}

void Enc_SetScrolFunc(void (*Enc_ScrolFunc)(int8_t dur)) {
    Enc_Scrol = Enc_ScrolFunc;
}

void Enc_SetClickFunc(void (*Enc_ClickFunc)(void)) {
    Enc_Click = Enc_ClickFunc;
}