#ifndef APP_TASKS_H_
#define APP_TASKS_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

/* ================================================================
 *  Estructuras de datos para comunicacion entre Tareas (Queues)
 * ================================================================ */

typedef struct {
    uint16_t remote_accel;   /* 0-100 (valor del mando PS4) */
    uint8_t  remote_brake;   /* 0 o 1 */
    uint8_t  active;         /* 1 = comando recibido recientemente */
} RemoteCommand_t;

typedef struct {
    double   engine_rpm;
    double   vehicle_speed;
    double   gear;
    uint16_t adc_pct;
    uint8_t  brake_active;
} ModelOutput_t;

/* ================================================================
 *  Queue Handles - mecanismos de comunicacion entre tareas
 * ================================================================ */
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xModelToDisplayQueue;
extern QueueHandle_t xModelToTelemetryQueue;
extern QueueHandle_t xRemoteQueue;

/* ================================================================
 *  Prioridades RMS (Rate Monotonic Scheduling)
 *  Periodo mas corto => Prioridad mas alta
 *
 *  Tarea             Periodo (ms)   Prioridad
 *  ---------------------------------------------------
 *  RemoteControl     continuo       4  (maxima)
 *  ModelControl      40             3
 *  Telemetry         100            2
 *  Display           200            1
 * ================================================================ */
#define TASK_REMOTE_PRIO        (tskIDLE_PRIORITY + 4U)
#define TASK_MODEL_PRIO         (tskIDLE_PRIORITY + 3U)
#define TASK_TELEMETRY_PRIO     (tskIDLE_PRIORITY + 2U)
#define TASK_DISPLAY_PRIO       (tskIDLE_PRIORITY + 1U)

/* Tamanos de stack */
#define STACK_REMOTE            256U
#define STACK_MODEL             256U
#define STACK_TELEMETRY         256U
#define STACK_DISPLAY           256U

/* Periodos en ms */
#define PERIOD_MODEL_MS         40U
#define PERIOD_TELEMETRY_MS     100U
#define PERIOD_DISPLAY_MS       200U

/* Timeout de comando remoto (ms) antes de safe-stop */
#define REMOTE_TIMEOUT_MS       500U

/* Prototipos de tareas */
void vTaskRemoteControl( void *pvParameters );
void vTaskModelControl( void *pvParameters );
void vTaskDisplay( void *pvParameters );
void vTaskTelemetry( void *pvParameters );

#endif /* APP_TASKS_H_ */
