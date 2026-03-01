// UART serial support on esp32c3 (interrupt-driven via RISC-V mtvec)
//
// Copyright (C) 2024  Klipper contributors
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// Signal indices (from ESP32-C3 gpio_sig_map.h):
//   U0TXD_OUT_IDX = 6, U0RXD_IN_IDX = 6
// GPIO pins: TX=GPIO21, RX=GPIO20

#include <stdint.h> // uint32_t
#include "board/serial_irq.h" // serial_rx_byte
#include "internal.h" // UART0_*, IO_MUX_*, GPIO_FUNC_*
#include "sched.h" // DECL_INIT

#define UART_GPIO_TX    21
#define UART_GPIO_RX    20
#define U0TXD_OUT_IDX   6
#define U0RXD_IN_IDX    6

#define UART_INT_RXFIFO_FULL    (1u << 0)
#define UART_INT_RXFIFO_TOUT    (1u << 8)
#define UART_INT_TXFIFO_EMPTY   (1u << 1)

/* ESP32-C3 interrupt matrix */
#define INTMTX_BASE             0x600C2000U
#define INTMTX_UART0_INT_MAP    MMIO32(INTMTX_BASE + 0x00C)

/* CPU interrupt number used for UART0 (1..31) */
#define UART0_CPU_INT           1

/* RISC-V mie bit for external interrupt 1 */
#define MIE_MEIE                (1u << 11)
#define MIE_EXT(n)              (1u << (n))


/****************************************************************
 * Interrupt handler
 ****************************************************************/

void __attribute__((interrupt("machine")))
uart0_isr(void)
{
    uint32_t status = UART0_INT_RAW_REG;

    if (status & (UART_INT_RXFIFO_FULL | UART_INT_RXFIFO_TOUT)) {
        while (UART0_STATUS_REG & UART_STATUS_RXFIFO_CNT_MASK)
            serial_rx_byte((uint8_t)(UART0_FIFO_REG & 0xFF));
        UART0_INT_CLR_REG = UART_INT_RXFIFO_FULL | UART_INT_RXFIFO_TOUT;
    }

    if (status & UART_INT_TXFIFO_EMPTY) {
        uint8_t data;
        while (!((UART0_STATUS_REG & UART_STATUS_TXFIFO_CNT_MASK)
                 >> UART_STATUS_TXFIFO_CNT_S >= 127)) {
            int ret = serial_get_tx_byte(&data);
            if (ret)
                break;
            UART0_FIFO_REG = data;
        }
        if (((UART0_STATUS_REG & UART_STATUS_TXFIFO_CNT_MASK)
             >> UART_STATUS_TXFIFO_CNT_S) == 0)
            UART0_INT_ENA_REG &= ~UART_INT_TXFIFO_EMPTY;
        UART0_INT_CLR_REG = UART_INT_TXFIFO_EMPTY;
    }
}

void
serial_enable_tx_irq(void)
{
    UART0_INT_ENA_REG |= UART_INT_TXFIFO_EMPTY;
}


/****************************************************************
 * Init
 ****************************************************************/

static void
serial_route_pins(void)
{
    /* Route U0TXD output signal to GPIO21 */
    GPIO_FUNC_OUT_SEL_REG(UART_GPIO_TX) = U0TXD_OUT_IDX;
    IO_MUX_GPIO_REG(UART_GPIO_TX) = IO_MUX_MCU_SEL_GPIO | IO_MUX_FUN_DRV_2;

    /* Route GPIO20 to U0RXD input signal */
    GPIO_FUNC_IN_SEL_REG(U0RXD_IN_IDX) = UART_GPIO_RX;
    IO_MUX_GPIO_REG(UART_GPIO_RX) = IO_MUX_MCU_SEL_GPIO | IO_MUX_FUN_IE
                                    | IO_MUX_FUN_WPU | IO_MUX_FUN_DRV_2;
}

static void
setup_uart0_interrupt(void)
{
    /* Route UART0 peripheral interrupt to CPU interrupt UART0_CPU_INT */
    INTMTX_UART0_INT_MAP = UART0_CPU_INT;

    /* Register ISR in vectored mtvec table.
     * The mtvec base must be 4-byte aligned; each entry is 4 bytes apart
     * in vectored mode (base + 4*cause). We install uart0_isr at
     * cause UART0_CPU_INT using a trampoline approach via the __mtvt
     * attribute if the toolchain supports it; otherwise fall through to
     * a direct mtvec write. */
    extern void uart0_isr(void);
    uintptr_t vec = (uintptr_t)uart0_isr;

    /* Set mtvec in direct mode pointing to our handler.
     * In direct mode all interrupts jump to mtvec base. If the toolchain
     * supports vectored mode (mtvec[1:0]=1), prefer that via a jump table,
     * but direct mode is simpler for a single source. */
    asm volatile("csrw mtvec, %0" :: "r"(vec | 0));

    /* Enable UART0 interrupt in MIE: external bit + specific line */
    asm volatile("csrs mie, %0" :: "r"(MIE_MEIE));
    asm volatile("csrs mstatus, %0" :: "r"(1u << 3));  /* MIE bit */
}

void
serial_init(void)
{
    uint32_t div = 80000000 / CONFIG_SERIAL_BAUD;
    UART0_CLKDIV_REG = div & 0xFFFFF;
    UART0_CONF0_REG = UART_CONF0_8N1;

    serial_route_pins();

    UART0_INT_CLR_REG = 0xFFFFFFFF;
    UART0_INT_ENA_REG = UART_INT_RXFIFO_FULL | UART_INT_RXFIFO_TOUT;

    setup_uart0_interrupt();
}
DECL_INIT(serial_init);
