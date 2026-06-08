#include <stdint.h>
#include "main.h"
#include "EngTrModel.h"
#include "lcd.h"
#include "usart.h"
#include "delay.h"
#include "app_tasks.h"

/* Simulated EventGroup variable.
 * In FreeRTOS this will be replaced by an actual EventGroupHandle_t.
 */
volatile uint32_t g_event_flags = 0U;

/* -----------------------------------------------------------
 * TASK 1: Model Control + PWM Output
 * Currently triggered by EVENT_MODEL_READY from TIM2 ISR.
 * In FreeRTOS this will run at 40 ms via vTaskDelayUntil().
 * ----------------------------------------------------------- */
void vTaskModelControl( void )
{
    double   throttle;
    uint16_t duty;

    /* Wait for the ISR to signal that new sensor data is ready */
    if( xEventGroupWaitBits( EVENT_MODEL_READY, 1U ) ){

        /* Map inputs to model per documentation:
         * Throttle: 1.5% to 100%
         * BrakeTorque: 0 to 100%
         */
        if( g_brake == 0 ){
            /* BRAKE PRESSED: interrupt potentiometer, cut throttle to idle */
            throttle = 1.5;
            EngTrModel_U.BrakeTorque = 100.0;
        } else {
            /* BRAKE NOT PRESSED: normal acceleration from potentiometer */
            throttle = (double)((g_adc_val * 100.0) / 4095.0);
            if( throttle < 1.5 ){
                throttle = 1.5; /* Minimum idle throttle to prevent stall */
            }
            EngTrModel_U.BrakeTorque = 0.0;
        }
        EngTrModel_U.Throttle = throttle;

        /* Execute model step */
        EngTrModel_step( );

        /* Copy outputs to global state (to be replaced by Queue in FreeRTOS) */
        g_engine_rpm    = EngTrModel_Y.EngineSpeed;
        g_vehicle_speed = EngTrModel_Y.VehicleSpeed;
        g_gear          = EngTrModel_Y.Gear;

        /* Update PWM duty cycle for all 4 motors */
        duty = (uint16_t)((g_vehicle_speed * 1000.0) / 120.0);
        if( duty > 1000 ) duty = 1000;
        /* Minimum kickstart duty to overcome static friction on DC motors */
        if( duty > 0 && duty < 100 ) duty = 100;

        TIM3->CCR1 = duty;
        TIM3->CCR2 = duty;
        TIM3->CCR3 = duty;
        TIM3->CCR4 = duty;
    }
}

/* -----------------------------------------------------------
 * TASK 2: Display
 * Reads global state and updates LCD.
 * In FreeRTOS this will run at 200 ms via vTaskDelayUntil().
 * ----------------------------------------------------------- */
void vTaskDisplay( void )
{
    uint16_t rpm_i;
    uint16_t vel_i;
    uint8_t  gear_i;
    uint16_t adc_pct;
    uint8_t  brake_active;

    rpm_i   = (uint16_t)g_engine_rpm;
    vel_i   = (uint16_t)g_vehicle_speed;
    gear_i  = (uint8_t)g_gear;

    if( g_brake == 0 ){
        adc_pct = 0; /* BRAKE PRESSED: ignore potentiometer, show 0% */
    } else {
        adc_pct = (uint16_t)((g_adc_val * 100U) / 4095U);
    }
    brake_active = g_brake ? 0 : 1; /* Inverted: 0=pressed, 1=not pressed */

    /* Update Line 1: RPM and Vehicle Speed */
    LCD_Set_Cursor( 1, 5 );
    LCD_Put_Num( rpm_i );
    LCD_Set_Cursor( 1, 12 );
    LCD_Put_Num( vel_i );

    /* Update Line 2: Gear, Acceleration and Brake */
    LCD_Set_Cursor( 2, 3 );
    LCD_Put_Num( gear_i );
    LCD_Set_Cursor( 2, 6 );
    if( adc_pct < 100 )
        LCD_Put_Char( ' ' );
    if( adc_pct < 10 )
        LCD_Put_Char( ' ' );
    LCD_Put_Num( adc_pct );
    LCD_Set_Cursor( 2, 12 );
    LCD_Put_Num( brake_active );
}

/* -----------------------------------------------------------
 * TASK 3: Telemetry
 * Sends data frame over USART.
 * In FreeRTOS this will run at 100 ms via vTaskDelayUntil().
 * ----------------------------------------------------------- */
void vTaskTelemetry( void )
{
    USER_USART_SendTelemetry( );
}

/* -----------------------------------------------------------
 * TASK 4: Heartbeat
 * Toggles on-board LED (PA5) and provides the 1-second pacing
 * of the current superloop.
 * In FreeRTOS this will run at 1000 ms via vTaskDelayUntil().
 * ----------------------------------------------------------- */
void vTaskHeartbeat( void )
{
    GPIOA->ODR ^= ( 0x1UL << 5U );
    USER_Delay_1sec( );
}
