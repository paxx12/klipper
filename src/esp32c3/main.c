// Startup code on esp32c3
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdint.h> // uint32_t
#include "board/misc.h" // bootloader_request
#include "command.h" // DECL_CONSTANT_STR, DECL_COMMAND_FLAGS
#include "compiler.h" // __visible
#include "generic/irq.h" // irqstatus_t, irq_poll
#include "internal.h" // UART0_*, SYSTIMER_*, RTC_CNTL_*
#include "sched.h" // sched_main
#if CONFIG_ESP32C3_SERIAL_UART0
#include <string.h> // memmove
#endif

DECL_CONSTANT_STR("MCU", "esp32c3");


/****************************************************************
 * Dynamic memory
 ****************************************************************/

static char dynmem_pool[48 * 1024];

void *
dynmem_start(void)
{
    return dynmem_pool;
}

void *
dynmem_end(void)
{
    return &dynmem_pool[sizeof(dynmem_pool)];
}


/****************************************************************
 * IRQ stubs (polling-based, no hardware interrupts used)
 ****************************************************************/

void
irq_disable(void)
{
}

void
irq_enable(void)
{
}

irqstatus_t
irq_save(void)
{
    return 0;
}

void
irq_restore(irqstatus_t flag)
{
    (void)flag;
}

void
irq_wait(void)
{
    irq_poll();
}

extern void timer_poll(void);

void
irq_poll(void)
{
    timer_poll();
    /* UART RX polling is handled in console_task() */
}


/****************************************************************
 * Console I/O (polling UART — only when UART0 polling is selected)
 ****************************************************************/

#if CONFIG_ESP32C3_SERIAL_UART0

#define RX_BUFFER_SIZE 192

static uint8_t receive_buf[RX_BUFFER_SIZE];
static int receive_pos;
static struct task_wake console_wake;

static int
uart_rx_pending(void)
{
    return (int)(UART0_STATUS_REG & UART_STATUS_RXFIFO_CNT_MASK);
}

static uint8_t
uart_rx_byte(void)
{
    return (uint8_t)(UART0_FIFO_REG & 0xFF);
}

static void
uart_tx_byte(uint8_t b)
{
    while (((UART0_STATUS_REG & UART_STATUS_TXFIFO_CNT_MASK)
            >> UART_STATUS_TXFIFO_CNT_S) >= 127)
        ;
    UART0_FIFO_REG = b;
}

void
console_task(void)
{
    while (uart_rx_pending()) {
        uint8_t c = uart_rx_byte();
        if (c == MESSAGE_SYNC)
            sched_wake_tasks();
        if (receive_pos < RX_BUFFER_SIZE)
            receive_buf[receive_pos++] = c;
        sched_wake_task(&console_wake);
    }

    if (!sched_check_wake(&console_wake))
        return;

    int len = receive_pos;
    uint_fast8_t pop_count, msglen = len > MESSAGE_MAX ? MESSAGE_MAX : len;
    int ret = command_find_and_dispatch(receive_buf, msglen, &pop_count);
    if (ret) {
        len -= pop_count;
        if (len) {
            memmove(receive_buf, &receive_buf[pop_count], len);
            sched_wake_task(&console_wake);
        }
    }
    receive_pos = len;
}
DECL_TASK(console_task);

void
console_sendf(const struct command_encoder *ce, va_list args)
{
    uint8_t buf[MESSAGE_MAX];
    uint_fast8_t msglen = command_encode_and_frame(buf, ce, args);
    for (int i = 0; i < msglen; i++)
        uart_tx_byte(buf[i]);
}

#endif /* CONFIG_ESP32C3_SERIAL_UART0 */


/****************************************************************
 * Hardware init
 ****************************************************************/

#if CONFIG_ESP32C3_SERIAL_UART0 || CONFIG_ESP32C3_SERIAL_UART0_IRQ
static void
uart_init(void)
{
    /* UART0 is already configured by the ROM bootloader.
     * Re-configure for CONFIG_SERIAL_BAUD at APB clock (~80MHz). */
    uint32_t div = 80000000 / CONFIG_SERIAL_BAUD;
    UART0_CLKDIV_REG = div & 0xFFFFF;
    UART0_CONF0_REG = UART_CONF0_8N1;
}
#endif

static void
disable_flash_boot_wdt(void)
{
    /* Disable TIMG0 flash-boot watchdog */
    TIMG0_WDTWPROTECT_REG = TIMG_WDT_WKEY;
    TIMG0_WDTCONFIG0_REG = TIMG_WDT_CONF_UPDATE_EN;
    TIMG0_WDTWPROTECT_REG = 0;

    /* Disable RTC watchdog */
    RTC_CNTL_WDTWPROTECT_REG = RTC_WDT_WKEY;
    RTC_CNTL_WDTCONFIG0_REG = 0;
    RTC_CNTL_WDTWPROTECT_REG = 0;

    /* Disable Super watchdog */
    RTC_CNTL_SWD_WPROTECT_REG = RTC_CNTL_SWD_WKEY;
    RTC_CNTL_SWD_CONF_REG |= RTC_CNTL_SWD_DISABLE;
    RTC_CNTL_SWD_WPROTECT_REG = 0;
}


/****************************************************************
 * Reset and bootloader
 ****************************************************************/

void
command_reset(uint32_t *args)
{
    RTC_CNTL_OPTIONS0_REG |= RTC_CNTL_SW_SYS_RST;
    for (;;)
        ;
}
DECL_COMMAND_FLAGS(command_reset, HF_IN_SHUTDOWN, "reset");

void
bootloader_request(void)
{
    command_reset(NULL);
}


/****************************************************************
 * Entry point (called by ROM bootloader)
 ****************************************************************/

void __visible __attribute__((noreturn))
esp32c3_main(void)
{
    /* Clear BSS segment */
    extern uint32_t _bss_start, _bss_end;
    uint32_t *p = &_bss_start;
    while (p < &_bss_end)
        *p++ = 0;

    disable_flash_boot_wdt();
#if CONFIG_ESP32C3_SERIAL_UART0 || CONFIG_ESP32C3_SERIAL_UART0_IRQ
    uart_init();
#endif

    sched_main();
    for (;;)
        ;
}
