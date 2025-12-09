#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define MAX_TASKS  8

typedef void (*TaskFunc_t)(void);

typedef struct {
    TaskFunc_t func;
    uint32_t   period;   // period in ticks
    uint32_t   nextRun;  // next tick to run
    uint8_t    active;   // 1 = active, 0 = free
} TaskControlBlock;

void RTOS_Init(uint32_t tick_hz);
int  RTOS_AddTask(TaskFunc_t func, uint32_t period_ticks, uint32_t start_delay_ticks);
void RTOS_Start(void);
void RTOS_TickHandler(void);

#endif
