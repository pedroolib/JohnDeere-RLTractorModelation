#include <stdint.h>
#include "delay.h"

/* System clock is 64 MHz */
/* Each loop iteration takes approximately 5 cycles */

void USER_TIM2_Delay_40ms(void){
	__asm(" 		ldr r0, =512000UL	");
	__asm(" loop_40ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_40ms       ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_4_1ms(void){
	__asm(" 		ldr r0, =52480UL	");
	__asm(" loop_4_1ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_4_1ms      ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_1ms(void){
	__asm(" 		ldr r0, =12800UL	");
	__asm(" loop_1ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_1ms        ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_100us(void){
	__asm(" 		ldr r0, =1280UL\t\t");
	__asm(" loop_100us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_100us      ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_53us(void){
	__asm(" 		ldr r0, =680UL\t\t");
	__asm(" loop_53us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_53us       ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_10us(void){
	__asm(" 		ldr r0, =128UL\t\t");
	__asm(" loop_10us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_10us       ");
	__asm("         \tnop                 ");
}

void USER_Delay_1sec( void ){
	__asm(" 		ldr r0, =7111111UL	");
	__asm(" loop_1sec:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne loop_1sec       ");
	__asm("         \tnop                 ");
}
