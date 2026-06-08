#include <stdint.h>
#include "main.h"
#include "system_clock.h"

void USER_SystemClock_Config( void ){
	FLASH->ACR	&=	~( 0x5UL << 0U );
	FLASH->ACR	|=	 ( 0x2UL << 0U );
	RCC->CFGR	&=	~( 0x1UL << 16U )
					&	~( 0x7UL << 11U )
					&	~( 0x3UL <<  8U );
	RCC->CFGR	|=	 ( 0xFUL << 18U )
					|	 ( 0x4UL <<  8U );
	RCC->CR		|=	 ( 0x1UL << 24U );
	while( !(RCC->CR & ( 0x1UL << 25U )));
	RCC->CFGR	&=	~( 0x1UL << 0U  );
	RCC->CFGR	|=	 ( 0x2UL << 0U  );
	while( 0x8UL != ( RCC->CFGR & 0xCUL ));
}
