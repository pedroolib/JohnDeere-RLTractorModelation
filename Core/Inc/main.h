#ifndef MAIN_H_
#define MAIN_H_

/* ------------------------------------------------------------------ */
/*  If building for STM32F103xB (CMSIS/HAL environment), pull in the   */
/*  official device header. It defines FLASH_TypeDef, RCC_TypeDef,   */
/*  GPIO_TypeDef, ADC_TypeDef, TIM_TypeDef, USART_TypeDef and the    */
/*  peripheral base addresses (FLASH_BASE, RCC_BASE, GPIOA_BASE,      */
/*  TIM2_BASE, USART1_BASE, …).                                      */
/* ------------------------------------------------------------------ */
#ifdef STM32F103xB
#include "stm32f1xx.h"
#ifdef USE_HAL_DRIVER
#include "stm32f1xx_hal.h"
#endif
#else
/* Fallback manual register definitions for non-CMSIS builds */
typedef struct{
	volatile uint32_t ACR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t AR;
	volatile uint32_t reserved;
	volatile uint32_t OBR;
	volatile uint32_t WRPR;
} FLASH_TypeDef;

typedef struct
{
	volatile uint32_t CR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
} RCC_TypeDef;

typedef struct
{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
	volatile uint32_t SR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMPR1;
	volatile uint32_t SMPR2;
	volatile uint32_t JOFR1;
	volatile uint32_t JOFR2;
	volatile uint32_t JOFR3;
	volatile uint32_t JOFR4;
	volatile uint32_t HTR;
	volatile uint32_t LTR;
	volatile uint32_t SQR1;
	volatile uint32_t SQR2;
	volatile uint32_t SQR3;
	volatile uint32_t JSQR;
	volatile uint32_t JDR1;
	volatile uint32_t JDR2;
	volatile uint32_t JDR3;
	volatile uint32_t JDR4;
	volatile uint32_t DR;
} ADC_TypeDef;

typedef struct
{
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMCR;
	volatile uint32_t DIER;
	volatile uint32_t SR;
	volatile uint32_t EGR;
	volatile uint32_t CCMR1;
	volatile uint32_t CCMR2;
	volatile uint32_t CCER;
	volatile uint32_t CNT;
	volatile uint32_t PSC;
	volatile uint32_t ARR;
	volatile uint32_t RCR;
	volatile uint32_t CCR1;
	volatile uint32_t CCR2;
	volatile uint32_t CCR3;
	volatile uint32_t CCR4;
	volatile uint32_t BDTR;
	volatile uint32_t DCR;
	volatile uint32_t DMAR;
} TIM_TypeDef;

typedef struct
{
	volatile uint32_t SR;
	volatile uint32_t DR;
	volatile uint32_t BRR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
	volatile uint32_t GTPR;
} USART_TypeDef;

#define FLASH_BASE	0x40022000UL
#define RCC_BASE	0x40021000UL
#define GPIOA_BASE	0x40010800UL
#define GPIOB_BASE	0x40010C00UL
#define GPIOC_BASE	0x40011000UL
#define ADC1_BASE	0x40012400UL
#define TIM2_BASE	0x40000000UL
#define TIM3_BASE	0x40000400UL
#define USART1_BASE	0x40013800UL
#define USART2_BASE	0x40004400UL

#define FLASH		(( FLASH_TypeDef *)FLASH_BASE )
#define RCC         (( RCC_TypeDef *)RCC_BASE )
#define GPIOA		(( GPIO_TypeDef *)GPIOA_BASE )
#define GPIOB		(( GPIO_TypeDef *)GPIOB_BASE )
#define GPIOC		(( GPIO_TypeDef *)GPIOC_BASE )
#define ADC1		(( ADC_TypeDef  *)ADC1_BASE  )
#define TIM2		(( TIM_TypeDef  *)TIM2_BASE  )
#define TIM3		(( TIM_TypeDef  *)TIM3_BASE  )
#define USART1		(( USART_TypeDef *)USART1_BASE )

#endif /* STM32F103xB */

/* NVIC registers (not provided as direct macros by CMSIS) */
#define NVIC_ISER0	(*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ICER0	(*(volatile uint32_t *)0xE000E180UL)
#define NVIC_ISPR0	(*(volatile uint32_t *)0xE000E200UL)
#define NVIC_ICPR0	(*(volatile uint32_t *)0xE000E280UL)

#endif /* MAIN_H_ */
