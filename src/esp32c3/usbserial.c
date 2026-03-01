// USB Serial/JTAG console on esp32c3
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdint.h>
#include "autoconf.h" // CONFIG_USB_SERIAL_NUMBER_CHIPID
#include "board/serial_irq.h" // serial_rx_byte, serial_get_tx_byte
#include "internal.h" // USB_DEVICE_*
#include "sched.h" // DECL_TASK, DECL_INIT
#if CONFIG_USB_SERIAL_NUMBER_CHIPID
#include "generic/usbstd.h" // usb_string_descriptor
void usb_fill_serial(struct usb_string_descriptor *d, int l, void *id)
{
    (void)d; (void)l; (void)id;
}
#endif

static struct task_wake usbserial_tx_wake;

void
usbserial_task(void)
{
    while (USB_DEVICE_EP1_CONF_REG & USB_DEVICE_SERIAL_OUT_AVAIL)
        serial_rx_byte((uint8_t)(USB_DEVICE_EP1_REG & 0xFF));

    if (!sched_check_wake(&usbserial_tx_wake))
        return;

    if (!(USB_DEVICE_EP1_CONF_REG & USB_DEVICE_SERIAL_IN_FREE)) {
        sched_wake_task(&usbserial_tx_wake);
        return;
    }

    int sent = 0;
    uint8_t b;
    while (USB_DEVICE_EP1_CONF_REG & USB_DEVICE_SERIAL_IN_FREE) {
        if (serial_get_tx_byte(&b))
            break;
        USB_DEVICE_EP1_REG = b;
        sent = 1;
    }
    if (sent)
        USB_DEVICE_EP1_CONF_REG = USB_DEVICE_WR_DONE;
}
DECL_TASK(usbserial_task);

void
serial_enable_tx_irq(void)
{
    sched_wake_task(&usbserial_tx_wake);
}

void
usbserial_init(void)
{
    USB_DEVICE_MISC_CONF_REG |= USB_DEVICE_CLK_EN;
}
DECL_INIT(usbserial_init);
