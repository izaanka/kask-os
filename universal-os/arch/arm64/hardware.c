#include "../../shared/kernel.h"

// QEMU Virt Board uses PL011 UART mapped to physical memory address 0x09000000
#define UART0_DR ((volatile unsigned int*)(0x09000000))

void hw_init(void) {
    // Hardware registers initialized by QEMU natively
}

void hw_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        *UART0_DR = (unsigned int)(str[i]); // Output character to serial pipeline
    }
}
