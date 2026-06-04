#include <stdint.h>
#include "main.h"
#include "brake.h"

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
