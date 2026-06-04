#include <stdint.h>
#include "main.h"
#include "gpio.h"

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
