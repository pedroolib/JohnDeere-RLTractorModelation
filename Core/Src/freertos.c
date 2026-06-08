/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"
#include "app_tasks.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief  FreeRTOS initialization
  *         Crea las Queues y las Tareas con prioridades RMS.
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* Crear Queues para comunicacion entre tareas */
  xSensorQueue           = xQueueCreate( 1, sizeof( SensorData_t ) );
  xModelToDisplayQueue   = xQueueCreate( 1, sizeof( ModelOutput_t ) );
  xModelToTelemetryQueue = xQueueCreate( 1, sizeof( ModelOutput_t ) );

  /* Verificar que las Queues se crearon correctamente */
  if( xSensorQueue == NULL || xModelToDisplayQueue == NULL || xModelToTelemetryQueue == NULL )
  {
    /* Error en creacion de queues - bloquear */
    for( ;; );
  }

  /* Crear Tareas Periodicas con calendarizacion RMS
   * Periodo mas corto => Prioridad mas alta
   */

  /* Tarea Sensor: Periodo 40 ms, Prioridad 4 (maxima) */
  xTaskCreate( vTaskSensor,
               "Sensor",
               STACK_SENSOR,
               NULL,
               TASK_SENSOR_PRIO,
               NULL );

  /* Tarea ModelControl: Periodo 40 ms, Prioridad 3 */
  xTaskCreate( vTaskModelControl,
               "ModelCtrl",
               STACK_MODEL,
               NULL,
               TASK_MODEL_PRIO,
               NULL );

  /* Tarea Telemetry: Periodo 100 ms, Prioridad 2 */
  xTaskCreate( vTaskTelemetry,
               "Telemetry",
               STACK_TELEMETRY,
               NULL,
               TASK_TELEMETRY_PRIO,
               NULL );

  /* Tarea Display: Periodo 200 ms, Prioridad 1 */
  xTaskCreate( vTaskDisplay,
               "Display",
               STACK_DISPLAY,
               NULL,
               TASK_DISPLAY_PRIO,
               NULL );

  /* Tarea Heartbeat: Periodo 1000 ms, Prioridad 0 (minima) */
  xTaskCreate( vTaskHeartbeat,
               "Heartbeat",
               STACK_HEARTBEAT,
               NULL,
               TASK_HEARTBEAT_PRIO,
               NULL );
}

/* USER CODE END Application */
