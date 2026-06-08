/* ******************************************* START *********************************************** */
/* Libraries, Definitions and Global Declarations */
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "lcd.h"
#include "delay.h"
#include "EngTrModel.h"
#include "system_clock.h"
#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "tim2.h"
#include "tim3.h"
#include "brake.h"
#include "app_tasks.h"

/* Global variables for ISR-to-task communication.
 * In the FreeRTOS port these will be replaced by Queues / EventGroups.
 */
volatile uint16_t g_adc_val = 0;
volatile uint8_t  g_brake = 0;
volatile double   g_engine_rpm = 0.0;
volatile double   g_vehicle_speed = 0.0;
volatile double   g_gear = 0.0;

/* Superloop structure */
int main(void)
{
	/* Declarations and Initializations */
	USER_SystemClock_Config( );
	USER_GPIO_Init( );
	USER_USART_Init( );
	USER_USART_SendString( "HELLO_FROM_STM32_PA9\r\n" );
	USER_ADC_Init( );
	USER_TIM2_Init( );
	USER_TIM3_Init( );
	USER_Brake_Init( );

	/* Wait for LCD power stabilization (>500ms) */
	for( int i = 0; i < 500; i++ )
		USER_TIM_Delay_1ms( );

	LCD_Init( );
	USER_USART_SendString( "DBG: LCD_Init done\r\n" );

	/* Initialize transmission model */
	EngTrModel_initialize( );

	/* Initial LCD display */
	LCD_Clear( );
	LCD_Set_Cursor( 1, 1 );
	LCD_Put_Str( "RPM:    V:  " );
	LCD_Set_Cursor( 2, 1 );
	LCD_Put_Str( "G:1 A:0% B:0" );
	USER_USART_SendString( "DBG: LCD text written\r\n" );

    /* Repetitive block — simulated task scheduler */
    for(;;){
    	vTaskModelControl( );  /* Task 1: Model + PWM   (40 ms event-driven) */
    	vTaskDisplay( );       /* Task 2: LCD update    (runs every loop)     */
    	vTaskTelemetry( );     /* Task 3: USART tx      (runs every loop)     */
    	vTaskHeartbeat( );     /* Task 4: LED + 1s wait (pacing of superloop) */
    }
}
