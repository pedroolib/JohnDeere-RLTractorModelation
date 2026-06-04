#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "usart.h"

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

	/* PA9 / USART1_TX: alternate-function push-pull, 10 MHz */
	GPIOA->CRH		&=	~( 0xFUL << 4U );
	GPIOA->CRH		|=	 ( 0x9UL << 4U );

	USART1->CR1		=	0;
	USART1->CR2		=	0;
	USART1->CR3		=	0;
	USART1->BRR		=	0x1A0BU; /* 64 MHz / 9600 baud */
	USART1->CR1		=	( 1UL << 13U ) | ( 1UL << 3U ); /* UE + TE */
}

void USER_USART_SendTelemetry( void )
{
	char line[32];

	uint32_t rpm = ( uint32_t )g_engine_rpm;
	uint8_t gear = ( uint8_t )( g_gear + 0.5 );

	int len = snprintf( line, sizeof( line ), "%lu,%.1f,%u\r\n",
		( unsigned long )rpm,
		g_vehicle_speed,
		( unsigned )gear );

	if( len <= 0 ){
		return;
	}

	for( int i = 0; i < len; i++ ){
		USER_USART_PutChar( line[i] );
	}
}
