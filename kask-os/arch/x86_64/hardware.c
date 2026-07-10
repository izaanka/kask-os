#include "../../shared/kernel.h"
#include <stdint.h>

/* ============================================================
 * Kask OS — VGA Text Mode Driver + PS/2 Keyboard (x86_64)
 * ============================================================ */

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#define VGA_COLOR(fg, bg) ((bg) << 4 | (fg))
#define VGA_BLACK     0
#define VGA_BLUE      1
#define VGA_GREEN     2
#define VGA_CYAN      3
#define VGA_RED       4
#define VGA_MAGENTA   5
#define VGA_BROWN     6
#define VGA_LGRAY     7
#define VGA_DGRAY     8
#define VGA_LBLUE     9
#define VGA_LGREEN    10
#define VGA_LCYAN     11
#define VGA_LRED      12
#define VGA_LMAGENTA  13
#define VGA_YELLOW    14
#define VGA_WHITE     15

static int cursor_row = 0;
static int cursor_col = 0;
static uint8_t current_color = VGA_COLOR(VGA_LGREEN, VGA_BLACK);
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_MEMORY;

/* ---- Port I/O ---- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ---- VGA Cursor Hardware ---- */
static void update_hardware_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_row * VGA_WIDTH + cursor_col);
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void enable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

/* ---- Scroll ---- */
static void scroll(void) {
    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (uint16_t)current_color << 8 | ' ';
    }
    cursor_row = VGA_HEIGHT - 1;
}

/* ---- Public API ---- */

void hw_clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)current_color << 8 | ' ';
    }
    cursor_row = 0;
    cursor_col = 0;
    update_hardware_cursor();
}

void hw_set_color(uint8_t color) {
    current_color = color;
}

uint8_t hw_get_color(void) {
    return current_color;
}

void hw_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\t') {
        cursor_col = (cursor_col + 8) & ~7;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = (uint16_t)current_color << 8 | ' ';
        }
    } else {
        vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = (uint16_t)current_color << 8 | (uint8_t)c;
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }

    if (cursor_row >= VGA_HEIGHT) {
        scroll();
    }
    update_hardware_cursor();
}

void hw_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        hw_putchar(str[i]);
    }
}

void hw_init(void) {
    enable_cursor();
    hw_clear_screen();
}

/* ============================================================
 * PS/2 Keyboard Driver (Polling, US QWERTY)
 * ============================================================ */

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64

static const char scancode_to_ascii[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0,  ' ', 0,  0,0,0,0,0,0,0,0,0,0,  0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const char scancode_to_ascii_shift[128] = {
    0,   27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,   '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*', 0,  ' ', 0,  0,0,0,0,0,0,0,0,0,0,  0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static int shift_held = 0;

char hw_getchar(void) {
    while (1) {
        while (!(inb(KB_STATUS_PORT) & 1))
            ;
        uint8_t scancode = inb(KB_DATA_PORT);

        if (scancode == 0x2A || scancode == 0x36) { shift_held = 1; continue; }
        if (scancode == 0xAA || scancode == 0xB6) { shift_held = 0; continue; }
        if (scancode & 0x80) continue;

        char c = shift_held ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
        if (c != 0) return c;
    }
}
