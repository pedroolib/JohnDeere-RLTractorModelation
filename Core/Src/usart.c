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
	char tmpbuf[6];
	uint8_t idx = 0;
	uint8_t i;
	uint8_t cs;
	uint32_t rpm;
	uint8_t gear;
	uint16_t speed_whole;
	uint16_t speed_frac;
	uint32_t rv;
	uint16_t sv;
	uint8_t ti;

	/* Sanitizar valores */
	rpm = ( uint32_t )output->engine_rpm;
	if( rpm > 9999 ) rpm = 9999;

	gear = ( uint8_t )( output->gear + 0.5 );
	if( gear < 1 ) gear = 1;
	if( gear > 4 ) gear = 4;

	if( output->vehicle_speed < 0.0 ){
		speed_whole = 0;
		speed_frac  = 0;
	} else {
		speed_whole = ( uint16_t )output->vehicle_speed;
		speed_frac  = ( uint16_t )(( output->vehicle_speed - ( double )speed_whole ) * 10.0 );
	}

	/* Construir trama NMEA en buffer local */
	line[idx++] = '$';
	line[idx++] = 'T';
	line[idx++] = 'R';
	line[idx++] = ',';

	/* RPM a string */
	rv = rpm;
	ti = 0;
	if( rv == 0 ) tmpbuf[ti++] = '0';
	while( rv > 0 ){ tmpbuf[ti++] = '0' + (uint8_t)( rv % 10 ); rv /= 10; }
	while( ti > 0 ){ line[idx++] = tmpbuf[--ti]; }

	line[idx++] = ',';

	/* Speed whole a string */
	sv = speed_whole;
	ti = 0;
	if( sv == 0 ) tmpbuf[ti++] = '0';
	while( sv > 0 ){ tmpbuf[ti++] = '0' + (uint8_t)( sv % 10 ); sv /= 10; }
	while( ti > 0 ){ line[idx++] = tmpbuf[--ti]; }

	line[idx++] = '.';
	line[idx++] = '0' + (uint8_t)( speed_frac % 10 );
	line[idx++] = ',';
	line[idx++] = '0' + gear;

	/* Calcular checksum XOR sobre el cuerpo (sin $ ni *) */
	cs = 0;
	for( i = 1; i < idx; i++ ){
		cs ^= (uint8_t)line[i];
	}

	line[idx++] = '*';
	line[idx++] = "0123456789ABCDEF"[ ( cs >> 4 ) & 0x0F ];
	line[idx++] = "0123456789ABCDEF"[ cs & 0x0F ];
	line[idx++] = '\n';

	/* Enviar por USART */
	for( i = 0; i < idx; i++ ){
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
	/* Clear overrun error if present */
	if( USART1->SR & ( 1UL << 3U ) )
	{
		(void)USART1->DR; /* Read DR to clear ORE */
	}
	return -1; /* No data available */
}
