// ADC functions on esp32c3 (APB_SARADC digital controller, 12-bit)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// ADC1 channels: GPIO0=ch0, GPIO1=ch1, GPIO2=ch2, GPIO3=ch3, GPIO4=ch4

#include "board/misc.h" // timer_from_us
#include "command.h" // shutdown
#include "gpio.h" // gpio_adc_setup
#include "internal.h" // APB_SARADC_*
#include "sched.h" // sched_shutdown

DECL_CONSTANT("ADC_MAX", 4095);

/* ADC1 uses GPIO0-GPIO4 (5 channels) */
#define ADC_NUM_CHANNELS 5

/* Pattern table entry: atten[1:0] | chan[5:2] | unit[6] (0=ADC1) */
#define ADC_PAT(chan)   (3u | ((chan) << 2))  /* 11dB attenuation */

/* SAR1_DATA result: bits[21:5] contain the 17-bit word; lower 12 = ADC value */
#define ADC_RESULT(reg) (((reg) >> 5) & 0xFFF)

enum { ADC_CHAN_NONE = 0xFF };
static uint8_t last_channel = ADC_CHAN_NONE;

static void
adc_enable(void)
{
    /* Enable APB_SARADC peripheral clock */
    SYSTEM_PERIP_CLK_EN0_REG |= SYSTEM_APB_SARADC_CLK_EN;
    SYSTEM_PERIP_RST_EN0_REG &= ~SYSTEM_APB_SARADC_CLK_EN;

    /* Use XTAL (40MHz) / 20 = 2MHz ADC clock, SAR_CLK_DIV=1 */
    APB_SARADC_CLKM_CONF_REG = SARADC_CLKM_CLK_EN | SARADC_CLKM_SEL_XTAL
                                | SARADC_CLKM_DIV_NUM(20);

    /* Force ADC power on, software trigger, 1 pattern entry, SAR_CLK_DIV=1 */
    APB_SARADC_CTRL_REG = SARADC_XPD_SAR_FORCE_ON | SARADC_START_FORCE
                          | SARADC_SAR_PATT_LEN(0) | SARADC_SAR_CLK_DIV(1);

    /* No measurement limit */
    APB_SARADC_CTRL2_REG = 0;
}


/****************************************************************
 * ADC interface
 ****************************************************************/

struct gpio_adc
gpio_adc_setup(uint32_t pin)
{
    if (pin >= ADC_NUM_CHANNELS)
        shutdown("Not a valid ADC pin");

    static uint8_t adc_enabled;
    if (!adc_enabled) {
        adc_enable();
        adc_enabled = 1;
    }

    /* Disable digital I/O on the pin: no output, no input buffer */
    IO_MUX_GPIO_REG(pin) = 0;

    return (struct gpio_adc){ .chan = pin };
}

uint32_t
gpio_adc_sample(struct gpio_adc g)
{
    if (last_channel == g.chan) {
        /* Conversion already started for this channel - check done flag */
        if (!(APB_SARADC_INT_RAW_REG & SARADC_ADC1_DONE))
            return timer_from_us(10);
        return 0;
    }

    if (last_channel != ADC_CHAN_NONE)
        /* Another channel is in progress */
        return timer_from_us(10);

    /* Load pattern table entry for this channel, then reset read pointer */
    APB_SARADC_SAR_PATT_TAB1_REG = ADC_PAT(g.chan);
    APB_SARADC_CTRL_REG |= SARADC_SAR_PATT_RESET;
    APB_SARADC_CTRL_REG &= ~SARADC_SAR_PATT_RESET;

    /* Clear done flag and trigger one conversion */
    APB_SARADC_INT_CLR_REG = SARADC_ADC1_DONE;
    APB_SARADC_CTRL_REG |= SARADC_START;

    last_channel = g.chan;
    return timer_from_us(20);
}

uint16_t
gpio_adc_read(struct gpio_adc g)
{
    last_channel = ADC_CHAN_NONE;
    return (uint16_t)ADC_RESULT(APB_SARADC_SAR1_DATA_REG);
}

void
gpio_adc_cancel_sample(struct gpio_adc g)
{
    if (last_channel == g.chan)
        last_channel = ADC_CHAN_NONE;
}
