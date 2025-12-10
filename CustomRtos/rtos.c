#include "rtos.h"
#include "stm32f4xx_hal.h"   // correct for F4 family

static volatile uint32_t g_tick = 0;
static TaskControlBlock g_tasks[MAX_TASKS];

void RTOS_Init(uint32_t tick_hz)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        g_tasks[i].active = 0;
    }
    g_tick = 0;

    uint32_t reload = HAL_RCC_GetHCLKFreq() / tick_hz - 1;
    SysTick->LOAD  = reload;
    SysTick->VAL   = 0;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;
}

int RTOS_AddTask(TaskFunc_t func, uint32_t period_ticks, uint32_t start_delay_ticks)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!g_tasks[i].active) {
            g_tasks[i].func    = func;
            g_tasks[i].period  = period_ticks;
            g_tasks[i].nextRun = g_tick + start_delay_ticks;
            g_tasks[i].active  = 1;
            return i;
        }
    }
    return -1;
}

void RTOS_TickHandler(void)
{
    g_tick++;
}

void RTOS_Start(void)
{
    while (1) {
        uint32_t now = g_tick;
        for (int i = 0; i < MAX_TASKS; i++) {
            if (g_tasks[i].active && (int32_t)(now - g_tasks[i].nextRun) >= 0) {
                g_tasks[i].func();
                g_tasks[i].nextRun += g_tasks[i].period;
            }
        }
        __NOP();  // idle hook
    }
}
