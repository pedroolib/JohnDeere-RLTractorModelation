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

/* Global variables for ISR-to-main communication */
volatile uint16_t g_adc_val = 0;
volatile uint8_t  g_brake = 0;
volatile double   g_engine_rpm = 0.0;
volatile double   g_vehicle_speed = 0.0;
volatile double   g_gear = 0.0;
volatile uint8_t  g_model_ready = 0;

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

	/* Wait for LCD power stabilization (>100ms) */
	for( int i = 0; i < 100; i++ )
		USER_TIM_Delay_1ms( );

	LCD_Init( );

	/* Initialize transmission model */
	EngTrModel_initialize( );

	/* Initial LCD display */
	LCD_Clear( );
	LCD_Set_Cursor( 1, 1 );
	LCD_Put_Str( "RPM:    V:  " );
	LCD_Set_Cursor( 2, 1 );
	LCD_Put_Str( "G:1 A:0% B:0" );

    /* Local variables for the superloop */
    uint16_t rpm_i;
    uint16_t vel_i;
    uint8_t  gear_i;
    uint16_t adc_pct;
    uint8_t  brake_active;
    double   throttle;
    uint16_t duty;

    /* Repetitive block */
    for(;;){
    	/* Execute model step if IRQ requested it */
    	if( g_model_ready ){
    		if( g_brake == 0 ){
    			throttle = 1.5;
    			EngTrModel_U.BrakeTorque	=	100.0;
    		} else {
    			throttle = (double)((g_adc_val * 100.0) / 4095.0);
    			if( throttle < 1.5 ){
    				throttle = 1.5; /* Minimum idle throttle to prevent stall */
    			}
    			EngTrModel_U.BrakeTorque	=	0.0;
    		}
    		EngTrModel_U.Throttle		=	throttle;

    		EngTrModel_step( );

    		g_engine_rpm	=	EngTrModel_Y.EngineSpeed;
    		g_vehicle_speed	=	EngTrModel_Y.VehicleSpeed;
    		g_gear			=	EngTrModel_Y.Gear;

    		duty = (uint16_t)((g_vehicle_speed * 1000.0) / 120.0);
    		if( duty > 1000 ) duty = 1000;

    		gear_i = (uint8_t)(g_gear + 0.5);
    		if( gear_i < 1 ) gear_i = 1;
    		if( gear_i > 4 ) gear_i = 4;

    		TIM3->CCR1 = duty;
    		TIM3->CCR2 = duty;
    		TIM3->CCR3 = duty;
    		TIM3->CCR4 = duty;

    		g_model_ready = 0;
    	}

    	rpm_i   = (uint16_t)g_engine_rpm;
    	vel_i   = (uint16_t)g_vehicle_speed;
    	gear_i  = (uint8_t)g_gear;
    	if( g_brake == 0 ){
    		adc_pct = 0; /* BRAKE PRESSED: ignore potentiometer, show 0% */
    	} else {
    		adc_pct = (uint16_t)((g_adc_val * 100U) / 4095U);
    	}
    	brake_active = g_brake ? 0 : 1; /* Inverted: 0=pressed, 1=not pressed */

    	/* Update Line 1: RPM and Vehicle Speed */
    	LCD_Set_Cursor( 1, 5 );
    	LCD_Put_Num( rpm_i );
    	LCD_Set_Cursor( 1, 12 );
    	LCD_Put_Num( vel_i );

    	/* Update Line 2: Gear, Acceleration and Brake */
    	LCD_Set_Cursor( 2, 3 );
    	LCD_Put_Num( gear_i );
    	LCD_Set_Cursor( 2, 6 );
    	if( adc_pct < 100 )
    		LCD_Put_Char( ' ' );
    	if( adc_pct < 10 )
    		LCD_Put_Char( ' ' );
    	LCD_Put_Num( adc_pct );
    	LCD_Set_Cursor( 2, 12 );
    	LCD_Put_Num( brake_active );

    	/* Send telemetry from the superloop, not from TIM2 interrupt */
    	USER_USART_SendTelemetry( );

    	GPIOA->ODR	^=	( 0x1UL << 5U );
    	USER_Delay_1sec( );
    }
}
