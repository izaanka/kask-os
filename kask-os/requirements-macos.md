# Kask OS -- macOS Build Requirements

## Quick Install (Homebrew)

```bash
brew install x86_64-elf-gcc aarch64-elf-gcc riscv64-elf-gcc nasm qemu xorriso mtools python3
```

Or: `make install-deps-macos`

## Packages

| Package | Purpose |
|---------|---------|
| `x86_64-elf-gcc` | x86_64 ELF cross-compiler |
| `aarch64-elf-gcc` | ARM64 cross-compiler |
| `riscv64-elf-gcc` | RISC-V 64 cross-compiler |
| `nasm` | x86 assembler (optional) |
| `qemu` | All VM targets |
| `xorriso` | ISO creation |
| `mtools` | FAT disk tools |
| `python3` | Packer script |

## Notes

- **Intel Mac**: QEMU can use HVF for x86 acceleration
- **Apple Silicon**: Cross-compiles x86/RV targets; ARM64 uses HVF
