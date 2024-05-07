/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

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
extern uint16_t Data[4];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RX2_DATA[RX2_LEN] = {0};
uint8_t RX2_OFFSET = 0;
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
//HAL_UART_Receive_IT(&huart2, (uint8_t *)Data, 4);  // 开启usart2 接收中断
//HAL_UART_Receive_DMA(&huart2, (uint8_t *)Data, 4);  //启用usart2 dma功能

HAL_UART_Receive_DMA(&huart2, (uint8_t *)RX2_DATA, RX2_LEN); //使能uart2，dma功能
__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);  //使能uart2， idle中断
printf(" PWR 测试程序\n");
HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);  //开启wakeup 引脚唤醒功能
//HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"USART DMA TEST", sizeof("USART DMA TEST"));


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
   
    //HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);   //设置WAKEUP用于唤醒
    //HAL_PWR_EnterSTANDBYMode();     //进入待机模式

		LED_SwitchStatus();
		HAL_Delay(2000);
		LED_SwitchStatus();
		printf("system running\n");
		
		LED_SwitchStatus();
		HAL_SuspendTick();  //暂停嘀嗒定时器
		printf("system standby\n");
		
		
		SystemClock_Config();   // 从新初始化时钟
		
		HAL_ResumeTick();  // 恢复滴答定时器
		printf("system wakeup\n");
		
		// 不恢复嘀嗒定时器，将不再执行while循环
		
		printf("sysclk = %d hclk = %d pclk:%d pclk2:%d\n source:%d \n ", HAL_RCC_GetSysClockFreq(),HAL_RCC_GetHCLKFreq(),
		HAL_RCC_GetPCLK1Freq(), HAL_RCC_GetPCLK2Freq(), __HAL_RCC_GET_SYSCLK_SOURCE());
     __HAL_RCC_PWR_CLK_ENABLE();     //使能PWR时钟
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);      //清除Wake_UP标志
		HAL_PWR_EnterSTANDBYMode();  // 待机（关机）模式
		/* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//LED_SwitchStatus();
		//printf("白哥测试\n");
		//HAL_Delay(1000);
		
		
		
		//HAL_Delay(500);
		
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
