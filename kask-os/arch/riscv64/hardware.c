#include "../../shared/kernel.h"
#include <stdint.h>

/* ============================================================
 * Kask OS — RISC-V NS16550A UART Driver
 * QEMU virt machine UART at 0x10000000
 * ============================================================ */

#define UART_BASE 0x10000000

#define UART_THR (*(volatile uint8_t*)(UART_BASE + 0x00))  /* Transmit Holding */
#define UART_RBR (*(volatile uint8_t*)(UART_BASE + 0x00))  /* Receive Buffer */
#define UART_IER (*(volatile uint8_t*)(UART_BASE + 0x01))  /* Interrupt Enable */
#define UART_FCR (*(volatile uint8_t*)(UART_BASE + 0x02))  /* FIFO Control */
#define UART_LCR (*(volatile uint8_t*)(UART_BASE + 0x03))  /* Line Control */
#define UART_LSR (*(volatile uint8_t*)(UART_BASE + 0x05))  /* Line Status */
#define UART_DLL (*(volatile uint8_t*)(UART_BASE + 0x00))  /* Divisor Latch Low */
#define UART_DLH (*(volatile uint8_t*)(UART_BASE + 0x01))  /* Divisor Latch High */

#define LSR_TX_EMPTY  (1 << 5)
#define LSR_RX_READY  (1 << 0)

void hw_init(void) {
    /* Disable interrupts */
    UART_IER = 0x00;

    /* Set baud rate: enable DLAB */
    UART_LCR = 0x80;
    UART_DLL = 0x01;   /* 115200 baud (divisor = 1 at default clock) */
    UART_DLH = 0x00;

    /* 8 data bits, no parity, 1 stop bit, disable DLAB */
    UART_LCR = 0x03;

    /* Enable FIFO, clear buffers */
    UART_FCR = 0x07;

    /* No modem control needed for QEMU */
}

void hw_putchar(char c) {
    while (!(UART_LSR & LSR_TX_EMPTY))
        ;
    UART_THR = (uint8_t)c;
    if (c == '\n') {
        while (!(UART_LSR & LSR_TX_EMPTY))
            ;
        UART_THR = '\r';
    }
}

void hw_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        hw_putchar(str[i]);
    }
}

char hw_getchar(void) {
    while (!(UART_LSR & LSR_RX_READY))
        ;
    char c = (char)UART_RBR;
    if (c == '\r') c = '\n';
    return c;
}

void hw_clear_screen(void) {
    hw_print("\033[2J\033[H");
}

void hw_set_color(uint8_t color) {
    static const char* ansi_fg[] = {
        "\033[30m", "\033[34m", "\033[32m", "\033[36m",
        "\033[31m", "\033[35m", "\033[33m", "\033[37m",
        "\033[90m", "\033[94m", "\033[92m", "\033[96m",
        "\033[91m", "\033[95m", "\033[93m", "\033[97m"
    };
    uint8_t fg = color & 0x0F;
    if (fg < 16) hw_print(ansi_fg[fg]);
}

uint8_t hw_get_color(void) {
    return 0x0F;
}
