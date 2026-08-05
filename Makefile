AsmCompile = nasm
CCompile = gcc
Linker = ld
ISO = Phosphor.iso
ISOPackage = xorriso
OVMF_CODE = ovmf/OVMF.fd
VNC_PORT ?= 5900
VNC_DISPLAY = $(shell echo $$(($(VNC_PORT) - 5900)))

CFLAGS = -Wall -Wextra -std=gnu11 \
		 -ffreestanding \
		 -fno-stack-protector \
		 -fno-stack-check \
		 -fno-pic -fno-pie \
		 -mno-red-zone \
		 -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
		 -mcmodel=kernel \
		 -march=x86-64 \
		 -Isrc

LDFLAGS = -T linker.lds -nostdlib -z max-page-size=0x1000

all: $(ISO)

gdt.o: src/gdt.c
	$(CCompile) $(CFLAGS) -c src/gdt.c -o gdt.o

interrupts.o: src/interrupts.c
	$(CCompile) $(CFLAGS) -c src/interrupts.c -o interrupts.o

kernel.o: src/kernel.c
	$(CCompile) $(CFLAGS) -c src/kernel.c -o kernel.o

interrupts_asm.o: src/interrupts.s
	$(AsmCompile) src/interrupts.s -f elf64 -o interrupts_asm.o

kernel.bin: gdt.o interrupts.o kernel.o interrupts_asm.o
	$(Linker) $(LDFLAGS) gdt.o interrupts.o kernel.o interrupts_asm.o -o kernel.bin

$(ISO): kernel.bin
	mkdir -p iso_root/boot
	cp kernel.bin iso_root/boot
	cp limine.conf iso_root/boot
	cp limine.conf iso_root
	cp cd-uefi.bin iso_root/
	$(ISOPackage) -as mkisofs -R -r -J --efi-boot cd-uefi.bin -efi-boot-part --efi-boot-image -o $(ISO) iso_root

run: $(ISO)
	./run-vnc-workflow.sh

#ignore this
clean:
	rm -rf *.o kernel.bin $(ISO) iso_root
#ignore this

.PHONY: all run clean