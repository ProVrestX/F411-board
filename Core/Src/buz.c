#include "buz.h"

static uint16_t buz_dur = 0;

void Buz_Start(uint16_t dur_ms) {
    HAL_GPIO_WritePin(Buz_GPIO_Port, Buz_Pin, GPIO_PIN_RESET);
    buz_dur = dur_ms;
}

void Buz_UpdateTime(uint16_t time_ms) {
    if(buz_dur <= time_ms) {
        Buz_Stop();
        return;
    }

    buz_dur -= time_ms;
}

void Buz_Stop(void) {
    HAL_GPIO_WritePin(Buz_GPIO_Port, Buz_Pin, GPIO_PIN_SET);
    buz_dur = 0;
}