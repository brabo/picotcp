#ifndef _INCLUDE_PICO_FROSTED
#define _INCLUDE_PICO_FROSTED

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico_constants.h"

/* monotonically increasing tick,
 * typically incremented every millisecond in a systick interrupt */
extern volatile unsigned int jiffies;

#define dbg printf

#define PICO_SUPPORT_MUTEX

#define pico_zalloc(x) kcalloc(x)
#define pico_free(x) free(x)

extern void *pico_mutex_init(void);
extern void pico_mutex_lock(void*);
extern void pico_mutex_unlock(void*);

static inline pico_time PICO_TIME_MS()
{
    return jiffies;
}

static inline pico_time PICO_TIME()
{
    return jiffies / 1000;
}

static inline void PICO_IDLE(void)
{
    pico_time now = PICO_TIME_MS();
    while(now == PICO_TIME_MS()) ;
}
