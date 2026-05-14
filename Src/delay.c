#include <stdint.h>
#include "delay.h"

/* System clock is 64 MHz */
/* Each loop iteration takes approximately 5 cycles:
 * sub(1) + cmp(1) + bne(2~3) + nop(1) ≈ 5 cycles
 * 64 MHz => 1 cycle ≈ 15.625 ns
 * 5 cycles ≈ 78.125 ns per iteration
 */

void USER_TIM2_Delay_40ms(void){
	__asm(" 		ldr r0, =512000UL	");//	~40 ms @ 64 MHz
	__asm(" again_40ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_40ms      ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_4_1ms(void){
	__asm(" 		ldr r0, =52480UL	");//	~4.1 ms @ 64 MHz
	__asm(" again_4_1ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_4_1ms     ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_1ms(void){
	__asm(" 		ldr r0, =12800UL	");//	~1 ms @ 64 MHz
	__asm(" again_1ms:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_1ms       ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_100us(void){
	__asm(" 		ldr r0, =1280UL\t\t");//	~100 us @ 64 MHz
	__asm(" again_100us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_100us     ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_53us(void){
	__asm(" 		ldr r0, =680UL\t\t");//	~53 us @ 64 MHz
	__asm(" again_53us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_53us      ");
	__asm("         \tnop                 ");
}

void USER_TIM_Delay_10us(void){
	__asm(" 		ldr r0, =128UL\t\t");//	~10 us @ 64 MHz
	__asm(" again_10us:\tsub r0, r0, #1\t\t");
	__asm("         \tcmp r0, #0          ");
	__asm("         \tbne again_10us      ");
	__asm("         \tnop                 ");
}
