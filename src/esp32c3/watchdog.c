// Watchdog code on esp32c3 (TIMG0 WDT)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "internal.h" // TIMG0_*
#include "sched.h" // DECL_TASK

/* Prescaler divides APB (80MHz) to 1MHz WDT clock; timeout ~350ms */
#define WDT_PRESCALER   79
#define WDT_TIMEOUT     350000

void
watchdog_reset(void)
{
    TIMG0_WDTWPROTECT_REG = TIMG_WDT_WKEY;
    TIMG0_WDTFEED_REG = 1;
    TIMG0_WDTWPROTECT_REG = 0;
}
DECL_TASK(watchdog_reset);

void
watchdog_init(void)
{
    TIMG0_WDTWPROTECT_REG = TIMG_WDT_WKEY;
    TIMG0_WDTCONFIG0_REG = 0;
    TIMG0_WDTCONFIG1_REG = WDT_PRESCALER;
    TIMG0_WDTCONFIG2_REG = WDT_TIMEOUT;
    TIMG0_WDTCONFIG0_REG = TIMG_WDT_EN | TIMG_WDT_STG0_RESET_SYS;
    TIMG0_WDTWPROTECT_REG = 0;
}
DECL_INIT(watchdog_init);
