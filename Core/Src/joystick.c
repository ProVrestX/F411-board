#include "joystick.h"
#include "adc.h"

volatile uint8_t joy_sw = 0;

static volatile uint16_t adc_data[2] = {0};

static uint8_t init_state = 0;
static Joystick_Data_t data_ofs = {0};


void Joystick_Start(void) {
    HAL_ADC_Start_DMA(&JOYSTICK_ADC, (uint32_t*)adc_data, 2);
    HAL_Delay(10);

    data_ofs.x = (int8_t)(adc_data[0] * 100 / 4095);
    data_ofs.y = (int8_t)(adc_data[1] * 100 / 4095);

    init_state = 1;
}

uint8_t Joystick_GetData(Joystick_Data_t *data) {
    if(!init_state)
        return 1;

    data->x = (int8_t)(adc_data[0] * 100 / 4095) - data_ofs.x;
    data->y = (int8_t)(adc_data[1] * 100 / 4095) - data_ofs.y;
    
    return 0;
}

void Joystick_Stop(void) {
    HAL_ADC_Stop_DMA(&JOYSTICK_ADC);
    init_state = 0;
}

uint8_t Joystick_GetClick(void) {
    uint8_t joy_sw_tmp = joy_sw;
    joy_sw = 0;
    return joy_sw_tmp;
}
