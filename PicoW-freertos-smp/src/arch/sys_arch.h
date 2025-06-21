#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"

// Defina os tipos como structs, como o sys_arch.c espera
typedef struct {
    SemaphoreHandle_t mut;
} sys_mutex_t;

typedef struct {
    SemaphoreHandle_t sem;
} sys_sem_t;

typedef struct {
    QueueHandle_t mbx;
} sys_mbox_t;

typedef TaskHandle_t sys_thread_t;
typedef int sys_prot_t;

void sys_init(void);
sys_thread_t sys_thread_new(const char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio);

#endif /* __SYS_ARCH_H__ */
