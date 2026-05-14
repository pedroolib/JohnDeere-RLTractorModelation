/* ******************************************* START *********************************************** */
/* Libraries, Definitions and Global Declarations */
#include <stdint.h> // 							standard integer library
#include "main.h"
#include "lcd.h"
#include "delay.h"

/* Superloop structure */
int main(void)
{
	/* Declarations and Initializations */
	USER_SystemClock_Config( ); // 				configure the system clock to 64 MHz
	USER_GPIO_Init( ); // 						initialize GPIOA pin 5 as output (for LD2)
	USER_ADC_Init( ); // 						initialize ADC1 for potentiometer on PA0

	// Wait for LCD power stabilization (>100ms)
	for( int i = 0; i < 100; i++ )
		USER_TIM_Delay_1ms( );

	LCD_Init( ); // 						initialize LCD display

	/* Show acceleration on LCD */
	LCD_Clear( );
	LCD_Set_Cursor( 1, 1 );
	LCD_Put_Str( "ACC:  0%" );
	LCD_Set_Cursor( 2, 1 );
	LCD_Put_Str( "Gear:3" );

    /* Repetitive block */
    for(;;){
    	uint16_t adc_val = USER_ADC_Read( );//		read potentiometer value (0-4095)
    	uint16_t acc_pct = ( adc_val * 100 ) / 4095;//	convert to 0-100%

    	// Update acceleration percentage on LCD with fixed width
    	LCD_Set_Cursor( 1, 6 );//				position cursor after "ACC: "
    	if( acc_pct < 100 )
    		LCD_Put_Char( ' ' );
    	if( acc_pct < 10 )
    		LCD_Put_Char( ' ' );
    	LCD_Put_Num( acc_pct );

    	GPIOA->ODR	^=	( 0x1UL <<  5U );//		value to toggle pin 5 of Port A (Toggle LD2)
    	USER_Delay_1sec( ); // 					delay function to create a visible blinking effect on LD2
    }
}

void USER_GPIO_Init( void ){
	// RCC_APB2ENR modified to IO port A clock enable
	RCC->APB2ENR	= RCC->APB2ENR//			RCC_APB2ENR actual value
					|//								to set
					( 0x1UL << 2U );//				(mask) IOPAEN bit

	// GPIOx_ODR modified to reset pin 5 of port A (LD2 is connected to PA5)
	GPIOA->ODR		= GPIOA->ODR//					GPIOx_ODR actual value
					&//								to clear
					~( 0x1UL << 5U );//				(mask) ODR5 bit

	// GPIOx_CRL modified to configure pin5 as output
	GPIOA->CRL		=	GPIOA->CRL//					GPIOx_CRL actual value
					&//								to clear
					~( 0x3UL << 22U )//				(mask) CNF5[1:0] bits
					&//								to clear
					~( 0x2UL << 20U );//				(mask) MODE5_1 bit

	// GPIOx_CRL modified to select pin5 max speed of 10MHz
	GPIOA->CRL		=	GPIOA->CRL//					GPIOx_CRL actual value
					|//								to set
					( 0x1UL << 20U );//				(mask) MODE5_0 bit
}

void USER_SystemClock_Config( void ){
	FLASH->ACR	&=	~( 0x5UL <<  0U );//			two wait states latency, if SYSCLK > 48MHz
	FLASH->ACR	|=	 ( 0x2UL <<  0U );//			two wait states latency, if SYSCLK > 48MHz
	RCC->CFGR	&=	~( 0x1UL << 16U )//				PLL HSI oscillator clock /2 selected as PLL input clock
				&	~( 0x7UL << 11U )// 				APB2 prescaler /1
				&	~( 0x3UL <<  8U );// 				APB1 prescaler /2
	RCC->CFGR	|=	 ( 0xFUL << 18U )//				PLL input clock x 16 (PLLMUL bits)
				|	 ( 0x4UL <<  8U );//				APB1 prescaler /2
	RCC->CR		|=	 ( 0x1UL << 24U );//				PLL2 ON
	while( !(RCC->CR & ~( 0x1UL << 25U )));//				wait until PLL is locked
	RCC->CFGR	&=	~( 0x1UL << 0U  );//				PLL used as system clock (SW bits)
	RCC->CFGR	|=	 ( 0x2UL << 0U  );//				PLL used as system clock (SW bits)
	while( 0x8UL != ( RCC->CFGR & 0xCUL ));//				wait until PLL is switched
}

void USER_Delay_1sec( void ){
	__asm(" 		ldr r0, =7111111UL	");//				load the value to be used as delay count
	__asm(" loop_1sec:\tsub r0, r0, #1\t\t");//				decrement the delay count
	__asm("         \tcmp r0, #0          ");//				check if the delay count has reached zero
	__asm("         \tbne loop_1sec       ");//				if not, repeat the process
	__asm("         \tnop                 ");//				no operation (to ensure exact timing)
}

void USER_ADC_Init( void ){
	// RCC_APB2ENR modified to enable ADC1 clock (IOPAEN already enabled in GPIO_Init)
	RCC->APB2ENR	= RCC->APB2ENR//			RCC_APB2ENR actual value
					|//								to set
					( 0x1UL << 9U );//				(mask) ADC1EN bit

	// GPIOx_CRL modified to configure pin0 as analog input (CNF=00, MODE=00)
	GPIOA->CRL		=	GPIOA->CRL//					GPIOx_CRL actual value
					&//								to clear
					~( 0x3UL << 2U )//				(mask) CNF0[1:0] bits
					&//								to clear
					~( 0x3UL << 0U );//				(mask) MODE0[1:0] bits

	// ADC1_SMPR2: set sample time for channel 0 to 239.5 cycles (max precision)
	ADC1->SMPR2		|=	( 0x7UL << 0U );

	// ADC1_SQR1: set sequence length to 1 conversion (L=0)
	ADC1->SQR1		&=	~( 0xFUL << 20U );

	// ADC1_SQR3: set first conversion in sequence to channel 0
	ADC1->SQR3		&=	~( 0x1FUL << 0U );

	// ADC1_CR2: turn on ADC (ADON=1)
	ADC1->CR2		|=	( 0x1UL << 0U );

	USER_TIM_Delay_1ms( );//						wait for ADC stabilization (~1 ms)

	// ADC1_CR2: reset calibration registers (RSTCAL=1)
	ADC1->CR2		|=	( 0x1UL << 3U );
	while( ADC1->CR2 & ( 0x1UL << 3U ) );//			wait until RSTCAL is cleared by hardware

	// ADC1_CR2: start calibration (CAL=1)
	ADC1->CR2		|=	( 0x1UL << 2U );
	while( ADC1->CR2 & ( 0x1UL << 2U ) );//			wait until CAL is cleared by hardware
}

uint16_t USER_ADC_Read( void ){
	// ADC1_CR2: start conversion by setting ADON a second time
	ADC1->CR2		|=	( 0x1UL << 0U );

	// Wait until End of Conversion (EOC) flag is set (bit 1 of SR)
	while( !( ADC1->SR & ( 0x1UL << 1U ) ) );

	// Return the 12-bit result from Data Register
	return ( uint16_t )( ADC1->DR & 0x0FFFU );
}
