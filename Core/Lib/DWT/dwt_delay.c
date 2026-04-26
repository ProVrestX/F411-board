#include "dwt_delay.h"


void DWT_Delay_Init(void) {
    /* Разблокируем доступ к DWT (Debug Exception and Monitor Control Register) */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    /* Сбрасываем счётчик циклов */
    DWT->CYCCNT = 0;
    
    /* Включаем счётчик циклов */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWT_DelayMs(uint32_t ms) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = ms * (SystemCoreClock / 1000);

    while ((DWT->CYCCNT - start) < ticks);
}

void DWT_DelayUs(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);

    while ((DWT->CYCCNT - start) < ticks);
}