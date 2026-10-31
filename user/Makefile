CROSS_A64 := aarch64-linux-gnu-
CROSS_A32 := arm-linux-gnueabihf-
CROSS_X64 := x86_64-linux-gnu-
CROSS_X86 := i686-linux-gnu-
CFLAGS := -ffreestanding -nostdlib -nostdinc -O2 -Wall -mgeneral-regs-only
all: init_a64.bin init_a32.bin init_x64.bin init_x86.bin
init_a64.bin: apps/init.c lib/syscall.c lib/crt0.S
	@$(CROSS_A64)gcc $(CFLAGS) -c lib/crt0.S -o crt0.o
	@$(CROSS_A64)gcc $(CFLAGS) -c apps/init.c -o init.o
	@$(CROSS_A64)gcc $(CFLAGS) -c lib/syscall.c -o syscall.o
	@$(CROSS_A64)ld -Ttext=0x200000 --entry=_start -o init.elf crt0.o init.o syscall.o
	@$(CROSS_A64)objcopy -O binary init.elf $@
	@rm -f *.o init.elf
init_a32.bin: apps/init.c lib/syscall.c lib/crt0.S
	@$(CROSS_A32)gcc $(CFLAGS) -c lib/crt0.S -o crt0.o
	@$(CROSS_A32)gcc $(CFLAGS) -c apps/init.c -o init.o
	@$(CROSS_A32)gcc $(CFLAGS) -c lib/syscall.c -o syscall.o
	@$(CROSS_A32)ld -Ttext=0x200000 --entry=_start -o init.elf crt0.o init.o syscall.o
	@$(CROSS_A32)objcopy -O binary init.elf $@
	@rm -f *.o init.elf
init_x64.bin: apps/init.c lib/syscall.c lib/crt0.S
	@$(CROSS_X64)gcc $(CFLAGS) -mno-red-zone -c lib/crt0.S -o crt0.o
	@$(CROSS_X64)gcc $(CFLAGS) -mno-red-zone -c apps/init.c -o init.o
	@$(CROSS_X64)gcc $(CFLAGS) -mno-red-zone -c lib/syscall.c -o syscall.o
	@$(CROSS_X64)ld -Ttext=0x200000 --entry=_start -o init.elf crt0.o init.o syscall.o
	@$(CROSS_X64)objcopy -O binary init.elf $@
	@rm -f *.o init.elf
init_x86.bin: apps/init.c lib/syscall.c lib/crt0.S
	@$(CROSS_X86)gcc $(CFLAGS) -c lib/crt0.S -o crt0.o
	@$(CROSS_X86)gcc $(CFLAGS) -c apps/init.c -o init.o
	@$(CROSS_X86)gcc $(CFLAGS) -c lib/syscall.c -o syscall.o
	@$(CROSS_X86)ld -Ttext=0x200000 --entry=_start -o init.elf crt0.o init.o syscall.o
	@$(CROSS_X86)objcopy -O binary init.elf $@
	@rm -f *.o init.elf
clean:
	rm -f *.o *.elf *.bin
