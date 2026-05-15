#ifndef MAIN_H_
#define MAIN_H_

/* Flash memory interface registers */
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

/* Reset and Clock Control registers */
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

/* General Purpose I/O registers */
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

/* Analog-to-Digital Converter registers */
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

/* Timer registers */
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

/* USART registers */
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

#define FLASH_BASE	0x40022000UL//		FLASH base address
#define RCC_BASE	0x40021000UL//		RCC base address
#define GPIOA_BASE	0x40010800UL//		GPIO Port A base address
#define GPIOB_BASE	0x40010C00UL//		GPIO Port B base address
#define GPIOC_BASE	0x40011000UL//		GPIO Port C base address
#define ADC1_BASE	0x40012400UL//		ADC1 base address
#define TIM2_BASE	0x40000000UL//		TIM2 base address
#define TIM3_BASE	0x40000400UL//		TIM3 base address
#define USART1_BASE	0x40013800UL//		USART1 base address

#define FLASH		(( FLASH_TypeDef *)FLASH_BASE )// 	FLASH base address points to FLASH structure
#define RCC         (( RCC_TypeDef *)RCC_BASE )//		RCC base address points to RCC structure
#define GPIOA		(( GPIO_TypeDef *)GPIOA_BASE )//	GPIO Port A base address points to GPIO structure
#define GPIOB		(( GPIO_TypeDef *)GPIOB_BASE )//	GPIO Port B base address points to GPIO structure
#define GPIOC		(( GPIO_TypeDef *)GPIOC_BASE )//	GPIO Port C base address points to GPIO structure
#define ADC1		(( ADC_TypeDef  *)ADC1_BASE  )//	ADC1 base address points to ADC structure
#define TIM2		(( TIM_TypeDef  *)TIM2_BASE  )//	TIM2 base address points to TIM structure
#define TIM3		(( TIM_TypeDef  *)TIM3_BASE  )//	TIM3 base address points to TIM structure
#define USART1		(( USART_TypeDef *)USART1_BASE )//	USART1 base address points to USART structure

/* NVIC registers */
#define NVIC_ISER0	(*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ICER0	(*(volatile uint32_t *)0xE000E180UL)
#define NVIC_ISPR0	(*(volatile uint32_t *)0xE000E200UL)
#define NVIC_ICPR0	(*(volatile uint32_t *)0xE000E280UL)

void USER_SystemClock_Config( void );
void USER_GPIO_Init( void );
void USER_Delay_1sec( void );
void USER_ADC_Init( void );
uint16_t USER_ADC_Read( void );
void USER_TIM2_Init( void );
void USER_Brake_Init( void );
uint8_t USER_Brake_Read( void );

#endif /* MAIN_H_ */