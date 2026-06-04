#include <stdint.h>
#include "main.h"
#include "adc.h"
#include "delay.h"

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
