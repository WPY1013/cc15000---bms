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
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "rs485.h"
#include "bms.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
//static uint8_t rs485_data[200];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART3_UART_Init();
  MX_RS485_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	//初始化
	BMS_SendReadBasicInfo(BMS_NextSequence());//0D 01
	HAL_Delay(500);
	BMS_SendReadBasicInfo(BMS_NextSequence());//0D 01
	HAL_Delay(500);
	BMS_SendReadBasicInfo(BMS_NextSequence());//0D 01
	HAL_Delay(500);
	BMS_SendReadSerialNumber(BMS_NextSequence());//0D 04
	HAL_Delay(500);
	//初次认证
	BMS_SendAuthentication30(BMS_NextSequence());//类型30初始认证
	HAL_Delay(500);
	BMS_SendAuthentication30Session(BMS_NextSequence());//类型30会话认证
	HAL_Delay(500);
	BMS_SendAuthentication31(BMS_NextSequence());	//类型31认证
	HAL_Delay(500);
	BMS_SendAuthentication30Session(BMS_NextSequence());//类型30会话认证
	HAL_Delay(500);
	BMS_SendEnableCharging(BMS_NextSequence());//开启充电MOS
	HAL_Delay(500);
	BMS_SendAuthentication31(BMS_NextSequence());	//类型31认证
	HAL_Delay(500);
	BMS_SendAuthentication30Session(BMS_NextSequence());//类型30会话认证
	HAL_Delay(500);
	BMS_SendAuthentication31(BMS_NextSequence());	//类型31认证
	HAL_Delay(500);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//485回显测试
//    size_t received = Rs485_ReadData(rs485_data, sizeof(rs485_data));
//    if (received > 0U)
//    {
//      (void)Rs485_SendData(rs485_data, received);
//    }

		BMS_SendReadSerialNumber(BMS_NextSequence());//0D 04在线检测
		HAL_Delay(500);
		BMS_SendReadRealtimeStatus(BMS_NextSequence());//0D 02实时状态
		HAL_Delay(500);
		BMS_SendEnableCharging(BMS_NextSequence());//0D C0 保持充电状态
		HAL_Delay(500);
		BMS_SendReadCellVoltages(BMS_NextSequence());//0D 03单体电压
		HAL_Delay(500);
		BMS_SendReadHardwareParams(BMS_NextSequence());//0D C4充电参数
		HAL_Delay(500);
		BMS_SendReadExtendedStatus(BMS_NextSequence());//0D DA扩展参数
		HAL_Delay(500);
//		//停止充电
//		BMS_SendChargeStopped(BMS_NextSequence());//0D 12
//		HAL_Delay(500);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
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
