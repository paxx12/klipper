// GPIO functions on esp32c3
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "command.h" // shutdown
#include "gpio.h" // gpio_out_setup
#include "internal.h" // GPIO_*
#include "sched.h" // sched_shutdown


/****************************************************************
 * Pin mappings
 ****************************************************************/

DECL_ENUMERATION_RANGE("pin", "gpio0", 0, NUM_GPIO);

static void
gpio_set_iomux(uint8_t pin, int pull_up, int input_en)
{
    uint32_t val = IO_MUX_MCU_SEL_GPIO | IO_MUX_FUN_DRV_2;
    if (pull_up > 0)
        val |= IO_MUX_FUN_WPU;
    if (pull_up < 0)
        val |= IO_MUX_FUN_WPD;
    if (input_en)
        val |= IO_MUX_FUN_IE;
    IO_MUX_GPIO_REG(pin) = val;
}


/****************************************************************
 * General Purpose Input Output (GPIO) pins
 ****************************************************************/

struct gpio_out
gpio_out_setup(uint8_t pin, uint8_t val)
{
    if (pin >= NUM_GPIO)
        shutdown("Not a valid pin");
    struct gpio_out g = { .bit = 1u << pin };
    gpio_out_reset(g, val);
    return g;
}

void
gpio_out_reset(struct gpio_out g, uint8_t val)
{
    int pin = __builtin_ctz(g.bit);
    gpio_out_write(g, val);
    GPIO_FUNC_OUT_SEL_REG(pin) = GPIO_OUT_SEL_SIMPLE;
    GPIO_ENABLE_W1TS_REG = g.bit;
    gpio_set_iomux(pin, 0, 0);
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
    if (GPIO_OUT_REG & g.bit)
        GPIO_OUT_W1TC_REG = g.bit;
    else
        GPIO_OUT_W1TS_REG = g.bit;
}

void
gpio_out_toggle(struct gpio_out g)
{
    gpio_out_toggle_noirq(g);
}

void
gpio_out_write(struct gpio_out g, uint8_t val)
{
    if (val)
        GPIO_OUT_W1TS_REG = g.bit;
    else
        GPIO_OUT_W1TC_REG = g.bit;
}


struct gpio_in
gpio_in_setup(uint8_t pin, int8_t pull_up)
{
    if (pin >= NUM_GPIO)
        shutdown("Not a valid pin");
    struct gpio_in g = { .bit = 1u << pin };
    gpio_in_reset(g, pull_up);
    return g;
}

void
gpio_in_reset(struct gpio_in g, int8_t pull_up)
{
    int pin = __builtin_ctz(g.bit);
    GPIO_ENABLE_W1TC_REG = g.bit;
    gpio_set_iomux(pin, pull_up, 1);
}

uint8_t
gpio_in_read(struct gpio_in g)
{
    return !!(GPIO_IN_REG & g.bit);
}


/****************************************************************
 * Stubs for unimplemented features
 ****************************************************************/

struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
    return (struct spi_config){ };
}

void
spi_prepare(struct spi_config config)
{
}

void
spi_transfer(struct spi_config config, uint8_t receive_data
             , uint8_t len, uint8_t *data)
{
}
