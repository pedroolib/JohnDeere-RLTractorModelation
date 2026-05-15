#include <stdint.h>
#include "main.h"
#include "lcd.h"
#include "delay.h"

//Caracter definido por usuario para cargar en la memoria CGRAM del LCD
const int8_t UserFont[8][8] =
{
		{ 0x11, 0x0A, 0x04, 0x1B, 0x11, 0x11, 0x11, 0x0E },
		{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 },
		{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },
		{ 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },
		{ 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E },
		{ 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

void LCD_Init(void){
	int8_t const *p;

	RCC->APB2ENR	|= 	( 0x1UL <<  4U );
	GPIOC->CRL		&=	~( 0x3UL << 30U ) & ~( 0x2UL << 28U )
					& 	~( 0x3UL << 26U ) & ~( 0x2UL << 24U );
	GPIOC->CRL 		|= 	 ( 0x1UL << 28U )
					| 	 ( 0x1UL << 24U );
	GPIOC->CRH		&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U )
					& 	~( 0x3UL << 14U ) & ~( 0x2UL << 12U )
					&	~( 0x3UL << 10U ) & ~( 0x2UL <<  8U )
					& 	~( 0x3UL <<  6U ) & ~( 0x2UL <<  4U )
					& 	~( 0x3UL <<  2U ) & ~( 0x2UL <<  0U );
	GPIOC->CRH		|= 	 ( 0x1UL << 16U )
					| 	 ( 0x1UL << 12U )
					| 	 ( 0x1UL <<  8U )
					| 	 ( 0x1UL <<  4U )
					| 	 ( 0x1UL <<  0U );

	GPIOC->BSRR	 =	LCD_RS_PIN_LOW;
	GPIOC->BSRR	 =	LCD_RW_PIN_LOW;
	GPIOC->BSRR	 =	LCD_EN_PIN_LOW;
	GPIOC->BSRR	 =	LCD_D4_PIN_LOW;
	GPIOC->BSRR	 =	LCD_D5_PIN_LOW;
	GPIOC->BSRR	 =	LCD_D6_PIN_LOW;
	GPIOC->BSRR	 =	LCD_D7_PIN_LOW;

	USER_TIM2_Delay_40ms( );

	LCD_Out_Data4( 0x03U );
	LCD_Pulse_EN( );
	USER_TIM_Delay_4_1ms( );

	LCD_Out_Data4( 0x03U );
	LCD_Pulse_EN( );
	USER_TIM_Delay_100us( );

	LCD_Out_Data4( 0x03U );
	LCD_Pulse_EN( );
	USER_TIM_Delay_53us( );

	LCD_Out_Data4( 0x02U );
	LCD_Pulse_EN( );
	USER_TIM_Delay_53us( );

	LCD_Write_Cmd( 0x28U );
	LCD_Write_Cmd( 0x08U );
	LCD_Write_Cmd( 0x01U );
	LCD_Write_Cmd( 0x06U );
	LCD_Write_Cmd( 0x0FU );

	LCD_Write_Cmd( 0x40U );
	p = &UserFont[0][0];
	for( int i = 0; i < (int)sizeof( UserFont ); i++, p++ )
		LCD_Put_Char( *p );

	LCD_Write_Cmd( 0x80U );
}

void LCD_Out_Data4(uint8_t val){
	if( ( val & 0x01U ) == 0x01U )
		GPIOC->BSRR	=	LCD_D4_PIN_HIGH;
	else
		GPIOC->BSRR	=	LCD_D4_PIN_LOW;

	if( ( val & 0x02U ) == 0x02U )
		GPIOC->BSRR	=	LCD_D5_PIN_HIGH;
	else
		GPIOC->BSRR	=	LCD_D5_PIN_LOW;

	if( ( val & 0x04U ) == 0x04U )
		GPIOC->BSRR	=	LCD_D6_PIN_HIGH;
	else
		GPIOC->BSRR	=	LCD_D6_PIN_LOW;

	if( ( val & 0x08U ) == 0x08U )
		GPIOC->BSRR	=	LCD_D7_PIN_HIGH;
	else
		GPIOC->BSRR	=	LCD_D7_PIN_LOW;
}

void LCD_Pulse_EN(void){
	GPIOC->BSRR	=	LCD_EN_PIN_LOW;
	USER_TIM_Delay_10us( );
	GPIOC->BSRR	=	LCD_EN_PIN_HIGH;
	USER_TIM_Delay_10us( );
	GPIOC->BSRR	=	LCD_EN_PIN_LOW;
	USER_TIM_Delay_53us( );
}

void LCD_Write_Byte(uint8_t val){
	LCD_Out_Data4( ( val >> 4 ) & 0x0FU );
	LCD_Pulse_EN( );
	LCD_Out_Data4( val & 0x0FU );
	LCD_Pulse_EN( );
	USER_TIM_Delay_53us( );
}

void LCD_Write_Cmd(uint8_t val){
	GPIOC->BSRR	=	LCD_RS_PIN_LOW;
	LCD_Write_Byte( val );
	if( val == 0x01U || val == 0x02U )
		USER_TIM_Delay_1ms( );
}

void LCD_Put_Char(uint8_t c){
	GPIOC->BSRR	=	LCD_RS_PIN_HIGH;
	LCD_Write_Byte( c );
}

void LCD_Set_Cursor(uint8_t line, uint8_t column){
	uint8_t address;
	column--;
	line--;
	address = ( line * 0x40U ) + column;
	address = 0x80U + ( address & 0x7FU );
	LCD_Write_Cmd( address );
}

void LCD_Put_Str(char * str){
	for( int16_t i = 0; i < 16 && str[ i ] != 0; i++ )
		LCD_Put_Char( str[ i ] );
}

void LCD_Put_Num(int16_t num){
	int16_t p;
	int16_t f = 0;
	int8_t ch[ 5 ];

	if( num == 0 ){
		LCD_Put_Char( '0' );
		return;
	}

	for( int16_t i = 0; i < 5; i++ ){
		p = 1;
		for( int16_t j = 4 - i; j > 0; j-- )
			p = p * 10;
		ch[ i ] = ( num / p );
		if( num >= p && !f )
			f = 1;
		num = num - ch[ i ] * p;
		ch[ i ] = ch[ i ] + 48;
		if( f )
			LCD_Put_Char( ch[ i ] );
	}
}

char LCD_Busy(void){
	GPIOC->CRH	&=	~( 0x2UL << 18U ) & ~( 0x3UL << 16U );
	GPIOC->CRH	|=	 ( 0x1UL << 18U );
	GPIOC->BSRR	 =	 LCD_RS_PIN_LOW;
	GPIOC->BSRR	 =	 LCD_RW_PIN_HIGH;
	GPIOC->BSRR	 =	 LCD_EN_PIN_HIGH;
	USER_TIM_Delay_100us( );
	if(( GPIOC->IDR & LCD_D7_PIN_HIGH )) {
		GPIOC->BSRR	=	LCD_EN_PIN_LOW;
		GPIOC->BSRR	=	LCD_RW_PIN_LOW;
		GPIOC->CRH	&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U );
		GPIOC->CRH	|=	 ( 0x1UL << 16U );
		return 1;
	} else {
		GPIOC->BSRR	=	LCD_EN_PIN_LOW;
		GPIOC->BSRR	=	LCD_RW_PIN_LOW;
		GPIOC->CRH	&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U );
		GPIOC->CRH	|=	 ( 0x1UL << 16U );
		return 0;
	}
}

void LCD_BarGraphic(int16_t value, int16_t size){
	value = value * size / 20;
	for( int16_t i = 0; i < size; i++ ){
		if( value > 5 ){
			LCD_Put_Char( 0x05U );
			value -= 5;
		} else {
			LCD_Put_Char( value );
			break;
		}
	}
}

void LCD_BarGraphicXY(int16_t pos_x, int16_t pos_y, int16_t value){
	LCD_Set_Cursor( pos_x, pos_y );
	for( int16_t i = 0; i < 16; i++ ){
		if( value > 5 ){
			LCD_Put_Char( 0x05U );
			value -= 5;
		} else {
			LCD_Put_Char( value );
			while( i++ < 16 )
				LCD_Put_Char( 0 );
		}
	}
}
