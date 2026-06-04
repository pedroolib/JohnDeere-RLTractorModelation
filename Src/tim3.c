#include <stdint.h>
#include "main.h"
#include "tim3.h"

void USER_TIM3_Init( void ){
	/* Enable TIM3 clock (APB1) and GPIOB clock (APB2) */
	RCC->APB1ENR	|=	( 0x1UL << 1U );  /* TIM3EN */
	RCC->APB2ENR	|=	( 0x1UL << 3U );  /* IOPBEN */

	/* PA6 (TIM3_CH1): Alternate function push-pull, 10MHz (CNF=10, MODE=01) */
	GPIOA->CRL		&=	~( 0x3UL << 30U ) & ~( 0x3UL << 28U );
	GPIOA->CRL		|=	 ( 0x2UL << 30U ) | ( 0x1UL << 28U );
	/* PA7 (TIM3_CH2): Alternate function push-pull, 10MHz (CNF=10, MODE=01) */
	GPIOA->CRL		&=	~( 0x3UL << 26U ) & ~( 0x3UL << 24U );
	GPIOA->CRL		|=	 ( 0x2UL << 26U ) | ( 0x1UL << 24U );

	/* PB0 (TIM3_CH3): Alternate function push-pull, 10MHz (CNF=10, MODE=01) */
	GPIOB->CRL		&=	~( 0x3UL << 2U ) & ~( 0x3UL << 0U );
	GPIOB->CRL		|=	 ( 0x2UL << 2U ) | ( 0x1UL << 0U );
	/* PB1 (TIM3_CH4): Alternate function push-pull, 10MHz (CNF=10, MODE=01) */
	GPIOB->CRL		&=	~( 0x3UL << 6U ) & ~( 0x3UL << 4U );
	GPIOB->CRL		|=	 ( 0x2UL << 6U ) | ( 0x1UL << 4U );

	/* TIM3 config: 64MHz / 64 = 1MHz, ARR=999 -> 1kHz PWM frequency */
	TIM3->PSC		=	63;
	TIM3->ARR		=	999;

	/* PWM Mode 1 on all 4 channels */
	/* CH1: PWM Mode 1 (OC1M = 110), preload enable */
	TIM3->CCMR1		&=	~( 0x7UL << 4U );
	TIM3->CCMR1		|=	 ( 0x6UL << 4U ) | ( 0x1UL << 3U );
	/* CH2: PWM Mode 1 (OC2M = 110), preload enable */
	TIM3->CCMR1		&=	~( 0x7UL << 12U );
	TIM3->CCMR1		|=	 ( 0x6UL << 12U ) | ( 0x1UL << 11U );
	/* CH3: PWM Mode 1 (OC3M = 110), preload enable */
	TIM3->CCMR2		&=	~( 0x7UL << 4U );
	TIM3->CCMR2		|=	 ( 0x6UL << 4U ) | ( 0x1UL << 3U );
	/* CH4: PWM Mode 1 (OC4M = 110), preload enable */
	TIM3->CCMR2		&=	~( 0x7UL << 12U );
	TIM3->CCMR2		|=	 ( 0x6UL << 12U ) | ( 0x1UL << 11U );

	/* Enable all 4 channel outputs */
	TIM3->CCER		|=	( 0x1UL << 0U ) | ( 0x1UL << 4U )
					| ( 0x1UL << 8U ) | ( 0x1UL << 12U );

	/* Enable counter and auto-reload preload */
	TIM3->CR1		|=	( 0x1UL << 7U ) | ( 0x1UL << 0U );
}
