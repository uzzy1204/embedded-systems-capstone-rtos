#include <stdio.h>
#include <stdint.h>
#include <time.h>

typedef void (*task_fn)(void);
typedef struct { const char* name; task_fn entry; uint32_t period_ms; uint32_t next_deadline_ms; } task_t;

static uint32_t now_ms(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return (uint32_t)(ts.tv_sec*1000 + ts.tv_nsec/1000000); }
static void task_sense(void){ /* stub */ }
static void task_control(void){ /* stub */ }
static void task_telemetry(void){ /* stub */ }

int main(void){
    printf("[rtos-sim] start\n");
    uint32_t t0 = now_ms();
    task_t tasks[] = {
        {.name="sense",     .entry=task_sense,     .period_ms=1,   .next_deadline_ms=0},
        {.name="control",   .entry=task_control,   .period_ms=10,  .next_deadline_ms=0},
        {.name="telemetry", .entry=task_telemetry, .period_ms=100, .next_deadline_ms=0},
    };
    while(now_ms() - t0 < 1000){
        uint32_t t = now_ms() - t0;
        for(size_t i=0;i<sizeof(tasks)/sizeof(tasks[0]);++i){
            if(t >= tasks[i].next_deadline_ms){
                tasks[i].entry();
                tasks[i].next_deadline_ms += tasks[i].period_ms;
            }
        }
    }
    printf("[rtos-sim] done\n");
    return 0;
}
