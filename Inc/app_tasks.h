#ifndef APP_TASKS_H_
#define APP_TASKS_H_

#include <stdint.h>

/* FreeRTOS-like EventGroup simulation for ISR-to-task signaling.
 * In the real FreeRTOS port, these will map to FreeRTOS primitives.
 */

#define EVENT_MODEL_READY   ( 1U << 0U )

extern volatile uint32_t g_event_flags;

/* ISR side: set event bits atomically (ISR-safe on Cortex-M) */
static inline void xEventGroupSetBits( uint32_t bitsToSet )
{
    g_event_flags |= bitsToSet;
}

/* Task side: wait for event bits, optionally clear on exit.
 * Returns non-zero if the requested bits are present.
 */
static inline uint32_t xEventGroupWaitBits(
    uint32_t bitsToWaitFor,
    uint8_t  clearOnExit
)
{
    uint32_t ret = 0U;
    if( g_event_flags & bitsToWaitFor ){
        ret = g_event_flags & bitsToWaitFor;
        if( clearOnExit ){
            g_event_flags &= ~bitsToWaitFor;
        }
    }
    return ret;
}

/* Task prototypes */
void vTaskModelControl( void );
void vTaskDisplay( void );
void vTaskTelemetry( void );
void vTaskHeartbeat( void );

#endif /* APP_TASKS_H_ */
