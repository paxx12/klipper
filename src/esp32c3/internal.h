#ifndef __ESP32C3_INTERNAL_H
#define __ESP32C3_INTERNAL_H

#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

/* System timer (SYSTIMER) - runs at 16MHz */
#define SYSTIMER_BASE               0x60023000U
#define SYSTIMER_CONF_REG           MMIO32(SYSTIMER_BASE + 0x000)
#define SYSTIMER_UNIT0_OP_REG       MMIO32(SYSTIMER_BASE + 0x004)
#define SYSTIMER_TARGET0_HI_REG     MMIO32(SYSTIMER_BASE + 0x01C)
#define SYSTIMER_TARGET0_LO_REG     MMIO32(SYSTIMER_BASE + 0x020)
#define SYSTIMER_TARGET0_CONF_REG   MMIO32(SYSTIMER_BASE + 0x034)
#define SYSTIMER_VALUE0_HI_REG      MMIO32(SYSTIMER_BASE + 0x040)
#define SYSTIMER_VALUE0_LO_REG      MMIO32(SYSTIMER_BASE + 0x044)
#define SYSTIMER_COMP0_LOAD_REG     MMIO32(SYSTIMER_BASE + 0x050)
#define SYSTIMER_INT_ENA_REG        MMIO32(SYSTIMER_BASE + 0x064)
#define SYSTIMER_INT_RAW_REG        MMIO32(SYSTIMER_BASE + 0x068)
#define SYSTIMER_INT_CLR_REG        MMIO32(SYSTIMER_BASE + 0x06C)

#define SYSTIMER_CLK_EN             (1u << 31)
#define SYSTIMER_UNIT0_WORK_EN      (1u << 0)
#define SYSTIMER_UNIT0_UPDATE       (1u << 31)
#define SYSTIMER_UNIT0_VALUE_VALID  (1u << 30)
#define SYSTIMER_TARGET0_ALARM_EN   (1u << 31)
#define SYSTIMER_INT_COMP0          (1u << 0)

/* Timer Group 0 (watchdog) */
#define TIMG0_BASE                  0x6001F000U
#define TIMG0_WDTCONFIG0_REG        MMIO32(TIMG0_BASE + 0x048)
#define TIMG0_WDTCONFIG1_REG        MMIO32(TIMG0_BASE + 0x04C)
#define TIMG0_WDTCONFIG2_REG        MMIO32(TIMG0_BASE + 0x050)
#define TIMG0_WDTFEED_REG           MMIO32(TIMG0_BASE + 0x060)
#define TIMG0_WDTWPROTECT_REG       MMIO32(TIMG0_BASE + 0x064)
#define TIMG_WDT_WKEY               0x50D83AA1U
#define TIMG_WDT_EN                 (1u << 31)
#define TIMG_WDT_STG0_RESET_SYS     (3u << 28)
#define TIMG_WDT_FLASHBOOT_MOD_EN   (1u << 14)

/* RTC_CNTL - system control and RTC watchdog */
#define RTC_CNTL_BASE               0x60008000U
#define RTC_CNTL_OPTIONS0_REG       MMIO32(RTC_CNTL_BASE + 0x000)
#define RTC_CNTL_WDTCONFIG0_REG     MMIO32(RTC_CNTL_BASE + 0x090)
#define RTC_CNTL_WDTWPROTECT_REG    MMIO32(RTC_CNTL_BASE + 0x0A4)
#define RTC_CNTL_SWD_CONF_REG       MMIO32(RTC_CNTL_BASE + 0x0B0)
#define RTC_CNTL_SWD_WPROTECT_REG   MMIO32(RTC_CNTL_BASE + 0x0B4)
#define RTC_WDT_WKEY                0x50D83AA1U
#define RTC_CNTL_SWD_WKEY           0x8F1D312AU
#define RTC_CNTL_SWD_DISABLE        (1u << 31)
#define RTC_CNTL_SW_SYS_RST         (1u << 31)

/* UART0 - APB clock (80MHz after ROM init) */
#define UART0_BASE                  0x60000000U
#define UART0_FIFO_REG              MMIO32(UART0_BASE + 0x000)
#define UART0_STATUS_REG            MMIO32(UART0_BASE + 0x01C)
#define UART0_CONF0_REG             MMIO32(UART0_BASE + 0x020)
#define UART0_CLKDIV_REG            MMIO32(UART0_BASE + 0x014)
#define UART0_INT_RAW_REG           MMIO32(UART0_BASE + 0x004)
#define UART0_INT_ENA_REG           MMIO32(UART0_BASE + 0x00C)
#define UART0_INT_CLR_REG           MMIO32(UART0_BASE + 0x010)
#define UART_STATUS_RXFIFO_CNT_MASK 0xFFu
#define UART_STATUS_TXFIFO_CNT_S    16
#define UART_STATUS_TXFIFO_CNT_MASK (0xFFu << 16)
#define UART_CONF0_8N1              0x1Cu   /* bit_num=3<<2, stop_bit_num=1<<4 */

/* GPIO */
#define GPIO_BASE                   0x60004000U
#define GPIO_OUT_REG                MMIO32(GPIO_BASE + 0x004)
#define GPIO_OUT_W1TS_REG           MMIO32(GPIO_BASE + 0x008)
#define GPIO_OUT_W1TC_REG           MMIO32(GPIO_BASE + 0x00C)
#define GPIO_ENABLE_REG             MMIO32(GPIO_BASE + 0x020)
#define GPIO_ENABLE_W1TS_REG        MMIO32(GPIO_BASE + 0x024)
#define GPIO_ENABLE_W1TC_REG        MMIO32(GPIO_BASE + 0x028)
#define GPIO_IN_REG                 MMIO32(GPIO_BASE + 0x03C)
#define GPIO_PIN_REG(n)             MMIO32(GPIO_BASE + 0x074 + (n) * 4)
#define GPIO_FUNC_OUT_SEL_REG(n)    MMIO32(GPIO_BASE + 0x554 + (n) * 4)
#define GPIO_FUNC_IN_SEL_REG(s)     MMIO32(GPIO_BASE + 0x154 + (s) * 4)

/* GPIO_FUNC_OUT_SEL: 0x100 = simple GPIO output via GPIO_OUT_REG */
#define GPIO_OUT_SEL_SIMPLE         0x100U

/* IO_MUX */
#define IO_MUX_BASE                 0x60009000U
#define IO_MUX_GPIO_REG(n)          MMIO32(IO_MUX_BASE + (n) * 4)
#define IO_MUX_FUN_WPD              (1u << 7)
#define IO_MUX_FUN_WPU              (1u << 8)
#define IO_MUX_FUN_IE               (1u << 9)
#define IO_MUX_FUN_DRV_2            (2u << 10)
#define IO_MUX_MCU_SEL_GPIO         (1u << 12)   /* func1 = GPIO matrix */

/* Number of GPIO pins on ESP32-C3 */
#define NUM_GPIO 22

/* APB_SARADC - digital ADC controller */
#define APB_SARADC_BASE             0x60040000U
#define APB_SARADC_CTRL_REG         MMIO32(APB_SARADC_BASE + 0x000)
#define APB_SARADC_CTRL2_REG        MMIO32(APB_SARADC_BASE + 0x004)
#define APB_SARADC_SAR_PATT_TAB1_REG MMIO32(APB_SARADC_BASE + 0x01C)
#define APB_SARADC_SAR1_DATA_REG    MMIO32(APB_SARADC_BASE + 0x02C)
#define APB_SARADC_INT_RAW_REG      MMIO32(APB_SARADC_BASE + 0x040)
#define APB_SARADC_INT_CLR_REG      MMIO32(APB_SARADC_BASE + 0x048)
#define APB_SARADC_CLKM_CONF_REG    MMIO32(APB_SARADC_BASE + 0x054)

/* APB_SARADC_CTRL bits */
#define SARADC_START_FORCE          (1u << 0)
#define SARADC_START                (1u << 1)
#define SARADC_SAR_CLK_DIV(x)       ((x) << 7)
#define SARADC_SAR_PATT_LEN(x)      ((x) << 15) /* 0 = 1 entry */
#define SARADC_SAR_PATT_RESET       (1u << 23)
#define SARADC_XPD_SAR_FORCE_ON     (2u << 29)

/* APB_SARADC_CLKM_CONF bits */
#define SARADC_CLKM_DIV_NUM(x)      ((x) << 0)
#define SARADC_CLKM_CLK_EN          (1u << 20)
#define SARADC_CLKM_SEL_XTAL        (0u << 21)

/* APB_SARADC_INT bit */
#define SARADC_ADC1_DONE            (1u << 17)

/* SYSTEM - peripheral clock gating */
#define SYSTEM_BASE                 0x600C0000U
#define SYSTEM_PERIP_CLK_EN0_REG    MMIO32(SYSTEM_BASE + 0x018)
#define SYSTEM_PERIP_RST_EN0_REG    MMIO32(SYSTEM_BASE + 0x01C)
#define SYSTEM_APB_SARADC_CLK_EN    (1u << 28)
#define SYSTEM_I2C0_CLK_EN          (1u << 7)
#define SYSTEM_I2C1_CLK_EN          (1u << 16)

/* SPI2 (GPSPI2/FSPI) */
#define SPI2_BASE                   0x60024000U
#define SPI2_CMD_REG                MMIO32(SPI2_BASE + 0x000)
#define SPI2_CTRL_REG               MMIO32(SPI2_BASE + 0x008)
#define SPI2_CLOCK_REG              MMIO32(SPI2_BASE + 0x018)
#define SPI2_USER_REG               MMIO32(SPI2_BASE + 0x01C)
#define SPI2_USER1_REG              MMIO32(SPI2_BASE + 0x020)
#define SPI2_MS_DLEN_REG            MMIO32(SPI2_BASE + 0x028)
#define SPI2_MISC_REG               MMIO32(SPI2_BASE + 0x02C)
#define SPI2_W0_REG                 MMIO32(SPI2_BASE + 0x058)

/* SPI2_CMD bits */
#define SPI_USR                     (1u << 17)

/* SPI2_USER bits */
#define SPI_DOUTDIN                 (1u << 0)   /* full-duplex */
#define SPI_CK_OUT_EDGE             (1u << 9)   /* CPHA */
#define SPI_USR_MOSI                (1u << 27)
#define SPI_USR_MISO                (1u << 28)

/* SPI2_MISC bits */
#define SPI_CK_IDLE_EDGE            (1u << 29)  /* CPOL */

/* SPI2 clock enable in SYSTEM_PERIP_CLK_EN0 */
#define SYSTEM_SPI2_CLK_EN          (1u << 6)

/* IO_MUX func2 = direct SPI2 function for GPIO2/6/7 */
#define IO_MUX_MCU_SEL_FUNC2        (2u << 12)

/* I2C0/I2C1 */
#define I2C0_BASE                   0x60013000U
#define I2C1_BASE                   0x60027000U
#define I2C_SCL_LOW_PERIOD_REG(b)   MMIO32((b) + 0x000)
#define I2C_CTR_REG(b)              MMIO32((b) + 0x004)
#define I2C_SR_REG(b)               MMIO32((b) + 0x008)
#define I2C_TO_REG(b)               MMIO32((b) + 0x00C)
#define I2C_FIFO_ST_REG(b)          MMIO32((b) + 0x014)
#define I2C_FIFO_CONF_REG(b)        MMIO32((b) + 0x018)
#define I2C_DATA_REG(b)             MMIO32((b) + 0x01C)
#define I2C_INT_RAW_REG(b)          MMIO32((b) + 0x020)
#define I2C_INT_CLR_REG(b)          MMIO32((b) + 0x024)
#define I2C_INT_ENA_REG(b)          MMIO32((b) + 0x028)
#define I2C_SDA_HOLD_REG(b)         MMIO32((b) + 0x030)
#define I2C_SDA_SAMPLE_REG(b)       MMIO32((b) + 0x034)
#define I2C_SCL_HIGH_PERIOD_REG(b)  MMIO32((b) + 0x038)
#define I2C_SCL_START_HOLD_REG(b)   MMIO32((b) + 0x040)
#define I2C_SCL_RSTART_SETUP_REG(b) MMIO32((b) + 0x044)
#define I2C_SCL_STOP_HOLD_REG(b)    MMIO32((b) + 0x048)
#define I2C_SCL_STOP_SETUP_REG(b)   MMIO32((b) + 0x04C)
#define I2C_FILTER_CFG_REG(b)       MMIO32((b) + 0x050)
#define I2C_CLK_CONF_REG(b)         MMIO32((b) + 0x054)
#define I2C_COMD_REG(b, n)          MMIO32((b) + 0x058 + (n) * 4)

/* I2C_CTR bits */
#define I2C_MS_MODE                 (1u << 4)   /* 1 = master */
#define I2C_TRANS_START             (1u << 5)
#define I2C_CLK_EN                  (1u << 8)

/* I2C_SR bits */
#define I2C_BUS_BUSY                (1u << 4)

/* I2C_INT bits */
#define I2C_INT_TRANS_COMPLETE      (1u << 7)
#define I2C_INT_ACK_ERR             (1u << 10)
#define I2C_INT_ARBITRATION_LOST    (1u << 5)
#define I2C_INT_ERROR_MASK          (I2C_INT_ACK_ERR | I2C_INT_ARBITRATION_LOST)

/* I2C command opcodes */
#define I2C_CMD_RSTART              0x0
#define I2C_CMD_WRITE               0x1
#define I2C_CMD_READ                0x2
#define I2C_CMD_STOP                0x3
#define I2C_CMD_END                 0x4

#define I2C_CMD(op, byte_num, ack_en, ack_exp, ack_val) \
    ((op) | ((byte_num) << 8) | ((ack_en) << 16) \
     | ((ack_exp) << 17) | ((ack_val) << 18))

/* eFuse - unique chip ID (MAC address in block 1) */
#define EFUSE_BASE                  0x60008800U
#define EFUSE_RD_MAC_SPI_SYS_0_REG  MMIO32(EFUSE_BASE + 0x044)
#define EFUSE_RD_MAC_SPI_SYS_1_REG  MMIO32(EFUSE_BASE + 0x048)

#endif // internal.h
