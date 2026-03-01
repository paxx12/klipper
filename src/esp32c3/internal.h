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

/* eFuse - unique chip ID (MAC address in block 1) */
#define EFUSE_BASE                  0x60008800U
#define EFUSE_RD_MAC_SPI_SYS_0_REG  MMIO32(EFUSE_BASE + 0x044)
#define EFUSE_RD_MAC_SPI_SYS_1_REG  MMIO32(EFUSE_BASE + 0x048)

#endif // internal.h
