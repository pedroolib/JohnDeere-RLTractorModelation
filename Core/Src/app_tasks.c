#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "EngTrModel.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "brake.h"
#include "app_tasks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Queue handles - definidos aqui para que sean visibles entre tareas */
QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xModelToDisplayQueue = NULL;
QueueHandle_t xModelToTelemetryQueue = NULL;

/* ================================================================
 *  TAREA 1: SENSOR (Periodo 40 ms, Prioridad maxima - RMS)
 *  Lee ADC y freno, envia datos por Queue a ModelControl.
 * ================================================================ */
void vTaskSensor( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SensorData_t data;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_SENSOR_MS ) );

        /* Deadline: lectura de sensores y envio por Queue */
        data.adc   = USER_ADC_Read();
        data.brake = USER_Brake_Read();

        /* Sobrescribe para que ModelControl siempre vea el ultimo dato */
        xQueueOverwrite( xSensorQueue, &data );
    }
}

/* ================================================================
 *  TAREA 2: MODEL CONTROL (Periodo 40 ms, Prioridad alta)
 *  Recibe datos de Sensor, ejecuta modelo, envia resultados
 *  por Queue a Display y Telemetry.
 * ================================================================ */
void vTaskModelControl( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SensorData_t sensorData;
    ModelOutput_t output;
    double throttle;
    uint16_t duty;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_MODEL_MS ) );

        /* Deadline: recibir datos, procesar modelo, enviar resultados */
        if( xQueueReceive( xSensorQueue, &sensorData, 0 ) == pdTRUE )
        {
            /* Mapeo de entradas al modelo */
            if( sensorData.brake == 0 )
            {
                throttle = 1.5;
                EngTrModel_U.BrakeTorque = 100.0;
            }
            else
            {
                throttle = (double)((sensorData.adc * 100.0) / 4095.0);
                if( throttle < 1.5 )
                {
                    throttle = 1.5;
                }
                EngTrModel_U.BrakeTorque = 0.0;
            }
            EngTrModel_U.Throttle = throttle;

            /* Ejecutar paso del modelo */
            EngTrModel_step();

            /* Preparar salida para otras tareas */
            output.engine_rpm    = EngTrModel_Y.EngineSpeed;
            output.vehicle_speed = EngTrModel_Y.VehicleSpeed;
            output.gear          = EngTrModel_Y.Gear;

            if( sensorData.brake == 0 )
            {
                output.adc_pct = 0;
            }
            else
            {
                output.adc_pct = (uint16_t)((sensorData.adc * 100U) / 4095U);
            }
            output.brake_active = sensorData.brake ? 0 : 1; /* 0=pressed, 1=not pressed */

            /* Enviar resultados a Display y Telemetry via Queues */
            xQueueOverwrite( xModelToDisplayQueue,   &output );
            xQueueOverwrite( xModelToTelemetryQueue, &output );

            /* Actualizar PWM duty cycle para los 4 motores */
            duty = (uint16_t)((output.vehicle_speed * 1000.0) / 120.0);
            if( duty > 1000 ) duty = 1000;
            if( duty > 0 && duty < 100 ) duty = 100;

            TIM3->CCR1 = duty;
            TIM3->CCR2 = duty;
            TIM3->CCR3 = duty;
            TIM3->CCR4 = duty;
        }
    }
}

/* ================================================================
 *  TAREA 3: DISPLAY (Periodo 200 ms, Prioridad media-baja)
 *  Recibe resultados del modelo por Queue y actualiza LCD.
 * ================================================================ */
void vTaskDisplay( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    ModelOutput_t output;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_DISPLAY_MS ) );

        /* Deadline: actualizar LCD con ultimo dato recibido */
        if( xQueueReceive( xModelToDisplayQueue, &output, 0 ) == pdTRUE )
        {
            /* Linea 1: RPM y Velocidad */
            LCD_Set_Cursor( 1, 5 );
            LCD_Put_Num( (uint16_t)output.engine_rpm );
            LCD_Set_Cursor( 1, 12 );
            LCD_Put_Num( (uint16_t)output.vehicle_speed );

            /* Linea 2: Gear, Aceleracion y Freno */
            LCD_Set_Cursor( 2, 3 );
            LCD_Put_Num( (uint8_t)output.gear );
            LCD_Set_Cursor( 2, 6 );
            if( output.adc_pct < 100 )
                LCD_Put_Char( ' ' );
            if( output.adc_pct < 10 )
                LCD_Put_Char( ' ' );
            LCD_Put_Num( output.adc_pct );
            LCD_Set_Cursor( 2, 12 );
            LCD_Put_Num( output.brake_active );
        }
    }
}

/* ================================================================
 *  TAREA 4: TELEMETRY (Periodo 100 ms, Prioridad media)
 *  Recibe resultados del modelo por Queue y envia por USART.
 * ================================================================ */
void vTaskTelemetry( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    ModelOutput_t output;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_TELEMETRY_MS ) );

        /* Deadline: transmitir datos por USART */
        if( xQueueReceive( xModelToTelemetryQueue, &output, 0 ) == pdTRUE )
        {
            USER_USART_SendTelemetry( &output );
        }
    }
}

/* ================================================================
 *  TAREA 5: HEARTBEAT (Periodo 1000 ms, Prioridad minima)
 *  Parpadeo del LED integrado (PA5).
 * ================================================================ */
void vTaskHeartbeat( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_HEARTBEAT_MS ) );

        /* Deadline: toggle LED */
        GPIOA->ODR ^= ( 0x1UL << 5U );
    }
}
