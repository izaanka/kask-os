# Kask OS

An operating system designed to run on any device.

Kask OS is a 64-bit multi-architecture operating system that features a custom kernel, a built-in shell, and a hierarchical in-memory file system.

## 🚀 Features

- **Multi-Architecture 64-Bit Support:**
  - **x86_64:** Boots from GRUB via Multiboot1 in 32-bit protected mode, automatically configures identity-mapped page tables (using 2MB pages), transitions to 64-bit long mode, and executes the C kernel.
  - **ARM64:** Custom boot stub that parks non-boot cores, clears BSS, and jumps to C code. Includes a PL011 UART serial console driver with ANSI color support for QEMU virt, Raspberry Pi, and Apple Silicon.
  - **RISC-V 64:** Conforms to OpenSBI conventions with a dedicated boot stub and an NS16550A UART serial driver.
- **Hierarchical File System & Shell:**
  - Features a fully functional directory tree structure.
  - Navigate directories and manage files just like a standard Unix environment.
  - **Supported Commands:** `cd`, `pwd`, `mkdir`, `rmdir`, `touch`, `rm`, `tree`, `ls`, `cat`, `echo`, `uname`, `whoami`, `hostname`, `uptime`, `date`, `clear`, `reboot`, `shutdown`, and `help`.
- **Cross-Platform Build System:**
  - Build on Linux or macOS (Intel/Apple Silicon).
  - Target `x86_64`, `ARM64`, and `RISC-V 64` from a single `Makefile`.
  - Automated ISO image creation with GRUB.

## 🛠️ Build Requirements

Please see the requirement files in the `kask-os` subdirectory for installation instructions:
- [Linux Requirements](kask-os/requirements-linux.md)
- [macOS Requirements](kask-os/requirements-macos.md)

- Also if youre using macos instead of linux, change ```grub-mkrescue -o kask-os.iso iso_root``` in Makefile to ```i686-elf-grub-mkrescue -o kask-os.iso iso_root```

Alternatively, you can quickly install all dependencies via the Makefile:
```bash
# On Linux (Ubuntu/Debian)
make install-deps-linux

# On macOS (Homebrew)
make install-deps-macos
```

## 🏗️ Building and Running

1. **Navigate to the source directory:**
   ```bash
   cd kask-os
   ```

2. **Build all targets:**
   ```bash
   make all
   make iso
   make arm
   make riscv
   ```

3. **Run in QEMU:**
   - **x86_64 (GUI mode via ISO):**
     ```bash
     make run-iso
     ```
   - **ARM64 (Serial shell mode):**
     ```bash
     make run-arm
     ```
   - **RISC-V 64 (Serial shell mode):**
     ```bash
     make run-riscv
     ```

## 📝 License
See the [LICENSE](LICENSE) file for more information.
