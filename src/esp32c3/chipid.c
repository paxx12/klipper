// Chip ID on esp32c3 - read unique MAC address from eFuse block 1
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_USB_SERIAL_NUMBER_CHIPID
#include "generic/usb_cdc.h" // usb_fill_serial
#include "generic/usbstd.h" // usb_string_descriptor
#include "internal.h" // EFUSE_*
#include "sched.h" // DECL_INIT

#define CHIPID_LEN 6  /* 6-byte MAC = 48-bit unique ID */

static struct {
    struct usb_string_descriptor desc;
    uint16_t data[CHIPID_LEN * 2];
} cdc_chipid;

struct usb_string_descriptor *
usbserial_get_serialid(void)
{
    return &cdc_chipid.desc;
}

static void
read_chipid(uint8_t *out)
{
    uint32_t lo = EFUSE_RD_MAC_SPI_SYS_0_REG;
    uint32_t hi = EFUSE_RD_MAC_SPI_SYS_1_REG;
    out[0] = lo & 0xFF;
    out[1] = (lo >> 8) & 0xFF;
    out[2] = (lo >> 16) & 0xFF;
    out[3] = (lo >> 24) & 0xFF;
    out[4] = hi & 0xFF;
    out[5] = (hi >> 8) & 0xFF;
}

void
chipid_init(void)
{
    if (!CONFIG_USB_SERIAL_NUMBER_CHIPID)
        return;
    uint8_t data[CHIPID_LEN];
    read_chipid(data);
    usb_fill_serial(&cdc_chipid.desc, ARRAY_SIZE(cdc_chipid.data), data);
}
DECL_INIT(chipid_init);
