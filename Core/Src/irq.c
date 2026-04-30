#include "main.h"

#include "dwt_delay.h"
#include "mpu.h"
#include "buz.h"


#define DEBOUNCE_MS 5
#define BUZ_DUR 50

extern volatile int8_t enc_dur;
extern volatile uint8_t enc_clk;
extern volatile int16_t enc_count;
extern volatile uint8_t joy_sw;
extern volatile uint8_t MPU_dma_state;
extern volatile uint8_t MPU_read_requested;
extern volatile uint8_t ST7735_dma_state;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    static uint8_t mpu_counter = 0;

    if(htim->Instance == TIM5) {
        Buz_UpdateTime(10);
        mpu_counter++;

        if(mpu_counter == 10) {
            mpu_counter = 0;
            // MPU_Read();
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t sw_last_time = 0;
    static uint32_t joy_last_time = 0;

    uint32_t now = HAL_GetTick();

    if(GPIO_Pin == Accel_INT_Pin) {
        // MPU_Read();
        MPU_read_requested = 1;
    }
    else if(GPIO_Pin == Enc_SW_Pin) {
        DWT_DelayUs(100);
        if(HAL_GPIO_ReadPin(Enc_SW_GPIO_Port, Enc_SW_Pin)) 
            return;
        if(now - sw_last_time < DEBOUNCE_MS)
            return;
        sw_last_time = now;
        enc_clk = 1;
        Buz_Start(BUZ_DUR);
    }
    else if(GPIO_Pin == Joy_SW_Pin) {
        DWT_DelayUs(100);
        if(HAL_GPIO_ReadPin(Joy_SW_GPIO_Port, Joy_SW_Pin))
            return;
        if(now - joy_last_time < DEBOUNCE_MS)
            return;
        joy_last_time = now;
        joy_sw = 1;
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if(hi2c->Instance == I2C1) {
        MPU_dma_state = 0;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if(hspi->Instance == SPI1) {
        ST7735_dma_state = 0;
    }
}
