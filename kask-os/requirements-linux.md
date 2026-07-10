# Kask OS -- Linux Build Requirements (Ubuntu/Debian)

## Quick Install

```bash
sudo apt-get install -y gcc gcc-multilib nasm make python3 \
    qemu-system-x86 qemu-system-arm qemu-system-misc \
    gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu \
    grub-pc-bin grub-common xorriso mtools
```

Or: `make install-deps-linux`

## Packages

| Package | Purpose |
|---------|---------|
| `gcc` + `gcc-multilib` | x86 host compiler with 32/64 support |
| `gcc-aarch64-linux-gnu` | ARM64 cross-compiler |
| `gcc-riscv64-linux-gnu` | RISC-V 64 cross-compiler |
| `nasm` | x86 assembler (optional) |
| `qemu-system-x86` | x86_64 VM |
| `qemu-system-arm` | ARM64 VM |
| `qemu-system-misc` | RISC-V VM |
| `grub-pc-bin` + `grub-common` | GRUB ISO creation |
| `xorriso` | ISO filesystem |
| `mtools` | FAT tools for disk images |
| `python3` | Packer script |
