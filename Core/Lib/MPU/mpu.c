#include "mpu.h"
#include "i2c.h"
#include <math.h>

extern I2C_HandleTypeDef MPU_I2C;


#ifdef MPU_I2C_DMA
    volatile uint8_t MPU_dma_state = 0;
#endif
volatile uint8_t MPU_read_requested = 0;

static uint8_t MPU_init_state = 0;
static volatile uint8_t MPU_read_data = 0;
static volatile uint8_t MPU_read_fail = 0;
static volatile uint8_t MPU_data[14];

static void MPU_Transmit(uint16_t mem_addr, uint8_t *p_buf, uint16_t size);
static void MPU_Receive(uint16_t mem_addr, uint8_t *p_buf, uint16_t size);


static void MPU_Transmit(uint16_t mem_addr, uint8_t *p_buf, uint16_t size) {
    HAL_I2C_Mem_Write(&MPU_I2C, MPU_ADDR, mem_addr, I2C_MEMADD_SIZE_8BIT, p_buf, size, 100);
}

static void MPU_Receive(uint16_t mem_addr, uint8_t *p_buf, uint16_t size) {
    HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, mem_addr, I2C_MEMADD_SIZE_8BIT, p_buf, size, 100);
}


#ifdef MPU_I2C_DMA
    uint8_t MPU_WaitReady(void) {
        return MPU_dma_state;
    }
#endif

uint8_t MPU_Init(void) {
    uint8_t attempt = 0;

    do {
        uint8_t cmd = 0x80;
        MPU_Transmit(MPU_PWR_MGMT_1, &cmd, 1);
        HAL_Delay(100);

        cmd = 0x00;
        MPU_Transmit(MPU_PWR_MGMT_1, &cmd, 1);
        HAL_Delay(20);

        cmd = 0x00;
        MPU_Transmit(MPU_SMPRT_DIV, &cmd, 1);

        cmd = 0x06;
        MPU_Transmit(MPU_CONFIG, &cmd, 1);

        cmd = 0x00;
        MPU_Transmit(MPU_GYRO_CONFIG, &cmd, 1);

        cmd = 0x00;
        MPU_Transmit(MPU_ACCEL_CONFIG, &cmd, 1);

        // cmd = 0x70;
        // MPU_Transmit(MPU_FIFO_EN, &cmd, 1);

        // cmd = 0x10;
        cmd = 0x01;
        MPU_Transmit(MPU_INT_ENABLE, &cmd, 1);

        // cmd = 0x44;
        // MPU_Transmit(MPU_USER_CTRL, &cmd, 1);
        // cmd = 0x40;
        // MPU_Transmit(MPU_USER_CTRL, &cmd, 1);

        uint8_t who_am_i;
        MPU_Receive(MPU_WHO_AM_I, &who_am_i, 1);
        if((who_am_i & 0x7E) != MPU_WHO_AM_I_VALUE) {
            // printf("WHO_AM_I Fail: %02X\r\n", who_am_i);
            MPU_BusRecovery();
            attempt++;
            continue;
        }

        MPU_init_state = 1;
    } while(!MPU_init_state && attempt < MPU_MAX_INIT_ATTEMPT);

    if(!MPU_init_state) {
        return 1;
    }

    return 0;
}

uint8_t MPU_StartRead(void) {
    return HAL_I2C_Mem_Read_DMA(&MPU_I2C, MPU_ADDR, MPU_ACCEL_OUT, I2C_MEMADD_SIZE_8BIT, (uint8_t*)MPU_data, MPU_DATA_LEN);
}

void MPU_Read(void) {
    if(!MPU_init_state || !MPU_read_requested)
        return;

    MPU_read_requested = 0;
    MPU_read_fail = 0;

    #ifdef MPU_I2C_DMA
        if(MPU_dma_state)
            return;

        MPU_dma_state = 1;
        if(HAL_I2C_Mem_Read_DMA(&MPU_I2C, MPU_ADDR, MPU_ACCEL_OUT, 
                I2C_MEMADD_SIZE_8BIT, (uint8_t*)MPU_data, MPU_DATA_LEN) != HAL_OK) {
            MPU_dma_state = 0;
            MPU_read_fail = 1;
            return;
        }
        // while(MPU_dma_state);
    #else   // Don't use it in irq \/
        if(HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, MPU_ACCEL_OUT, 
                I2C_MEMADD_SIZE_8BIT, (uint8_t*)MPU_data, MPU_DATA_LEN, 10) != HAL_OK) {
            MPU_read_fail = 1;
            return;
        }
    #endif

    MPU_read_data = 1;
}

uint8_t MPU_Read_FIFO(MPU_Data_t *data, uint8_t max_frames) {
    
    uint8_t fifo_count_h, fifo_count_l;
    uint16_t fifo_count;
    uint8_t buffer[6]; // Макс. размер фрейма
    int frames_read = 0;
    
    // 1. Читаем количество байт в FIFO
    if (HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, 0x72, 1, &fifo_count_h, 1, 10) != HAL_OK) return 0;
    if (HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, 0x73, 1, &fifo_count_l, 1, 10) != HAL_OK) return 0;
    fifo_count = (fifo_count_h << 8) | fifo_count_l;
    
    if (HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, MPU_ACCEL_OUT, 1, buffer, 6, 50) != HAL_OK) {
        return 0;
    }

    for(int i = 0; i < 3; i++) {
        data->accel[i] = (int16_t)((buffer[i*2] << 8) | buffer[i*2 + 1]) / MPU_ACCEL_DIV * G_ACCEL;
    }

    // Размер одного фрейма: 6 байт (GYRO)
    const uint8_t FRAME_SIZE = 6;
    
    // 2. Читаем полные фреймы (не больше max_frames)
    while (fifo_count >= FRAME_SIZE && frames_read < max_frames) {
        // Читаем фрейм целиком
        if (HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR, 0x74, 1, buffer, FRAME_SIZE, 50) != HAL_OK) {
            return 0;
        }
        
        // Парсим данные (порядок: GX, GY, GZ)
        // Все данные в формате big-endian
        for(int i = 0; i < 3; i++) {
            // data->accel[i] = (int16_t)((buffer[i*2] << 8) | buffer[i*2 + 1]) / MPU_ACCEL_DIV * G_ACCEL;
            data->gyro[i] = (int16_t)((buffer[i*2] << 8) | buffer[i*2 + 1]) / MPU_GYRO_DIV;
        }
        data->ready = 1;
        
        fifo_count -= FRAME_SIZE;
        frames_read++;
        
        // Указатели сдвигаем, если нужно читать массив
        data++;
    }
    

    uint8_t ctrl = 0x44; // FIFO_RST + FIFO_EN
    HAL_I2C_Mem_Write(&MPU_I2C, MPU_ADDR, MPU_USER_CTRL, 1, &ctrl, 1, 100);
    
    return frames_read;
}

// void MPU_ReadIfFail(void) {
//     if(!MPU_read_fail || MPU_read_data)
//         return;

//     MPU_read_fail = 0;
//     MPU_Read();
// }

uint8_t MPU_GetData(MPU_Data_t *data) {
    if(!MPU_init_state)
        return 1;

    MPU_Read();

    #ifdef MPU_I2C_DMA
        while(MPU_dma_state);
    #endif

    if(MPU_read_fail || !MPU_read_data)
        return 1;
        
    __disable_irq();
    int16_t raw_accel[3], raw_gyro[3], raw_temp;
    for(int i = 0; i < 3; i++) {
        raw_accel[i] = (int16_t)((MPU_data[i*2] << 8) | MPU_data[i*2 + 1]);
        raw_gyro[i]  = (int16_t)((MPU_data[i*2 + 8] << 8) | MPU_data[i*2 + 9]);
    }
    raw_temp = (int16_t)((MPU_data[6] << 8) | MPU_data[7]);
    __enable_irq();

    for(int i = 0; i < 3; i++) {
        data->accel[i] = raw_accel[i] / MPU_ACCEL_DIV * G_ACCEL;
        data->gyro[i]  = raw_gyro[i] / MPU_GYRO_DIV;
    }
    data->temp = raw_temp / 340.0f + 36.53f;
        
    MPU_read_data = 0;
    data->ready = 1;

    return 0;
}

void MPU_CalcAnglesFromAccel(const MPU_Data_t* data, MPU_Angles_t* angles) {
    if (!data || !angles || !data->ready) return;

    float ax = -data->accel[1];
    float ay = -data->accel[2];
    float az = data->accel[0];

    // 1. Сырые углы из вектора гравитации
    float raw_roll  = atan2f(ay, az);
    float raw_pitch = atan2f(-ax, sqrtf(ay * ay + az * az));

    // 2. Радианы → Градусы
    const float RAD_TO_DEG = 57.295779513f;
    raw_roll  *= RAD_TO_DEG;
    raw_pitch *= RAD_TO_DEG;

    // 3. Экспоненциальный НЧ-фильтр (убирает шум/вибрации)
    // Используем static для сохранения состояния между вызовами
    static float filt_roll  = 0.0f;
    static float filt_pitch = 0.0f;
    const float ALPHA = 0.25f; // 0.1 = сильное сглаживание, 0.3 = быстрая реакция

    filt_roll  = ALPHA * raw_roll  + (1.0f - ALPHA) * filt_roll;
    filt_pitch = ALPHA * raw_pitch + (1.0f - ALPHA) * filt_pitch;

    // 4. Запись результата
    angles->roll  = filt_roll;
    angles->pitch = filt_pitch;
}

// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
//     if (hi2c == accel._hi2c) {
// 		extern Flag tim_accel_flag;

// 		if(accel._check_flag != FLAG_SET && tim_accel_flag != FLAG_IN_PROCESSING) {
// 			int16_t accel_tmp;
// 			for(int i = 0; i < 3; i++) {
// 				accel_tmp = (accel._data_data[i*2] << 8) | accel._data_data[i*2+1];
// 				accel._sum_data[i] += accel_tmp;
// 			}
// 			accel._sum_count++;
// 		}

// 		HAL_I2C_Mem_Read_DMA(accel._hi2c, 0xD0, 0x3B, I2C_MEMADD_SIZE_8BIT, accel._data_data, 14);
//     }
// }


// void MPU::check() {
// 	if(_init_state != FLAG_SET) {
// 		return;
// 	}

// 	if(_check_flag == FLAG_ALT || (HAL_GetTick() - _prev_time) > 100) {
// 		_timer = DWT->CYCCNT;
// //		_check_flag = FLAG_SET;
// 	}
// 	if(_sum_count == 0) {
// 		if((HAL_GetTick() - _prev_time) > 100) {
// 			DMA_Restart();
// 		}
// 		return;
// 	}

// 	_check_flag = FLAG_SET;
// 	int16_t _tmp_data[3];
// 	for(int i = 0; i < 3; i++) {
// 		_tmp_data[i] = _sum_data[i] / _sum_count;
// 		_sum_data[i] = 0;
// 	}
// 	_sum_count = 0;
// 	_check_flag = FLAG_RESET;


// 	int16_t accel_axes[3];

// 	// Переводим в мм/с2 (±2g: 16384 LSB/g)
// 	for(int i = 0; i < 3; i++) {
// 		accel_axes[i] = (float)_tmp_data[i] / 16.384f * G_MPU * 10 - _accel_axes_offset[i];
// 	}

// 	// Скорость в см/с
// 	uint32_t dt = (DWT->CYCCNT - _timer) / (SystemCoreClock/1000000);
// 	_timer = DWT->CYCCNT;
// 	for(int i = 0; i < 3; i++) {
// 			_vel[i] += (float)accel_axes[i] / 10.0f * (dt / 1000.0f / 1000.0f);
// 	}
// 	_vel_total = sqrt(_vel[0]*_vel[0] + _vel[1]*_vel[1] + _vel[2]*_vel[2]);


// 	_accel_total_prev = _accel_total;
// 	_accel_total = sqrt(accel_axes[0]*accel_axes[0] + accel_axes[1]*accel_axes[1] +accel_axes[2]*accel_axes[2]);
// 	uint16_t d_accel = abs(_accel_total - _accel_total_prev);

// 	if(d_accel < D_MPU_MAX) {
// 		if(_accel_cur_counter < MPU_COUNT) {
// 			_accel_cur_counter++;
// 			for(int i = 0; i < 3; i++) {
// 				_accel_axes_sum[i] += accel_axes[i];
// 			}
// 		}
// 		else {
// 			for(int i = 0; i < 3; i++) {
// 				_accel_axes_offset[i] += _accel_axes_sum[i]/_accel_cur_counter;
// 				_accel_axes_sum[i] = 0;
// 				_vel[i] /= 2;
// 			}
// 			_accel_cur_counter = 0;
// 		}
// 	}
// 	else {
// 		if(_accel_cur_counter > 0) {
// 			_accel_cur_counter--;
// 			for(int i = 0; i < 3; i++) {
// 				_accel_axes_sum[i] -= accel_axes[i];
// 			}
// 		}
// 	}

// 	_prev_time = HAL_GetTick();
// }

void MPU_BusRecovery(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Настройка SCL (PB6) и SDA (PB7) как GPIO
    GPIO_InitStruct.Pin = MPU_SCL_PIN | MPU_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MPU_I2C_PORT, &GPIO_InitStruct);

    // Генерация 9 тактовых импульсов для "разблокировки" устройств
    for (int i = 0; i < MPU_DATA_LEN; i++) {
        HAL_GPIO_WritePin(MPU_I2C_PORT, MPU_SCL_PIN, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(MPU_I2C_PORT, MPU_SCL_PIN, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // Стоп-условие
    HAL_GPIO_WritePin(MPU_I2C_PORT, MPU_SDA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MPU_I2C_PORT, MPU_SCL_PIN, GPIO_PIN_SET);
    HAL_Delay(1);

    // Восстановление альтернативной функции
    HAL_I2C_MspInit(&MPU_I2C);
}

// void MPU::DMA_Restart() {
// 	do {
// 		HAL_DMA_DeInit(_hi2c->hdmatx);
// 		HAL_I2C_DeInit(_hi2c);

// 		busRecovery();
// 		HAL_I2C_Init(_hi2c);
// 	}
//     while(init() != 0);
// }
