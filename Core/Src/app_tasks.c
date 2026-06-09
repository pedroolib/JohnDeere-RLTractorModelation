#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
QueueHandle_t xRemoteQueue = NULL;

/* ================================================================
 *  TAREA 0: REMOTE CONTROL (Periodo 50 ms, Prioridad MAXIMA - RMS)
 *  Lee USART1 RX (PA10), parsea comandos A:xxx;B:y del ESP8266,
 *  envia por Queue a ModelControl. Safe-stop si no hay comando.
 * ================================================================ */
void vTaskRemoteControl( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    RemoteCommand_t cmd;
    char rxBuffer[32];
    uint8_t rxIndex = 0;
    int16_t ch;
    uint8_t timeoutCounter = 0;
    const uint8_t timeoutThreshold = (uint8_t)( REMOTE_TIMEOUT_MS / PERIOD_REMOTE_MS ); /* 10 ciclos */

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_REMOTE_MS ) );

        /* Leer todos los caracteres disponibles en USART1 RX */
        while( ( ch = USER_USART1_GetChar() ) >= 0 )
        {
            if( ch == '\n' || ch == '\r' )
            {
                if( rxIndex > 0 )
                {
                    rxBuffer[rxIndex] = '\0';

                    /* Parsear formato: A:xxx;B:y  (ej: A:128;B:1) */
                    char *aToken = strstr( rxBuffer, "A:" );
                    char *bToken = strstr( rxBuffer, "B:" );

                    if( aToken != NULL && bToken != NULL )
                    {
                        uint16_t accel = (uint16_t)atoi( aToken + 2 );
                        uint8_t  brake = (uint8_t)atoi( bToken + 2 );

                        cmd.remote_accel = accel;
                        cmd.remote_brake = ( brake != 0 ) ? 1 : 0;
                        cmd.active       = 1;

                        xQueueOverwrite( xRemoteQueue, &cmd );
                        timeoutCounter = 0; /* Reset timeout */
                    }
                    rxIndex = 0;
                }
            }
            else if( rxIndex < ( sizeof(rxBuffer) - 1U ) )
            {
                rxBuffer[rxIndex++] = (char)ch;
            }
        }

        /* Timeout: si no recibimos comando nuevo en REMOTE_TIMEOUT_MS, enviar safe-stop */
        timeoutCounter++;
        if( timeoutCounter >= timeoutThreshold )
        {
            cmd.remote_accel = 0;
            cmd.remote_brake = 1; /* Frenar */
            cmd.active       = 0; /* No hay control remoto activo */
            xQueueOverwrite( xRemoteQueue, &cmd );
            timeoutCounter = timeoutThreshold; /* No seguir contando */
        }
    }
}

/* ================================================================
 *  TAREA 1: SENSOR (Periodo 40 ms, Prioridad alta - RMS)
 *  Lee ADC y freno local, envia datos por Queue a ModelControl.
 *  Nota: ModelControl ignora estos datos cuando hay control remoto.
 * ================================================================ */
void vTaskSensor( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SensorData_t data;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_SENSOR_MS ) );

        /* Lectura de sensores locales (solo para telemetria/debug) */
        data.adc   = USER_ADC_Read();
        data.brake = USER_Brake_Read();

        xQueueOverwrite( xSensorQueue, &data );
    }
}

/* ================================================================
 *  TAREA 2: MODEL CONTROL (Periodo 40 ms, Prioridad alta)
 *  Recibe comando remoto por Queue (overridea sensores locales).
 *  Ejecuta modelo Simulink, envia resultados por Queue.
 * ================================================================ */
void vTaskModelControl( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    RemoteCommand_t remoteCmd;
    SensorData_t sensorData;
    ModelOutput_t output;
    double throttle;
    uint16_t duty;
    uint8_t localBrakeActive;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_MODEL_MS ) );

        /* Intentar recibir comando remoto (no bloqueante) */
        if( xQueueReceive( xRemoteQueue, &remoteCmd, 0 ) == pdTRUE && remoteCmd.active )
        {
            /* CONTROL REMOTO ACTIVO: overridea potenciometro y freno local */
            if( remoteCmd.remote_brake == 1 )
            {
                /* Freno presionado remoto: cut throttle idle */
                throttle = 1.5;
                EngTrModel_U.BrakeTorque = 100.0;
            }
            else
            {
                /* Aceleracion remota: mapear 0-255 a 1.5-100% */
                throttle = 1.5 + ( (double)remoteCmd.remote_accel * 28.5 / 255.0 );
                if( throttle > 100.0 ) throttle = 100.0;
                EngTrModel_U.BrakeTorque = 0.0;
            }
            EngTrModel_U.Throttle = throttle;

            /* Calcular adc_pct para telemetria */
            output.adc_pct = (uint16_t)(( remoteCmd.remote_accel * 100U ) / 255U);
            localBrakeActive = remoteCmd.remote_brake ? 0 : 1; /* 0=pressed, 1=not pressed */
        }
        else
        {
            /* SAFE STOP: no hay control remoto activo */
            /* Fallback: leer sensores locales solo para no quedar sin datos */
            if( xQueueReceive( xSensorQueue, &sensorData, 0 ) == pdTRUE )
            {
                if( sensorData.brake == 0 )
                {
                    throttle = 1.5;
                    EngTrModel_U.BrakeTorque = 100.0;
                }
                else
                {
                    throttle = (double)((sensorData.adc * 100.0) / 4095.0);
                    if( throttle < 1.5 ) throttle = 1.5;
                    EngTrModel_U.BrakeTorque = 0.0;
                }
                EngTrModel_U.Throttle = throttle;

                if( sensorData.brake == 0 )
                {
                    output.adc_pct = 0;
                }
                else
                {
                    output.adc_pct = (uint16_t)((sensorData.adc * 100U) / 4095U);
                }
                localBrakeActive = sensorData.brake ? 0 : 1;
            }
            else
            {
                /* Ni remoto ni local: forzar stop */
                EngTrModel_U.Throttle     = 1.5;
                EngTrModel_U.BrakeTorque  = 100.0;
                output.adc_pct = 0;
                localBrakeActive = 0;
            }
        }

        /* Ejecutar paso del modelo Simulink */
        EngTrModel_step();

        /* Preparar salida para otras tareas */
        output.engine_rpm    = EngTrModel_Y.EngineSpeed;
        output.vehicle_speed = EngTrModel_Y.VehicleSpeed;
        output.gear          = EngTrModel_Y.Gear;
        output.brake_active  = localBrakeActive;

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

        GPIOA->ODR ^= ( 0x1UL << 5U );
    }
}
