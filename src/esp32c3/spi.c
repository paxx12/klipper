// SPI functions on esp32c3 (SPI2/GPSPI2/FSPI)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// Default bus pins use IO_MUX func2 (no GPIO matrix overhead):
//   SCK=GPIO6, MOSI=GPIO7, MISO=GPIO2

#include "command.h" // shutdown
#include "gpio.h" // spi_setup
#include "internal.h" // SPI2_*
#include "sched.h" // sched_shutdown

/* spi2: MISO=gpio2, MOSI=gpio7, SCK=gpio6 via IO_MUX func2 */
DECL_ENUMERATION("spi_bus", "spi2", 0);
DECL_CONSTANT_STR("BUS_PINS_spi2", "gpio2,gpio7,gpio6");

/* APB clock = 80MHz */
#define SPI_APB_HZ  80000000U

/* Maximum bytes per transfer (SPI2 has 16 data words = 64 bytes) */
#define SPI_MAX_BYTES 64


/****************************************************************
 * Setup
 ****************************************************************/

static uint32_t
compute_clock_reg(uint32_t rate)
{
    if (rate >= SPI_APB_HZ)
        /* Use APB clock directly */
        return (1u << 31);

    uint32_t div = (SPI_APB_HZ + rate - 1) / rate;
    if (div < 2)
        div = 2;
    uint32_t n = div - 1;
    uint32_t h = n / 2;
    uint32_t l = n / 2;
    return l | (h << 6) | (n << 12);
}

static void
spi2_init_pins(void)
{
    /* Enable SPI2 clock */
    SYSTEM_PERIP_CLK_EN0_REG |= SYSTEM_SPI2_CLK_EN;
    SYSTEM_PERIP_RST_EN0_REG &= ~SYSTEM_SPI2_CLK_EN;

    /* Configure IO_MUX to func2 (direct SPI2) for SCK, MOSI, MISO */
    IO_MUX_GPIO_REG(6) = IO_MUX_MCU_SEL_FUNC2 | IO_MUX_FUN_DRV_2;
    IO_MUX_GPIO_REG(7) = IO_MUX_MCU_SEL_FUNC2 | IO_MUX_FUN_DRV_2;
    IO_MUX_GPIO_REG(2) = IO_MUX_MCU_SEL_FUNC2 | IO_MUX_FUN_IE | IO_MUX_FUN_DRV_2;
}

struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
    if (bus != 0)
        shutdown("Invalid spi bus");

    static uint8_t initialized;
    if (!initialized) {
        spi2_init_pins();
        initialized = 1;
    }

    return (struct spi_config){
        .clock_reg = compute_clock_reg(rate),
        .mode = mode,
    };
}


/****************************************************************
 * Transfer
 ****************************************************************/

void
spi_prepare(struct spi_config config)
{
    SPI2_CLOCK_REG = config.clock_reg;

    /* Set CPOL via MISC_REG and CPHA via USER_REG */
    if (config.mode & 2)
        SPI2_MISC_REG |= SPI_CK_IDLE_EDGE;
    else
        SPI2_MISC_REG &= ~SPI_CK_IDLE_EDGE;

    if (config.mode & 1)
        SPI2_USER_REG |= SPI_CK_OUT_EDGE;
    else
        SPI2_USER_REG &= ~SPI_CK_OUT_EDGE;
}

void
spi_transfer(struct spi_config config, uint8_t receive_data
             , uint8_t len, uint8_t *data)
{
    while (len) {
        uint8_t chunk = len > SPI_MAX_BYTES ? SPI_MAX_BYTES : len;

        /* Write TX data into W registers (little-endian byte order) */
        volatile uint32_t *w = &SPI2_W0_REG;
        uint8_t *src = data;
        for (int i = 0; i < (chunk + 3) / 4; i++) {
            uint32_t word = 0;
            for (int b = 0; b < 4 && (i * 4 + b) < chunk; b++)
                word |= (uint32_t)src[i * 4 + b] << (b * 8);
            w[i] = word;
        }

        /* Set transfer length in bits */
        SPI2_MS_DLEN_REG = chunk * 8 - 1;

        /* Enable MOSI and full-duplex MISO if needed */
        uint32_t user = SPI2_USER_REG & ~(SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN);
        user |= SPI_USR_MOSI;
        if (receive_data)
            user |= SPI_USR_MISO | SPI_DOUTDIN;
        SPI2_USER_REG = user;

        /* Start transfer */
        SPI2_CMD_REG = SPI_USR;
        while (SPI2_CMD_REG & SPI_USR)
            ;

        /* Read RX data from W registers */
        if (receive_data) {
            volatile uint32_t *rw = &SPI2_W0_REG;
            for (int i = 0; i < (chunk + 3) / 4; i++) {
                uint32_t word = rw[i];
                for (int b = 0; b < 4 && (i * 4 + b) < chunk; b++)
                    data[i * 4 + b] = (word >> (b * 8)) & 0xFF;
            }
        }

        data += chunk;
        len -= chunk;
    }
}
