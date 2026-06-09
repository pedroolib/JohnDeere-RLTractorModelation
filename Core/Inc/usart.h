#ifndef USART_H_
#define USART_H_

#include <stdint.h>
#include "app_tasks.h"

void USER_USART_Init( void );
void USER_USART_SendString( const char *str );
void USER_USART_SendTelemetry( const ModelOutput_t *output );

/* RX functions for remote control via USART1 (PA10) */
int16_t USER_USART1_GetChar( void );

#endif /* USART_H_ */
