ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
CROSS_A64 := aarch64-linux-gnu-
CROSS_A32 := arm-linux-gnueabihf-
CROSS_X64 := x86_64-linux-gnu-
CROSS_X86 := i686-linux-gnu-
CFLAGS_COMMON := -ffreestanding -nostdlib -nostdinc -fno-builtin \
                 -Wall -Wextra -O2 -fno-common -fno-stack-protector \
                 -I$(ROOT)/kernel/include
CFLAGS_A64 := $(CFLAGS_COMMON) -mgeneral-regs-only
CFLAGS_A32 := $(CFLAGS_COMMON) -march=armv7-a -marm -mfloat-abi=soft
CFLAGS_X64 := $(CFLAGS_COMMON) -mno-red-zone -mcmodel=large
CFLAGS_X86 := $(CFLAGS_COMMON) -m32

COMMON_C := \
    kernel/lib/string.c kernel/lib/printf.c kernel/lib/font8x16.c \
    kernel/mm/mm.c \
    kernel/core/arch_common.c \
    kernel/fs/vfs.c \
    kernel/proc/proc.c kernel/proc/syscall.c \
    kernel/ui/gesture.c kernel/ui/xcui_core.c kernel/ui/xcui_draw.c \
    kernel/ui/xcui_text.c kernel/ui/xcui_input.c kernel/ui/xcui_widget.c \
    kernel/ui/xcui_toast.c \
    kernel/apps/demo.c kernel/apps/panels.c kernel/apps/shell.c \
    kernel/apps/codec_test.c \
    kernel/codec/codec_common.c \
    kernel/codec/wav.c kernel/codec/mp3.c kernel/codec/mp4.c \
    kernel/codec/png.c kernel/codec/jpg.c kernel/codec/gif.c

ARM_C := kernel/core/arm_common.c kernel/core/fdt.c \
         kernel/drivers/fb.c kernel/drivers/virtio_mmio.c \
         kernel/drivers/virtio_input.c kernel/drivers/input.c
X86_C := kernel/core/x86_common.c

# ── ARM64 ──
A64_OBJS := out/a64/boot.o out/a64/mmu.o out/a64/vectors.o out/a64/context.o \
            $(patsubst %.c,out/a64/%.o,$(COMMON_C) $(ARM_C))
out/a64/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	@$(CROSS_A64)gcc $(CFLAGS_A64) -c $< -o $@
out/a64/boot.o: $(ROOT)/arch/arm64/boot.S
	@mkdir -p out/a64
	@$(CROSS_A64)gcc $(CFLAGS_A64) -c $< -o $@
out/a64/mmu.o: $(ROOT)/arch/arm64/mmu.S
	@mkdir -p out/a64
	@$(CROSS_A64)gcc $(CFLAGS_A64) -c $< -o $@
out/a64/vectors.o: $(ROOT)/arch/arm64/vectors.S
	@mkdir -p out/a64
	@$(CROSS_A64)gcc $(CFLAGS_A64) -c $< -o $@
out/a64/context.o: $(ROOT)/arch/arm64/context.S
	@mkdir -p out/a64
	@$(CROSS_A64)gcc $(CFLAGS_A64) -c $< -o $@
out/a64/kernel.elf: $(A64_OBJS) $(ROOT)/arch/arm64/linker.ld
	@$(CROSS_A64)ld -T $(ROOT)/arch/arm64/linker.ld -o $@ $(A64_OBJS)
kernel_a64.img: out/a64/kernel.elf
	@$(CROSS_A64)objcopy -O binary $< $@
	@echo "  a64: $$(stat -c%s $@)"

# ── ARM32 ──
A32_OBJS := out/a32/boot.o out/a32/mmu.o out/a32/vectors.o out/a32/context.o \
            $(patsubst %.c,out/a32/%.o,$(COMMON_C) $(ARM_C))
out/a32/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	@$(CROSS_A32)gcc $(CFLAGS_A32) -c $< -o $@
out/a32/boot.o: $(ROOT)/arch/arm32/boot.S
	@mkdir -p out/a32
	@$(CROSS_A32)gcc $(CFLAGS_A32) -c $< -o $@
out/a32/mmu.o: $(ROOT)/arch/arm32/mmu.S
	@mkdir -p out/a32
	@$(CROSS_A32)gcc $(CFLAGS_A32) -c $< -o $@
out/a32/vectors.o: $(ROOT)/arch/arm32/vectors.S
	@mkdir -p out/a32
	@$(CROSS_A32)gcc $(CFLAGS_A32) -c $< -o $@
out/a32/context.o: $(ROOT)/arch/arm32/context.S
	@mkdir -p out/a32
	@$(CROSS_A32)gcc $(CFLAGS_A32) -c $< -o $@
out/a32/kernel.elf: $(A32_OBJS) $(ROOT)/arch/arm32/linker.ld
	@$(CROSS_A32)ld -T $(ROOT)/arch/arm32/linker.ld -o $@ $(A32_OBJS)
kernel_a32.img: out/a32/kernel.elf
	@$(CROSS_A32)objcopy -O binary $< $@
	@echo "  a32: $$(stat -c%s $@)"

# ── x86_64 ──
X64_OBJS := out/x64/boot.o out/x64/vectors.o out/x64/context.o \
            $(patsubst %.c,out/x64/%.o,$(COMMON_C) $(X86_C))
out/x64/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	@$(CROSS_X64)gcc $(CFLAGS_X64) -c $< -o $@
out/x64/boot.o: $(ROOT)/arch/x86_64/boot.S
	@mkdir -p out/x64
	@$(CROSS_X64)gcc $(CFLAGS_X64) -c $< -o $@
out/x64/vectors.o: $(ROOT)/arch/x86_64/vectors.S
	@mkdir -p out/x64
	@$(CROSS_X64)gcc $(CFLAGS_X64) -c $< -o $@
out/x64/context.o: $(ROOT)/arch/x86_64/context.S
	@mkdir -p out/x64
	@$(CROSS_X64)gcc $(CFLAGS_X64) -c $< -o $@
out/x64/kernel.elf: $(X64_OBJS) $(ROOT)/arch/x86_64/linker.ld
	@$(CROSS_X64)ld -T $(ROOT)/arch/x86_64/linker.ld -o $@ $(X64_OBJS)
kernel_x64.img: out/x64/kernel.elf
	@$(CROSS_X64)objcopy -O binary $< $@
	@echo "  x64: $$(stat -c%s $@)"

# ── x86 ──
X86_OBJS := out/x86/boot.o out/x86/vectors.o out/x86/context.o \
            $(patsubst %.c,out/x86/%.o,$(COMMON_C) $(X86_C))
out/x86/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	@$(CROSS_X86)gcc $(CFLAGS_X86) -c $< -o $@
out/x86/boot.o: $(ROOT)/arch/x86/boot.S
	@mkdir -p out/x86
	@$(CROSS_X86)gcc $(CFLAGS_X86) -c $< -o $@
out/x86/vectors.o: $(ROOT)/arch/x86/vectors.S
	@mkdir -p out/x86
	@$(CROSS_X86)gcc $(CFLAGS_X86) -c $< -o $@
out/x86/context.o: $(ROOT)/arch/x86/context.S
	@mkdir -p out/x86
	@$(CROSS_X86)gcc $(CFLAGS_X86) -c $< -o $@
out/x86/kernel.elf: $(X86_OBJS) $(ROOT)/arch/x86/linker.ld
	@$(CROSS_X86)ld -T $(ROOT)/arch/x86/linker.ld -o $@ $(X86_OBJS)
kernel_x86.img: out/x86/kernel.elf
	@$(CROSS_X86)objcopy -O binary $< $@
	@echo "  x86: $$(stat -c%s $@)"

all: catin.img
catin.img: kernel_a64.img kernel_a32.img kernel_x64.img kernel_x86.img
	@echo "=== pack catin.img ==="
	@rm -f $@
	@dd if=/dev/zero of=$@ bs=1M count=96 status=none
	@mkfs.vfat -F 32 -n CATIN $@ >/dev/null
	@mcopy -i $@ kernel_a64.img ::/kernel_a64.img
	@mcopy -i $@ kernel_a32.img ::/kernel_a32.img
	@mcopy -i $@ kernel_x64.img ::/kernel_x64.img
	@mcopy -i $@ kernel_x86.img ::/kernel_x86.img
	@echo "  catin.img $$(stat -c%s $@)"

run-a64: kernel_a64.img
	qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 512M \
	    -kernel kernel_a64.img -device ramfb -device virtio-tablet-device \
	    -serial mon:stdio -display gtk
run-a32: kernel_a32.img
	qemu-system-arm -M virt,gic-version=2 -cpu cortex-a15 -m 512M \
	    -kernel kernel_a32.img -device ramfb -device virtio-tablet-device \
	    -serial mon:stdio -display gtk
run-x64: kernel_x64.img
	qemu-system-x86_64 -m 512M -kernel kernel_x64.img -serial mon:stdio -display gtk
run-x86: kernel_x86.img
	qemu-system-i386 -m 512M -kernel kernel_x86.img -serial mon:stdio -display gtk
clean:
	rm -rf out kernel_*.img catin.img
	@$(MAKE) -C $(ROOT)/user clean || true
.PHONY: all clean run-a64 run-a32 run-x64 run-x86