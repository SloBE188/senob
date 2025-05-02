FILES= ./build/boot.o ./build/vbe/vbe.o ./build/mm/pmm.o ./build/libk/libk.o ./build/sys/process.o ./build/sys/sched.o ./build/sys/mp.o ./build/sys/trampoline.o ./build/sys/startup.o ./build/sys/smp.o ./build/sys/spinlock.o ./build/sys/lapic.o ./build/sys/thread.o ./build/vbe/wm/window.o ./build/libk/string.o ./build/syscalls/syscalls.o ./build/fatfs/diskio.o ./build/fatfs/ff.o ./build/mm/paging/paging.s.o ./build/disk/ramdisk.o ./build/mm/paging/paging.o ./build/mm/heap/heap.o ./build/vbe/font.o ./build/kernel.o ./build/gdt/gdt.o ./build/libk/stdiok.o ./build/interrupts/pit.o ./build/drivers/keyboard.o ./build/gdt/gdt.s.o ./build/vga/vga.o ./build/libk/memory.o ./build/interrupts/idt.o ./build/interrupts/idt.s.o ./build/io/io.s.o
FLAGS= -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive

all: $(FILES) ./senob/boot/senob.bin programs ./senob/boot/ramdisk.img iso


iso:
	rm -f ./out/senob.iso
	mkdir -p out
	docker run --platform=linux/amd64 --rm \
	  -v "$$(pwd):/work" -w /work/senob debian bash -c "\
	    apt update && \
	    apt install -y grub-pc-bin grub-common xorriso mtools && \
	    grub-mkrescue -o /work/senob.iso ."



./senob/boot/senob.bin:
	i686-elf-gcc -T linker.ld -o ./senob/boot/senob.bin -ffreestanding -O2 -nostartfiles $(FILES) -L$(HOME)/opt/cross/i686-elf/lib -lc -lgcc

./senob/boot/ramdisk.img:
	dd if=/dev/zero of=./senob/boot/ramdisk.img bs=8M count=1
	mkfs.vfat ./senob/boot/ramdisk.img
	# here i c1an copy files to /mnt (ramdisk)
	mcopy -i senob/boot/ramdisk.img ./test.txt ::test.txt
	mcopy -i senob/boot/ramdisk.img ./userland/programs/shell/shell.bin ::shell.bin

./build/kernel.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/kernel.c -o ./build/kernel.o $(FLAGS)

./build/boot.o:
	nasm -f elf ./kernel/arch/x86-32/boot/boot.s -o ./build/boot.o

./build/disk/ramdisk.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/disk/ramdisk.c -o ./build/disk/ramdisk.o $(FLAGS)

./build/gdt/gdt.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/gdt/gdt.c -o ./build/gdt/gdt.o $(FLAGS)

./build/gdt/gdt.s.o:
	nasm -f elf ./kernel/arch/x86-32/gdt/gdt.s -o ./build/gdt/gdt.s.o

./build/vga/vga.o:
	i686-elf-gcc -g -c ./drivers/video/vga/vga.c -o ./build/vga/vga.o $(FLAGS)

./build/libk/memory.o:
	i686-elf-gcc -g -c ./kernel/libk/memory.c -o ./build/libk/memory.o $(FLAGS)


./build/libk/stdiok.o:
	i686-elf-gcc -g -c ./kernel/libk/stdiok.c -o ./build/libk/stdiok.o $(FLAGS)

./build/libk/string.o:
	i686-elf-gcc -g -c ./kernel/libk/string.c -o ./build/libk/string.o $(FLAGS)


./build/interrupts/idt.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/idt.c -o ./build/interrupts/idt.o $(FLAGS)

./build/interrupts/idt.s.o:
	nasm -f elf ./kernel/arch/x86-32/interrupts/idt.s -o ./build/interrupts/idt.s.o

./build/interrupts/pit.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/pit.c -o ./build/interrupts/pit.o $(FLAGS)

./build/io/io.s.o:
	nasm -f elf ./kernel/arch/x86-32/io/io.s -o ./build/io/io.s.o

./build/drivers/keyboard.o:
	i686-elf-gcc -g -c ./drivers/keyboard/keyboard.c -o ./build/drivers/keyboard.o $(FLAGS)

./build/vbe/vbe.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/vbe.c -o ./build/vbe/vbe.o $(FLAGS)

./build/vbe/font.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/font.c -o ./build/vbe/font.o $(FLAGS)

./build/vbe/wm/window.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/wm/window.c -o ./build/vbe/wm/window.o $(FLAGS)


./build/mm/heap/heap.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/mm/heap/heap.c -o ./build/mm/heap/heap.o $(FLAGS)

./build/mm/paging/paging.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/mm/paging/paging.c -o ./build/mm/paging/paging.o $(FLAGS)

./build/mm/paging/paging.s.o:
	nasm -f elf ./kernel/arch/x86-32/mm/paging/paging.s -o ./build/mm/paging/paging.s.o

./build/mm/pmm.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/mm/PMM/pmm.c -o ./build/mm/pmm.o $(FLAGS)

./build/sys/process.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/process.c -o ./build/sys/process.o $(FLAGS)

./build/fatfs/diskio.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/fatfs/diskio.c -o ./build/fatfs/diskio.o $(FLAGS)

./build/fatfs/ff.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/fatfs/ff.c -o ./build/fatfs/ff.o $(FLAGS)


./build/syscalls/syscalls.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/syscalls/syscalls.c -o ./build/syscalls/syscalls.o $(FLAGS)


./build/libk/libk.o:
	i686-elf-gcc -g -c ./kernel/libk/libk.c -o ./build/libk/libk.o $(FLAGS)

./build/sys/process.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/process.c -o ./build/sys/process.o $(FLAGS)

./build/sys/thread.o:
	nasm -f elf ./kernel/arch/x86-32/sys/thread.s -o ./build/sys/thread.o

./build/sys/mp.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/mp.c -o ./build/sys/mp.o $(FLAGS)

./build/sys/lapic.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/lapic.c -o ./build/sys/lapic.o $(FLAGS)

./build/sys/trampoline.o:
	nasm -f elf ./kernel/arch/x86-32/sys/trampoline.s -o ./build/sys/trampoline.o

./build/sys/smp.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/smp.c -o ./build/sys/smp.o $(FLAGS)

./build/sys/startup.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/startup.c -o ./build/sys/startup.o $(FLAGS)

./build/sys/sched.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/sched.c -o ./build/sys/sched.o $(FLAGS)


./build/sys/spinlock.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/sys/spinlock.c -o ./build/sys/spinlock.o $(FLAGS)


.PHONY: programs
programs:
	@echo "Running make in programs/start..."
	cd ./userland/programs/stdlib/ && $(MAKE) all
	cd ./userland/programs/test/ && $(MAKE) all
	cd ./userland/programs/doomgeneric/ && $(MAKE) all
	cd ./userland/programs/shell/ && $(MAKE) all


programs_clean:
	cd ./userland/programs/stdlib/ && $(MAKE) clean
	cd ./userland/programs/test/ && $(MAKE) clean
	cd ./userland/programs/doomgeneric/ && $(MAKE) clean
	cd ./userland/programs/shell/ && $(MAKE) clean

clean: programs_clean
	rm -rf ${FILES}
	rm -rf ./senob/boot/senob.bin
	rm -rf ./senob/boot/ramdisk.img