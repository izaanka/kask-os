#include "../../shared/kernel.h"
#include <stdint.h>

/* ============================================================
 * Kask OS — ARM64 Hardware Driver
 * Runtime detection: Cortex-A72 (part 0xD08) = Pi 4 (BCM2711)
 *                    Cortex-A53 (part 0xD03) = Pi Zero2W/Pi3 (BCM2837)
 * GPIO 14/15 are mux'd to UART0 (Alt0). UART clock = 48 MHz.
 * ============================================================ */

#define PERIPH_PI3  0x3F000000u   /* BCM2837 (Pi Zero2W, Pi 3) */
#define PERIPH_PI4  0xFE000000u   /* BCM2711 (Pi 4) */

static volatile uint32_t* _gpio = (volatile uint32_t*)0;
static volatile uint32_t* _uart = (volatile uint32_t*)0;
static uint32_t _periph_base = 0;

/* ---- CPU part detection ---- */
static uint32_t detect_periph_base(void) {
    uint64_t midr;
    __asm__ volatile("mrs %0, midr_el1" : "=r"(midr));
    /* PartNum = bits[15:4]; Cortex-A72 = 0xD08 → Pi 4 */
    return (((uint32_t)(midr >> 4)) & 0xFFFu) == 0xD08u
           ? PERIPH_PI4 : PERIPH_PI3;
}

/* ---- Busy-wait N NOPs ---- */
static void spin(uint32_t n) {
    while (n--) __asm__ volatile("nop");
}

/* ---- GPIO pin function select (3-bit field per pin) ----
 * Registers GPFSEL0-5 at GPIO_BASE + 0x00..0x14, 10 pins each.
 * Alt0 = 0b100 = 4 (UART0 TXD/RXD on GPIO 14/15).
 */
static void gpio_fsel(uint32_t pin, uint32_t func) {
    volatile uint32_t* fsel = _gpio + (pin / 10u); /* +0,+1,+2... */
    uint32_t shift = (pin % 10u) * 3u;
    uint32_t v = *fsel;
    v &= ~(7u << shift);
    v |=  (func & 7u) << shift;
    *fsel = v;
}

/* ---- BCM2837 pull-up/down disable (GPPUD + GPPUDCLK) ---- */
static void gpio_pull_off_2837(uint32_t pin) {
    volatile uint32_t* gppud     = _gpio + (0x94u / 4u);
    volatile uint32_t* gppudclk0 = _gpio + (0x98u / 4u);
    *gppud     = 0u;
    spin(150u);
    *gppudclk0 = 1u << pin;
    spin(150u);
    *gppud     = 0u;
    *gppudclk0 = 0u;
}

/* ---- BCM2711 pull disable (GPPUPPDN0, 2-bits per GPIO, 00=no pull) ---- */
static void gpio_pull_off_2711(uint32_t pin) {
    volatile uint32_t* r = _gpio + (0xE4u / 4u) + (pin / 16u);
    uint32_t shift = (pin % 16u) * 2u;
    *r = (*r) & ~(3u << shift);
}

/* ---- UART0 (PL011) register helper ---- */
#define UART_OFF_DR   0x00u
#define UART_OFF_FR   0x18u
#define UART_OFF_IBRD 0x24u
#define UART_OFF_FBRD 0x28u
#define UART_OFF_LCRH 0x2Cu
#define UART_OFF_CR   0x30u
#define UART_OFF_ICR  0x44u
#define FR_TXFF (1u << 5)
#define FR_RXFE (1u << 4)

static inline void   uw(uint32_t off, uint32_t v){ *(_uart + off/4u) = v; }
static inline uint32_t ur(uint32_t off)          { return *(_uart + off/4u); }

/* ============================================================
 * hw_init — detect SoC, configure GPIO 14/15, init PL011 UART
 * ============================================================ */
void hw_init(void) {
    _periph_base = detect_periph_base();
    _gpio = (volatile uint32_t*)(uintptr_t)(_periph_base + 0x200000u);
    _uart = (volatile uint32_t*)(uintptr_t)(_periph_base + 0x201000u);

    /* Disable UART and clear interrupts */
    uw(UART_OFF_CR, 0u);
    uw(UART_OFF_ICR, 0x7FFu);

    /* GPIO 14 = TXD0, GPIO 15 = RXD0, Alt0 = func 4 */
    gpio_fsel(14u, 4u);
    gpio_fsel(15u, 4u);

    /* Disable pull-up/down on UART pins */
    if (_periph_base == PERIPH_PI4) {
        gpio_pull_off_2711(14u);
        gpio_pull_off_2711(15u);
    } else {
        gpio_pull_off_2837(14u);
        gpio_pull_off_2837(15u);
    }

    /*
     * Baud-rate divisors for 48 MHz UART clock (RPi firmware default
     * when enable_uart=1 in config.txt) at 115200 bps:
     *   Divisor = 48000000 / (16 × 115200) = 26.0416…
     *   IBRD = 26,  FBRD = round(0.0416… × 64) = 3
     */
    uw(UART_OFF_IBRD, 26u);
    uw(UART_OFF_FBRD, 3u);

    /* 8 data bits, 1 stop bit, no parity, FIFO enable */
    uw(UART_OFF_LCRH, (3u << 5) | (1u << 4));

    /* Enable UART + TX + RX */
    uw(UART_OFF_CR, (1u << 0) | (1u << 8) | (1u << 9));
}

/* ---- TX ---- */
void hw_putchar(char c) {
    while (ur(UART_OFF_FR) & FR_TXFF);
    uw(UART_OFF_DR, (uint32_t)(unsigned char)c);
    if (c == '\n') {
        while (ur(UART_OFF_FR) & FR_TXFF);
        uw(UART_OFF_DR, '\r');
    }
}

void hw_print(const char* s) {
    while (*s) hw_putchar(*s++);
}

/* ---- RX ---- */
char hw_getchar(void) {
    while (ur(UART_OFF_FR) & FR_RXFE);
    char c = (char)(ur(UART_OFF_DR) & 0xFFu);
    return (c == '\r') ? '\n' : c;
}

/* ---- Display helpers (ANSI over serial) ---- */
void hw_clear_screen(void) { hw_print("\033[2J\033[H"); }

void hw_set_color(uint8_t color) {
    static const char* fg[16] = {
        "\033[30m","\033[34m","\033[32m","\033[36m",
        "\033[31m","\033[35m","\033[33m","\033[37m",
        "\033[90m","\033[94m","\033[92m","\033[96m",
        "\033[91m","\033[95m","\033[93m","\033[97m"
    };
    hw_print(fg[color & 0x0Fu]);
}

uint8_t hw_get_color(void) { return 0x0Fu; }
