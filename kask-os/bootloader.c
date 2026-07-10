#include <stdint.h>

#define FAT_MAGIC 0xCAFEBABE

typedef struct {
    uint32_t cpu_type;
    uint32_t offset;
    uint32_t size;
    uint32_t load_address;
} __attribute__((packed)) arch_descriptor_t;

typedef struct {
    uint32_t magic;
    uint32_t arch_count;
    arch_descriptor_t archs[];
} __attribute__((packed)) fat_header_t;

// Parses the fat file loaded in memory and returns the pointer to the native code
void* parse_fat_binary(void* fat_file_base, uint32_t target_cpu) {
    fat_header_t* header = (fat_header_t*)fat_file_base;
    
    // Convert Big-Endian magic from packer.py to native host representation if needed
    // For this example, we assume matching endianness or manual conversion
    if (header->magic != 0xBEBAFECA && header->magic != FAT_MAGIC) {
        return 0; // Invalid file format
    }

    uint32_t count = __builtin_bswap32(header->arch_count);

    for (uint32_t i = 0; i < count; i++) {
        uint32_t cpu = __builtin_bswap32(header->archs[i].cpu_type);
        if (cpu == target_cpu) {
            uint32_t offset = __builtin_bswap32(header->archs[i].offset);
            return (void*)((uintptr_t)fat_file_base + offset);
        }
    }
    return 0; // Architecture not found
}
