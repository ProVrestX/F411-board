/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "usbd_cdc_if.h"
#include "dwt_delay.h"
#include "sd_functions.h"
#include "st7735.h"
#include "flash.h"
#include "dht.h"
#include "mpu.h"
#include "encoder.h"
#include "joystick.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

char mesg[128] = {0};
uint8_t mesg_len = 0;



DHT_Data_t dht_data = {0};
MPU_Data_t mpu_data = {0};
Joystick_Data_t joy_data = {0};
MPU_Angles_t angles = {0};


// Буферы для углов (DMP выдаёт их в градусах)
float gs_pitch = 0.0f, gs_roll = 0.0f, gs_yaw = 0.0f;
int16_t accel_raw[3];
float accel_g[3];         
int16_t gyro_raw[3]; 
float gyro_dps[3];
int32_t quat[4];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int fd, unsigned char *buf, int len) {
    if(fd == 1 || fd == 2) {
        HAL_UART_Transmit(&huart1, buf, len, 100);
    }
    return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
    DWT_Delay_Init();
    HAL_TIM_Base_Start_IT(&htim5);

    // if(SD_Mount()) {
    //     Error_Handler();
    // }

    // if(W25Q_Init()) {
    //     Error_Handler();
    // }

    // W25Q_Init();
    // SD_Mount();

    ST7735_Init();
    ST7735_rotation(4);

    Joystick_Start();
    MPU_Init();

    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_2);

    // Encoder_SetClickFunc(test_clk);
    // Encoder_SetScrolFunc(test_scrol);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    // uint32_t mpu_time = 0;
    uint32_t ui_time = 0;
    while (1)
    {

        // if(HAL_GetTick() - mpu_time > 50) {
        //     mpu_time = HAL_GetTick();
        //     // MPU_Read_FIFO(mpu_data, 20);
        //     printf("MPU: %d\r\n", MPU_Read_FIFO(mpu_data, 100));
        //     printf("T: %lu\r\n", HAL_GetTick() - mpu_time);
        // }

        if(HAL_GetTick() - ui_time > 200) {
            ui_time = HAL_GetTick();
            MPU_GetData(&mpu_data);
            MPU_CalcAnglesFromAccel(&mpu_data, &angles);
            Joystick_GetData(&joy_data);
            DHT_Read(&dht_data);

            ST7735_ClearFrameBuffer();

            char data_str[64];
            sprintf(data_str, "Enc C=%d D=%d SW=%d", Encoder_GetCount(), Encoder_GetScrol(), Encoder_GetClick());
            ST7735_print(0, 5, ST7735_CYAN, 0, 0, &Font_7x9, 0, data_str);
            
            sprintf(data_str, "Joy X=%d Y=%d SW=%d", -joy_data.x, -joy_data.y, Joystick_GetClick());
            ST7735_print(0, 20, ST7735_CYAN, 0, 0, &Font_7x9, 0, data_str);

            sprintf(data_str, "Accel X=%d Y=%d Z=%d", -(int)(mpu_data.accel[1]*100), -(int)(mpu_data.accel[2]*100), (int)(-mpu_data.accel[0]*100));
            ST7735_print(0, 35, ST7735_CYAN, 0, 0, &Font_6x8, 0, data_str);

            sprintf(data_str, "Angle P=%d R=%d Temp=%d", (int)angles.pitch, (int)angles.roll, (int)(mpu_data.temp));
            ST7735_print(0, 50, ST7735_CYAN, 0, 0, &Font_6x8, 0, data_str);

            sprintf(data_str, "Temp=%d Hum=%d", (int)dht_data.temp, dht_data.hum);
            ST7735_print(0, 100, ST7735_CYAN, 0, 0, &Font_7x9, 0, data_str);

            sprintf(data_str, "Flash=%d SD=%d", W25Q_CheckState(), SD_CheckState());
            ST7735_print(0, 115, ST7735_CYAN, 0, 0, &Font_7x9, 0, data_str);

            ST7735_Update();

            HAL_GPIO_TogglePin(Led_GPIO_Port, Led_Pin);
            // printf("T: %lu\r\n", HAL_GetTick() - ui_time);
        }
        
        
        HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(Led_GPIO_Port, Led_Pin);
    DWT_DelayMs(50);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
