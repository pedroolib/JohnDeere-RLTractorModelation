#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "usart.h"
#include "app_tasks.h"

static void USER_USART_PutChar( char c )
{
	while( !( USART1->SR & ( 1UL << 7U ) ) ); /* Wait for TXE */
	USART1->DR = ( uint16_t )( uint8_t )c;
}

void USER_USART_SendString( const char *str )
{
	while( *str ){
		USER_USART_PutChar( *str++ );
	}
}

void USER_USART_Init( void )
{
	/* USART1 is on APB2. APB2 = 64 MHz. Baud = 9600. */
	RCC->APB2ENR	|=	( 1UL << 0U );   /* AFIOEN */
	RCC->APB2ENR	|=	( 1UL << 14U );  /* USART1EN */

	/* PA9 / USART1_TX: alternate-function push-pull, 10 MHz (CNF=10, MODE=01) */
	GPIOA->CRH		&=	~( 0xFUL << 4U );
	GPIOA->CRH		|=	 ( 0x9UL << 4U );

	/* PA10 / USART1_RX: input floating (CNF=01, MODE=00) */
	GPIOA->CRH		&=	~( 0xFUL << 8U );
	GPIOA->CRH		|=	 ( 0x4UL << 8U ); /* CNF=01 (floating input), MODE=00 */

	USART1->CR1		=	0;
	USART1->CR2		=	0;
	USART1->CR3		=	0;
	USART1->BRR		=	0x1A0BU; /* 64 MHz / 9600 baud */
	/* UE=1, TE=1, RE=1 (enable USART, transmitter and receiver) */
	USART1->CR1		=	( 1UL << 13U ) | ( 1UL << 3U ) | ( 1UL << 2U );
}

void USER_USART_SendTelemetry( const ModelOutput_t *output )
{
	char line[32];
	int len;

	uint32_t rpm = ( uint32_t )output->engine_rpm;
	uint8_t gear = ( uint8_t )( output->gear + 0.5 );

	/* Descomponer velocidad en parte entera y 1 decimal (sin usar %f) */
	uint16_t speed_whole = ( uint16_t )output->vehicle_speed;
	uint16_t speed_frac  = ( uint16_t )(( output->vehicle_speed - ( double )speed_whole ) * 10.0 );

	len = snprintf( line, sizeof( line ), "%lu,%u.%u,%u\n",
		( unsigned long )rpm,
		( unsigned )speed_whole,
		( unsigned )speed_frac,
		( unsigned )gear );

	if( len <= 0 ){
		return;
	}

	for( int i = 0; i < len; i++ ){
		USER_USART_PutChar( line[i] );
	}
}

int16_t USER_USART1_GetChar( void )
{
	/* Check RXNE (Read Data Register Not Empty) */
	if( USART1->SR & ( 1UL << 5U ) )
	{
		return (int16_t)( USART1->DR & 0xFFU );
	}
	return -1; /* No data available */
}
