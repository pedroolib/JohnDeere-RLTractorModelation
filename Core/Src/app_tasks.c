#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "EngTrModel.h"
#include "lcd.h"
#include "usart.h"
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
 *  TAREA 0: REMOTE CONTROL (Prioridad MAXIMA - RMS)
 *  Lee USART1 RX (PA10) constantemente, parsea comandos A:accel;B:brake;S:steering
 *  del ESP8266, envia por Queue a ModelControl. Safe-stop si timeout.
 * ================================================================ */
void vTaskRemoteControl( void *pvParameters )
{
    RemoteCommand_t cmd;
    char rxBuffer[48];
    uint8_t rxIndex = 0;
    int16_t ch;
    TickType_t xLastCommandTime = xTaskGetTickCount();

    (void)pvParameters;

    for( ;; )
    {
        /* Leer todos los caracteres disponibles en USART1 RX */
        while( ( ch = USER_USART1_GetChar() ) >= 0 )
        {
            if( ch == '\n' || ch == '\r' )
            {
                if( rxIndex > 0 )
                {
                    rxBuffer[rxIndex] = '\0';

                    /* Parsear formato: A:accel;B:brake;S:steering
                     * Ej: A:50;B:0;S:128
                     */
                    char *aToken = strstr( rxBuffer, "A:" );
                    char *bToken = strstr( rxBuffer, "B:" );
                    /* S: se ignora por ahora */

                    if( aToken != NULL && bToken != NULL )
                    {
                        uint16_t accel = (uint16_t)atoi( aToken + 2 );
                        uint8_t  brake = (uint8_t)atoi( bToken + 2 );

                        /* Limitar a rangos validos */
                        if( accel > 100 ) accel = 100;
                        cmd.remote_accel = accel;
                        cmd.remote_brake = ( brake != 0 ) ? 1 : 0;
                        cmd.active       = 1;

                        xQueueOverwrite( xRemoteQueue, &cmd );
                        xLastCommandTime = xTaskGetTickCount(); /* Reset timeout */
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
        if( ( xTaskGetTickCount() - xLastCommandTime ) > pdMS_TO_TICKS( REMOTE_TIMEOUT_MS ) )
        {
            cmd.remote_accel = 0;
            cmd.remote_brake = 1; /* Frenar */
            cmd.active       = 0; /* No hay control remoto activo */
            xQueueOverwrite( xRemoteQueue, &cmd );
        }

        /* Esperar 1ms para no saturar CPU, pero seguir leyendo UART frecuentemente */
        vTaskDelay( 1 );
    }
}

/* ================================================================
 *  TAREA 1: MODEL CONTROL (Periodo 40 ms, Prioridad alta)
 *  Recibe comando remoto por Queue, ejecuta modelo Simulink,
 *  envia resultados a Display y Telemetry via Queues.
 * ================================================================ */
void vTaskModelControl( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    static RemoteCommand_t lastRemoteCmd = { 0, 1, 0 }; /* safe defaults */
    RemoteCommand_t freshCmd;
    ModelOutput_t output;
    double throttle;
    uint16_t duty;
    uint8_t localBrakeActive;

    (void)pvParameters;

    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( PERIOD_MODEL_MS ) );

        /* Intentar recibir comando remoto (no bloqueante).
         * Si hay dato nuevo lo guardamos; si no, conservamos el ultimo
         * comando valido para no cortar la aceleracion entre mensajes
         * de la ESP (que llegan cada ~100 ms mientras esta tarea corre
         * cada 40 ms). */
        if( xQueueReceive( xRemoteQueue, &freshCmd, 0 ) == pdTRUE )
        {
            lastRemoteCmd = freshCmd;
        }

        if( lastRemoteCmd.active )
        {
            /* CONTROL REMOTO ACTIVO */
            if( lastRemoteCmd.remote_brake == 1 )
            {
                /* Freno presionado: cut throttle idle */
                throttle = 1.5;
                EngTrModel_U.BrakeTorque = 100.0;
            }
            else
            {
                /* Aceleracion remota: mapear 0-100 a 1.5-100% */
                throttle = 1.5 + ( (double)lastRemoteCmd.remote_accel * 98.5 / 100.0 );
                if( throttle > 100.0 ) throttle = 100.0;
                EngTrModel_U.BrakeTorque = 0.0;
            }
            EngTrModel_U.Throttle = throttle;

            /* Calcular adc_pct para display/telemetry */
            output.adc_pct = lastRemoteCmd.remote_accel;
            localBrakeActive = lastRemoteCmd.remote_brake ? 0 : 1; /* 0=pressed, 1=not pressed */
        }
        else
        {
            /* SAFE STOP: timeout de comando remoto */
            EngTrModel_U.Throttle     = 1.5;
            EngTrModel_U.BrakeTorque  = 100.0;
            output.adc_pct = 0;
            localBrakeActive = 0;
        }

        /* Ejecutar paso del modelo Simulink */
        EngTrModel_step();

        /* Preparar salida para Display y Telemetry */
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
 *  TAREA 2: DISPLAY (Periodo 200 ms, Prioridad media-baja)
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
            LCD_Put_Str("    "); /* borrar residuos */
            LCD_Set_Cursor( 1, 5 );
            LCD_Put_Num( (uint16_t)output.engine_rpm );

            LCD_Set_Cursor( 1, 12 );
            LCD_Put_Str("    "); /* borrar residuos */
            LCD_Set_Cursor( 1, 12 );
            LCD_Put_Num( (uint16_t)output.vehicle_speed );

            /* Linea 2: Gear, Aceleracion y Freno */
            LCD_Set_Cursor( 2, 3 );
            LCD_Put_Str(" "); /* borrar residuo */
            LCD_Set_Cursor( 2, 3 );
            LCD_Put_Num( (uint8_t)output.gear );

            LCD_Set_Cursor( 2, 7 );
            LCD_Put_Str("   "); /* borrar residuos */
            LCD_Set_Cursor( 2, 7 );
            LCD_Put_Num( output.adc_pct );

            LCD_Set_Cursor( 2, 13 );
            LCD_Put_Str(" "); /* borrar residuo */
            LCD_Set_Cursor( 2, 13 );
            LCD_Put_Num( output.brake_active );
        }
    }
}

/* ================================================================
 *  TAREA 3: TELEMETRY (Periodo 100 ms, Prioridad media)
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
