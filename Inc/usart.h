#ifndef USART_H_
#define USART_H_

void USER_USART_Init( void );
void USER_USART_SendString( const char *str );
void USER_USART_SendTelemetry( void );

#endif /* USART_H_ */
