// Timer functions on esp32c3 (SYSTIMER unit0, 16MHz)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/misc.h" // timer_read_time
#include "board/timer_irq.h" // timer_dispatch_many
#include "internal.h" // SYSTIMER_*
#include "sched.h" // DECL_INIT

static uint32_t timer_next;
static uint8_t timer_pending;


/****************************************************************
 * Low level timer code
 ****************************************************************/

static void
systimer_latch(void)
{
    SYSTIMER_UNIT0_OP_REG = SYSTIMER_UNIT0_UPDATE;
    while (!(SYSTIMER_UNIT0_OP_REG & SYSTIMER_UNIT0_VALUE_VALID))
        ;
}

uint32_t
timer_read_time(void)
{
    systimer_latch();
    return SYSTIMER_VALUE0_LO_REG;
}

static void
timer_set(uint32_t next)
{
    timer_next = next;
    timer_pending = 1;
    SYSTIMER_TARGET0_HI_REG = 0;
    SYSTIMER_TARGET0_LO_REG = next;
    SYSTIMER_COMP0_LOAD_REG = 0;
    SYSTIMER_TARGET0_CONF_REG |= SYSTIMER_TARGET0_ALARM_EN;
    SYSTIMER_INT_CLR_REG = SYSTIMER_INT_COMP0;
}

void
timer_kick(void)
{
    timer_set(timer_read_time() + 50);
}

void
timer_poll(void)
{
    if (!timer_pending)
        return;
    if (SYSTIMER_INT_RAW_REG & SYSTIMER_INT_COMP0) {
        SYSTIMER_INT_CLR_REG = SYSTIMER_INT_COMP0;
        SYSTIMER_TARGET0_CONF_REG &= ~SYSTIMER_TARGET0_ALARM_EN;
        timer_pending = 0;
        uint32_t next = timer_dispatch_many();
        timer_set(next);
    }
}

void
timer_init(void)
{
    SYSTIMER_CONF_REG |= SYSTIMER_CLK_EN | SYSTIMER_UNIT0_WORK_EN;
    timer_kick();
}
DECL_INIT(timer_init);
