// USB Serial/JTAG console on esp32c3 (polling)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memmove
#include <stdint.h> // uint8_t
#include "command.h" // command_find_and_dispatch
#include "internal.h" // USB_DEVICE_*
#include "sched.h" // DECL_TASK, DECL_INIT

#define RX_BUFFER_SIZE 192

static uint8_t receive_buf[RX_BUFFER_SIZE];
static int receive_pos;
static struct task_wake console_wake;

static int
usb_rx_pending(void)
{
    return !!(USB_DEVICE_EP1_CONF_REG & USB_DEVICE_SERIAL_OUT_AVAIL);
}

static uint8_t
usb_rx_byte(void)
{
    return (uint8_t)(USB_DEVICE_EP1_REG & 0xFF);
}

static void
usb_tx_byte(uint8_t b)
{
    while (!(USB_DEVICE_EP1_CONF_REG & USB_DEVICE_SERIAL_IN_FREE))
        ;
    USB_DEVICE_EP1_REG = b;
}

void
console_task(void)
{
    while (usb_rx_pending()) {
        uint8_t c = usb_rx_byte();
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
        usb_tx_byte(buf[i]);
    USB_DEVICE_EP1_CONF_REG = USB_DEVICE_WR_DONE;
}

void
usbserial_init(void)
{
    USB_DEVICE_MISC_CONF_REG |= USB_DEVICE_CLK_EN;
}
DECL_INIT(usbserial_init);
