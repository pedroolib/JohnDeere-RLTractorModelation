/* ******************************************* START *********************************************** */
/* Libraries, Definitions and Global Declarations */
#include <stdint.h>
#include "main.h"
#include "lcd.h"
#include "delay.h"
#include "EngTrModel.h"

/* Global variables for ISR-to-main communication */
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
	USER_ADC_Init( );
	USER_TIM2_Init( );
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

    /* Repetitive block */
    for(;;){
    	uint16_t rpm_i   = (uint16_t)g_engine_rpm;
    	uint16_t vel_i   = (uint16_t)g_vehicle_speed;
    	uint8_t  gear_i  = (uint8_t)g_gear;
    	uint16_t adc_pct;
    	if( g_brake == 0 ){
    		adc_pct = 0; /* BRAKE PRESSED: ignore potentiometer, show 0% */
    	} else {
    		adc_pct = (uint16_t)((g_adc_val * 100U) / 4095U);
    	}
    	uint8_t  brake_active = g_brake ? 0 : 1; /* Inverted: 0=pressed, 1=not pressed */

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

    	GPIOA->ODR	^=	( 0x1UL << 5U );
    	USER_Delay_1sec( );
    }
}

void USER_GPIO_Init( void ){
	/* Enable clocks for GPIOA and GPIOC */
	RCC->APB2ENR	=	RCC->APB2ENR
						| ( 0x1UL << 2U )  /* IOPAEN */
						| ( 0x1UL << 4U ); /* IOPCEN */

	/* PA5: Output for LD2 */
	GPIOA->ODR		&=	~( 0x1UL << 5U );
	GPIOA->CRL		&=	~( 0x3UL << 22U ) & ~( 0x2UL << 20U );
	GPIOA->CRL		|=	 ( 0x1UL << 20U );
}

void USER_ADC_Init( void ){
	/* Enable ADC1 clock */
	RCC->APB2ENR	|=	( 0x1UL << 9U );

	/* PA0: Analog input (CNF=00, MODE=00) */
	GPIOA->CRL		&=	~( 0x3UL << 2U ) & ~( 0x3UL << 0U );

	/* Sample time 239.5 cycles for channel 0 */
	ADC1->SMPR2		|=	( 0x7UL << 0U );

	/* Sequence length = 1, first channel = 0 */
	ADC1->SQR1		&=	~( 0xFUL << 20U );
	ADC1->SQR3		&=	~( 0x1FUL << 0U );

	/* Turn on ADC */
	ADC1->CR2		|=	( 0x1UL << 0U );

	USER_TIM_Delay_1ms( );

	/* Calibration */
	ADC1->CR2		|=	( 0x1UL << 3U );
	while( ADC1->CR2 & ( 0x1UL << 3U ) );
	ADC1->CR2		|=	( 0x1UL << 2U );
	while( ADC1->CR2 & ( 0x1UL << 2U ) );
}

uint16_t USER_ADC_Read( void ){
	ADC1->CR2		|=	( 0x1UL << 0U );
	while( !( ADC1->SR & ( 0x1UL << 1U ) ) );
	return ( uint16_t )( ADC1->DR & 0x0FFFU );
}

void USER_TIM2_Init( void ){
	/* Enable TIM2 clock (APB1) */
	RCC->APB1ENR	|=	( 0x1UL << 0U );

	/* TIM2 config for 40ms interrupt at APB1=32MHz */
	TIM2->PSC		=	6399;   /* 32MHz/6400 = 5kHz */
	TIM2->ARR		=	199;    /* 5000/200 = 25Hz = 40ms */
	TIM2->DIER		|=	( 0x1UL << 0U ); /* Update interrupt enable */
	TIM2->CR1		|=	( 0x1UL << 0U ); /* Counter enable */

	/* Enable TIM2 interrupt in NVIC (IRQ 28) */
	NVIC_ISER0		|=	( 0x1UL << 28U );
}

void USER_Brake_Init( void ){
	/* PA1: Input with pull-up (CNF=10, MODE=00) */
	GPIOA->CRL		&=	~( 0x3UL << 6U ); /* clear CNF1 */
	GPIOA->CRL		|=	 ( 0x2UL << 6U ); /* CNF1 = 10 (input pull-up/pull-down) */
	GPIOA->CRL		&=	~( 0x3UL << 4U ); /* MODE1 = 00 (input) */
	GPIOA->ODR		|=	 ( 0x1UL << 1U ); /* Pull-up enabled */
}

uint8_t USER_Brake_Read( void ){
	return ( uint8_t )(( GPIOA->IDR >> 1U ) & 0x1UL );
}

void USER_SystemClock_Config( void ){
	FLASH->ACR	&=	~( 0x5UL << 0U );
	FLASH->ACR	|=	 ( 0x2UL << 0U );
	RCC->CFGR	&=	~( 0x1UL << 16U )
					&	~( 0x7UL << 11U )
					&	~( 0x3UL <<  8U );
	RCC->CFGR	|=	 ( 0xFUL << 18U )
					|	 ( 0x4UL <<  8U );
	RCC->CR		|=	 ( 0x1UL << 24U );
	while( !(RCC->CR & ~( 0x1UL << 25U )));
	RCC->CFGR	&=	~( 0x1UL << 0U  );
	RCC->CFGR	|=	 ( 0x2UL << 0U  );
	while( 0x8UL != ( RCC->CFGR & 0xCUL ));
}

void USER_Delay_1sec( void ){
	__asm(" 		ldr r0, =7111111UL	");
	__asm(" loop_1sec:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_1sec       ");
	__asm("         \tnop                 ");
}

/* TIM2 Interrupt Handler - runs every 40ms */
void TIM2_IRQHandler(void){
	/* Clear update interrupt flag */
	TIM2->SR		&=	~( 0x1UL << 0U );

	/* Read sensors */
	g_adc_val		=	USER_ADC_Read( );
	g_brake			=	USER_Brake_Read( );

	/* Map inputs to model per documentation:
	 * Throttle: 1.5% to 100%
	 * BrakeTorque: 0 to 100%
	 */
	double throttle;
	if( g_brake == 0 ){
		/* BRAKE PRESSED: interrupt potentiometer, cut throttle to idle */
		throttle = 1.5;
		EngTrModel_U.BrakeTorque	=	100.0;
	} else {
		/* BRAKE NOT PRESSED: normal acceleration from potentiometer */
		throttle = (double)((g_adc_val * 100.0) / 4095.0);
		if( throttle < 1.5 ){
			throttle = 1.5; /* Minimum idle throttle to prevent stall */
		}
		EngTrModel_U.BrakeTorque	=	0.0;
	}
	EngTrModel_U.Throttle		=	throttle;

	/* Execute model step */
	EngTrModel_step( );

	/* Copy outputs */
	g_engine_rpm	=	EngTrModel_Y.EngineSpeed;
	g_vehicle_speed	=	EngTrModel_Y.VehicleSpeed;
	g_gear			=	EngTrModel_Y.Gear;
}
