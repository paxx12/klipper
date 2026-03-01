// I2C functions on esp32c3 (command-queue master controller)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// I2C0 buses use GPIO matrix; common default configs exposed below.
// SDA/SCL signal indices for I2C0: SDA_OUT=65, SCL_OUT=66, SDA_IN=65, SCL_IN=66
// (verify exact values from ESP32-C3 gpio_sig_map.h)

#include "board/misc.h" // timer_read_time
#include "command.h" // shutdown
#include "gpio.h" // i2c_setup
#include "i2ccmds.h" // I2C_BUS_SUCCESS
#include "internal.h" // I2C0_BASE
#include "sched.h" // sched_shutdown

/* I2C0 GPIO matrix signal indices for ESP32-C3 */
#define I2C0_SCL_OUT_IDX    29
#define I2C0_SCL_IN_IDX     29
#define I2C0_SDA_OUT_IDX    28
#define I2C0_SDA_IN_IDX     28

/* I2C1 signal indices */
#define I2C1_SCL_OUT_IDX    95
#define I2C1_SCL_IN_IDX     95
#define I2C1_SDA_OUT_IDX    94
#define I2C1_SDA_IN_IDX     94

/* GPIO_FUNC_IN_SEL: bit 7 = bypass GPIO matrix (use constant) */
#define GPIO_FUNC_IN_BYPASS (1u << 7)

/* Timeout: ~10ms at 16MHz SYSTIMER */
#define I2C_TIMEOUT_US  10000

DECL_ENUMERATION("i2c_bus", "i2c0_gpio5_gpio6",   0);
DECL_CONSTANT_STR("BUS_PINS_i2c0_gpio5_gpio6",    "gpio5,gpio6");
DECL_ENUMERATION("i2c_bus", "i2c0_gpio8_gpio9",   1);
DECL_CONSTANT_STR("BUS_PINS_i2c0_gpio8_gpio9",    "gpio8,gpio9");
DECL_ENUMERATION("i2c_bus", "i2c1_gpio18_gpio19", 2);
DECL_CONSTANT_STR("BUS_PINS_i2c1_gpio18_gpio19",  "gpio18,gpio19");

struct i2c_info {
    uint32_t base;
    uint8_t sda_pin, scl_pin;
    uint8_t sda_out, scl_out, sda_in, scl_in;
    uint32_t clk_en;
};

static const struct i2c_info i2c_buses[] = {
    { I2C0_BASE,  5,  6, I2C0_SDA_OUT_IDX, I2C0_SCL_OUT_IDX,
      I2C0_SDA_IN_IDX, I2C0_SCL_IN_IDX, SYSTEM_I2C0_CLK_EN },
    { I2C0_BASE,  8,  9, I2C0_SDA_OUT_IDX, I2C0_SCL_OUT_IDX,
      I2C0_SDA_IN_IDX, I2C0_SCL_IN_IDX, SYSTEM_I2C0_CLK_EN },
    { I2C1_BASE, 18, 19, I2C1_SDA_OUT_IDX, I2C1_SCL_OUT_IDX,
      I2C1_SDA_IN_IDX, I2C1_SCL_IN_IDX, SYSTEM_I2C1_CLK_EN },
};

/* Packed into i2c_config.cfg: base address (top 28 bits) | addr (7 bits) */
#define CFG_BASE(b)  ((b) & ~0xFFFu)
#define CFG_ADDR(a)  ((a) & 0x7F)
#define CFG_GET_BASE(c)  ((c) & ~0xFFFu)
#define CFG_GET_ADDR(c)  ((c) & 0x7F)


/****************************************************************
 * Setup
 ****************************************************************/

static void
i2c_configure_pin(uint8_t pin, uint8_t out_sig, uint8_t in_sig)
{
    /* Open-drain output through GPIO matrix */
    GPIO_PIN_REG(pin) |= (1u << 2);           /* PAD_DRIVER = open-drain */
    GPIO_FUNC_OUT_SEL_REG(pin) = out_sig;
    GPIO_FUNC_IN_SEL_REG(in_sig) = pin;
    IO_MUX_GPIO_REG(pin) = IO_MUX_MCU_SEL_GPIO | IO_MUX_FUN_IE
                           | IO_MUX_FUN_WPU | IO_MUX_FUN_DRV_2;
    GPIO_ENABLE_W1TS_REG = (1u << pin);
}

static void
i2c_set_timing(uint32_t base, uint32_t rate)
{
    /* APB = 80MHz; compute half-period counts */
    uint32_t half = (80000000u / (2 * rate)) - 1;
    I2C_SCL_LOW_PERIOD_REG(base) = half;
    I2C_SCL_HIGH_PERIOD_REG(base) = half;
    I2C_SDA_HOLD_REG(base) = half / 2;
    I2C_SDA_SAMPLE_REG(base) = half / 2;
    I2C_SCL_START_HOLD_REG(base) = half;
    I2C_SCL_RSTART_SETUP_REG(base) = half;
    I2C_SCL_STOP_HOLD_REG(base) = half;
    I2C_SCL_STOP_SETUP_REG(base) = half;
    I2C_TO_REG(base) = 0xFFFF;
    /* Use APB as I2C clock source */
    I2C_CLK_CONF_REG(base) = (1u << 20) | (0u << 21);  /* CLK_EN, SEL=APB */
}

struct i2c_config
i2c_setup(uint32_t bus, uint32_t rate, uint8_t addr)
{
    if (bus >= ARRAY_SIZE(i2c_buses))
        shutdown("Invalid i2c bus");

    const struct i2c_info *info = &i2c_buses[bus];

    /* Enable peripheral clock */
    SYSTEM_PERIP_CLK_EN0_REG |= info->clk_en;
    SYSTEM_PERIP_RST_EN0_REG &= ~info->clk_en;

    /* Set master mode and enable clock */
    I2C_CTR_REG(info->base) = I2C_MS_MODE | I2C_CLK_EN;

    i2c_set_timing(info->base, rate);
    i2c_configure_pin(info->sda_pin, info->sda_out, info->sda_in);
    i2c_configure_pin(info->scl_pin, info->scl_out, info->scl_in);

    return (struct i2c_config){
        .cfg = CFG_BASE(info->base) | CFG_ADDR(addr),
    };
}


/****************************************************************
 * Transfer helpers
 ****************************************************************/

static int
i2c_wait_done(uint32_t base)
{
    uint32_t deadline = timer_read_time() + timer_from_us(I2C_TIMEOUT_US);
    for (;;) {
        uint32_t raw = I2C_INT_RAW_REG(base);
        if (raw & I2C_INT_ERROR_MASK)
            return I2C_BUS_NACK;
        if (raw & I2C_INT_TRANS_COMPLETE)
            return I2C_BUS_SUCCESS;
        if (timer_is_before(deadline, timer_read_time()))
            return I2C_BUS_TIMEOUT;
    }
}

static void
i2c_start_transfer(uint32_t base)
{
    I2C_INT_CLR_REG(base) = 0xFFFFFFFF;
    I2C_CTR_REG(base) |= I2C_TRANS_START;
}


/****************************************************************
 * Write and read
 ****************************************************************/

int
i2c_write(struct i2c_config config, uint8_t write_len, uint8_t *write)
{
    uint32_t base = CFG_GET_BASE(config.cfg);
    uint8_t addr  = CFG_GET_ADDR(config.cfg);

    /* Build command queue:
     * [0] RSTART
     * [1] WRITE addr+W, ACK_CHECK
     * [2] WRITE data bytes, ACK_CHECK
     * [3] STOP */
    I2C_COMD_REG(base, 0) = I2C_CMD(I2C_CMD_RSTART, 0, 0, 0, 0);
    I2C_COMD_REG(base, 1) = I2C_CMD(I2C_CMD_WRITE, 1, 1, 0, 0);
    I2C_COMD_REG(base, 2) = I2C_CMD(I2C_CMD_WRITE, write_len, 1, 0, 0);
    I2C_COMD_REG(base, 3) = I2C_CMD(I2C_CMD_STOP, 0, 0, 0, 0);

    /* Push address byte then data into FIFO */
    I2C_DATA_REG(base) = (addr << 1) & 0xFE;
    for (int i = 0; i < write_len; i++)
        I2C_DATA_REG(base) = write[i];

    i2c_start_transfer(base);
    return i2c_wait_done(base);
}

int
i2c_read(struct i2c_config config, uint8_t reg_len, uint8_t *reg
         , uint8_t read_len, uint8_t *read)
{
    uint32_t base = CFG_GET_BASE(config.cfg);
    uint8_t addr  = CFG_GET_ADDR(config.cfg);

    /* Write phase (register address) then repeated-start read */
    I2C_COMD_REG(base, 0) = I2C_CMD(I2C_CMD_RSTART, 0, 0, 0, 0);
    I2C_COMD_REG(base, 1) = I2C_CMD(I2C_CMD_WRITE, 1 + reg_len, 1, 0, 0);
    I2C_COMD_REG(base, 2) = I2C_CMD(I2C_CMD_RSTART, 0, 0, 0, 0);
    I2C_COMD_REG(base, 3) = I2C_CMD(I2C_CMD_WRITE, 1, 1, 0, 0);
    I2C_COMD_REG(base, 4) = I2C_CMD(I2C_CMD_READ, read_len - 1, 1, 0, 0);
    I2C_COMD_REG(base, 5) = I2C_CMD(I2C_CMD_READ, 1, 1, 0, 1); /* last=NACK */
    I2C_COMD_REG(base, 6) = I2C_CMD(I2C_CMD_STOP, 0, 0, 0, 0);

    /* Push address+W and register bytes */
    I2C_DATA_REG(base) = (addr << 1) & 0xFE;
    for (int i = 0; i < reg_len; i++)
        I2C_DATA_REG(base) = reg[i];
    /* Push address+R */
    I2C_DATA_REG(base) = (addr << 1) | 0x01;

    i2c_start_transfer(base);
    int ret = i2c_wait_done(base);
    if (ret != I2C_BUS_SUCCESS)
        return ret;

    /* Drain received bytes from FIFO */
    for (int i = 0; i < read_len; i++)
        read[i] = I2C_DATA_REG(base) & 0xFF;

    return I2C_BUS_SUCCESS;
}
