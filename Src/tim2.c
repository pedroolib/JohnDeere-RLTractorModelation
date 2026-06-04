#include <stdint.h>
#include "main.h"
#include "tim2.h"
#include "adc.h"
#include "brake.h"

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

/* TIM2 Interrupt Handler - runs every 40ms */
void TIM2_IRQHandler(void){
	/* Clear update interrupt flag */
	TIM2->SR		&=	~( 0x1UL << 0U );

	/* Read sensors and signal main() to run the model */
	g_adc_val		=	USER_ADC_Read( );
	g_brake			=	USER_Brake_Read( );
	g_model_ready	=	1;
}
