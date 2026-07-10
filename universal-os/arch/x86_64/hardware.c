#include "../../shared/kernel.h"

void hw_init(void) {
    // Clear screen or reset VGA cursors if necessary
}

void hw_print(const char* str) {
    volatile char* vga = (volatile char*)0xB8000;
    // Advance to safe position on screen to avoid overwriting firmware messages
    vga += 320; 
    for(int i = 0; str[i] != '\0'; i++) {
        *vga++ = str[i];
        *vga++ = 0x0F; // White text on black background
    }
}
