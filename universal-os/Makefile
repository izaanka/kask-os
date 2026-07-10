# Toolchain Configurations for Intel macOS (via Homebrew)
X86_CC      = x86_64-elf-gcc
X86_AS      = x86_64-elf-as
X86_LD      = x86_64-elf-ld
X86_OBJCOPY = x86_64-elf-objcopy

ARM_CC      = aarch64-elf-gcc
ARM_AS      = aarch64-elf-as
ARM_LD      = aarch64-elf-ld
ARM_OBJCOPY = aarch64-elf-objcopy

# Common freestanding compilation flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdlib

all: os_core.fat

# --- Intel Target Build Sequence ---
kernel_x86.elf: arch/x86_64/boot.s arch/x86_64/hardware.c kernel/main.c
	$(X86_AS) arch/x86_64/boot.s -o x86_boot.o
	$(X86_CC) -c arch/x86_64/hardware.c -o x86_hw.o $(CFLAGS)
	$(X86_CC) -c kernel/main.c -o x86_main.o $(CFLAGS)
	$(X86_LD) -T arch/x86_64/linker.ld x86_boot.o x86_hw.o x86_main.o -o kernel_x86.elf

kernel_x86.bin: kernel_x86.elf
	$(X86_OBJCOPY) -O binary kernel_x86.elf kernel_x86.bin

# --- ARM Target Build Sequence ---
kernel_arm.elf: arch/arm64/boot.s arch/arm64/hardware.c kernel/main.c
	$(ARM_AS) arch/arm64/boot.s -o arm_boot.o
	$(ARM_CC) -c arch/arm64/hardware.c -o arm_hw.o $(CFLAGS)
	$(ARM_CC) -c kernel/main.c -o arm_main.o $(CFLAGS)
	$(ARM_LD) -T arch/arm64/linker.ld arm_boot.o arm_hw.o arm_main.o -o kernel_arm.elf

kernel_arm.bin: kernel_arm.elf
	$(ARM_OBJCOPY) -O binary kernel_arm.elf kernel_arm.bin

# --- Merger Sequence ---
os_core.fat: kernel_x86.bin kernel_arm.bin
	python3 packer.py

# --- Execution Simulation Targets ---
run-x86: kernel_x86.elf
	qemu-system-x86_64 -kernel kernel_x86.elf

run-arm: kernel_arm.elf
	qemu-system-aarch64 -M virt -cpu cortex-a57 -device virtio-gpu-device -kernel kernel_arm.elf
clean:
	rm -f *.o *.elf *.bin *.fat

# --- ISO Creation Target ---
os_core.iso: kernel_x86.elf os_core.fat
	cp kernel_x86.elf iso_root/boot/
	cp os_core.fat iso_root/boot/
	grub-mkrescue -o os_core.iso iso_root

run-iso: os_core.iso
	qemu-system-x86_64 -cdrom os_core.iso

# --- Universal VM Disk Generation Target ---
os_disk.img: kernel_x86.bin kernel_arm.bin os_core.fat
	# 1. Create a clean staging directory
	mkdir -p disk_staging/EFI/BOOT
	
	# 2. Copy the native binary slices into their standard UEFI designated positions
	cp kernel_x86.bin disk_staging/EFI/BOOT/BOOTX64.EFI
	cp kernel_arm.bin disk_staging/EFI/BOOT/BOOTAA64.EFI
	cp os_core.fat disk_staging/EFI/BOOT/os_core.fat
	
	# 3. Allocate a blank 64MB raw hard drive image file
	dd if=/dev/zero of=os_disk.img bs=1M count=64
	
	# 4. Format the raw hard disk image with a standard FAT32 file system partition
	mkfs.vfat -F 32 os_disk.img
	
	# 5. Copy our entire UEFI structural tree directly into the virtual hard drive
	mcopy -i os_disk.img -s disk_staging/* ::/
	
	@echo "[SUCCESS] Generated universal VM disk image: os_disk.img"

# --- Execute hard disk mode simulations ---
run-vm-x86: os_disk.img
	qemu-system-x86_64 -bios /usr/local/share/qemu/edk2-x86_64-code.fd -drive file=os_disk.img,format=raw

run-vm-arm: os_disk.img
	qemu-system-aarch64 -M virt -cpu cortex-a57 -bios /usr/local/share/qemu/edk2-aarch64-code.fd -drive file=os_disk.img,format=raw
