/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for FreeRTOS application
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
#include "FreeRTOS.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
#include "lcd.h"
#include "delay.h"
#include "system_clock.h"
#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "tim2.h"
#include "tim3.h"
#include "brake.h"
#include "EngTrModel.h"

/* Private function prototypes -----------------------------------------------*/
void MX_FREERTOS_Init(void);
static void MX_Peripherals_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  USER_SystemClock_Config();

  /* Initialize all configured peripherals and application modules */
  MX_Peripherals_Init();

  /* Initialize the transmission model */
  EngTrModel_initialize();

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  while (1)
  {
  }
}

/**
  * @brief  Initialize peripherals and application modules.
  * @retval None
  */
static void MX_Peripherals_Init(void)
{
  USER_GPIO_Init();
  USER_USART_Init();
  USER_USART_SendString("HELLO_FROM_STM32_PA9\r\n");
  USER_ADC_Init();
  USER_TIM2_Init();
  USER_TIM3_Init();
  USER_Brake_Init();

  /* Wait for LCD power stabilization (>500ms) */
  for (int i = 0; i < 500; i++)
  {
    USER_TIM_Delay_1ms();
  }

  LCD_Init();
  USER_USART_SendString("DBG: LCD_Init done\r\n");

  /* Initial LCD display */
  LCD_Clear();
  LCD_Set_Cursor(1, 1);
  LCD_Put_Str("RPM:    V:  ");
  LCD_Set_Cursor(2, 1);
  LCD_Put_Str("G:1 A:0% B:0");
  USER_USART_SendString("DBG: LCD text written\r\n");
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
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
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */
