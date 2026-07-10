#include "../../shared/kernel.h"
#include <stdint.h>

/* ============================================================
 * Kask OS — ARM64 PL011 UART Driver
 * Works on: QEMU virt, Raspberry Pi, Apple Silicon VMs
 * ============================================================ */

/* PL011 UART base address for QEMU virt machine */
#define UART_BASE 0x09000000

#define UART_DR   (*(volatile uint32_t*)(UART_BASE + 0x00))  /* Data Register */
#define UART_FR   (*(volatile uint32_t*)(UART_BASE + 0x18))  /* Flag Register */
#define UART_IBRD (*(volatile uint32_t*)(UART_BASE + 0x24))  /* Integer Baud Rate */
#define UART_FBRD (*(volatile uint32_t*)(UART_BASE + 0x28))  /* Fractional Baud Rate */
#define UART_LCRH (*(volatile uint32_t*)(UART_BASE + 0x2C))  /* Line Control */
#define UART_CR   (*(volatile uint32_t*)(UART_BASE + 0x30))  /* Control Register */
#define UART_IMSC (*(volatile uint32_t*)(UART_BASE + 0x38))  /* Interrupt Mask */

#define FR_TXFF  (1 << 5)  /* Transmit FIFO Full */
#define FR_RXFE  (1 << 4)  /* Receive FIFO Empty */

void hw_init(void) {
    /* Disable UART */
    UART_CR = 0;

    /* Set baud rate (115200 @ 24MHz clock) */
    UART_IBRD = 13;
    UART_FBRD = 1;

    /* 8 data bits, no parity, 1 stop bit, enable FIFO */
    UART_LCRH = (3 << 5) | (1 << 4);

    /* Mask all interrupts */
    UART_IMSC = 0;

    /* Enable UART, TX, RX */
    UART_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void hw_putchar(char c) {
    /* Wait until TX FIFO is not full */
    while (UART_FR & FR_TXFF)
        ;
    UART_DR = (uint32_t)c;
    /* Send \r before \n for proper terminal behavior */
    if (c == '\n') {
        while (UART_FR & FR_TXFF)
            ;
        UART_DR = '\r';
    }
}

void hw_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        hw_putchar(str[i]);
    }
}

char hw_getchar(void) {
    /* Wait until RX FIFO is not empty */
    while (UART_FR & FR_RXFE)
        ;
    char c = (char)(UART_DR & 0xFF);

    /* Echo the character */
    if (c == '\r') c = '\n';
    return c;
}

void hw_clear_screen(void) {
    /* ANSI escape: clear screen + move cursor home */
    hw_print("\033[2J\033[H");
}

void hw_set_color(uint8_t color) {
    /* Map VGA color codes to ANSI escape sequences */
    static const char* ansi_fg[] = {
        "\033[30m", "\033[34m", "\033[32m", "\033[36m",   /* 0-3: black,blue,green,cyan */
        "\033[31m", "\033[35m", "\033[33m", "\033[37m",   /* 4-7: red,magenta,brown,lgray */
        "\033[90m", "\033[94m", "\033[92m", "\033[96m",   /* 8-B: dgray,lblue,lgreen,lcyan */
        "\033[91m", "\033[95m", "\033[93m", "\033[97m"    /* C-F: lred,lmagenta,yellow,white */
    };
    uint8_t fg = color & 0x0F;
    if (fg < 16) {
        hw_print(ansi_fg[fg]);
    }
}

uint8_t hw_get_color(void) {
    return 0x0F; /* Always report white — ANSI state not tracked */
}
