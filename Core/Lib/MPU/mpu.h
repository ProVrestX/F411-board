#ifndef INC_MPU_H_
#define INC_MPU_H_

#ifdef __cplusplus
extern C {
#endif

#include "main.h"


/* setup */
// #define MPU_I2C_DMA

#define MPU_I2C hi2c1
#define MPU_I2C_PORT GPIOB
#define MPU_SDA_PIN GPIO_PIN_7
#define MPU_SCL_PIN GPIO_PIN_6

#define MPU_MAX_INIT_ATTEMPT 5
#define MPU_ADDR 0xD0
#define MPU_WHO_AM_I_VALUE 0x68
#define MPU_DATA_LEN 14

#define MPU_ACCEL_DIV 16384.0f
#define MPU_GYRO_DIV 131.0f

#define G_ACCEL 9.815f

/* Registers */
#define MPU_SMPRT_DIV 0x19
#define MPU_CONFIG 0x1A
#define MPU_GYRO_CONFIG 0x1B
#define MPU_ACCEL_CONFIG 0x1C
#define MPU_FIFO_EN 0x23
#define MPU_INT_ENABLE 0x38

#define MPU_ACCEL_OUT 0x3B
#define MPU_TEMP_OUT 0x41
#define MPU_GYRO_OUT 0x43

#define MPU_SIGNAL_PATH_RESET 0x68
#define MPU_USER_CTRL 0x6A
#define MPU_PWR_MGMT_1 0x6B
#define MPU_PWR_MGMT_2 0x6C

#define MPU_FIFO_COUNT 0x72
#define MPU_FIFO_R_W 0x74

#define MPU_WHO_AM_I 0x75


typedef struct {
    float accel[3];
    float gyro[3];
    float temp;
    uint8_t ready;
} MPU_Data_t;

typedef struct {
    float pitch;
    float roll;
} MPU_Angles_t;

#ifdef MPU_I2C_DMA
    uint8_t MPU_WaitReady(void);
#endif
uint8_t MPU_Init(void);
uint8_t MPU_StartRead(void);
void MPU_Read(void);
uint8_t MPU_Read_FIFO(MPU_Data_t *data, uint8_t max_frames);
// void MPU_ReadIfFail(void);
uint8_t MPU_GetData(MPU_Data_t *data);
void MPU_CalcAnglesFromAccel(const MPU_Data_t* data, MPU_Angles_t* angles);
void MPU_BusRecovery(void);

// class Accel {
// public:
// 	Accel(I2C_HandleTypeDef *hi2c_accel);

// 	uint8_t init();
// 	void check();

// 	inline uint16_t getAccel() {return _accel_total;};
// 	inline uint16_t getVel() {return _vel_total;};

// 	inline void setState(AccelState new_state) {_state = new_state;};

// 	friend void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

// private:
// 	void busRecovery();
// 	void DMA_Restart();

// private:
// 	I2C_HandleTypeDef *_hi2c;

// 	Flag _init_state = FLAG_RESET;

// 	volatile Flag _check_flag = FLAG_ALT;
// 	volatile int32_t _sum_data[3] = {0};
// 	volatile Counter _sum_count = 0;

// 	AccelState _state = IS_STOP_READ;

// 	uint8_t _mpu_data[14];
// 	int16_t _accel_axes_offset[3];

// 	Time _timer = 0, _prev_time = 0;

// 	float _vel[3] = {0};
// 	uint16_t _vel_total = 0;

// 	uint16_t _accel_total = 0;
// 	uint16_t _accel_total_prev = 0;

// 	Counter _accel_cur_counter = 0;
// 	int32_t _accel_axes_sum[3] = {0};
// };

// extern Accel accel;


#ifdef __cplusplus
}
#endif

#endif /* INC_MPU_H_ */
