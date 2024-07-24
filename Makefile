FILES= ./build/boot.o ./build/kernel.o ./build/gdt/gdt.o ./build/interrupts/pit.o ./build/gdt/gdt.s.o ./build/vga/vga.o ./build/libk/memory.o ./build/interrupts/idt.o ./build/interrupts/idt.s.o ./build/io/io.s.o


all: $(FILES) ./senob/boot/senob.bin
	grub-mkrescue -o senob.iso senob/



./senob/boot/senob.bin:
	i686-elf-gcc -T linker.ld -o ./senob/boot/senob.bin -ffreestanding -O2 -nostdlib $(FILES) -lgcc

./build/kernel.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/kernel.c -o ./build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/boot.o:
	nasm -f elf ./kernel/arch/x86-32/boot/boot.s -o ./build/boot.o

./build/gdt/gdt.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/gdt/gdt.c -o ./build/gdt/gdt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/gdt/gdt.s.o:
	nasm -f elf ./kernel/arch/x86-32/gdt/gdt.s -o ./build/gdt/gdt.s.o

./build/vga/vga.o:
	i686-elf-gcc -g -c ./drivers/video/vga/vga.c -o ./build/vga/vga.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/libk/memory.o:
	i686-elf-gcc -g -c ./kernel/libk/memory.c -o ./build/libk/memory.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/interrupts/idt.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/idt.c -o ./build/interrupts/idt.o -std=gnu99 -O2 -Wall -Wextra

./build/interrupts/idt.s.o:
	nasm -f elf ./kernel/arch/x86-32/interrupts/idt.s -o ./build/interrupts/idt.s.o

./build/interrupts/pit.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/pit.c -o ./build/interrupts/pit.o -std=gnu99 -O2 -Wall -Wextra

./build/io/io.s.o:
	nasm -f elf ./kernel/arch/x86-32/io/io.s -o ./build/io/io.s.o



clean:
	rm -rf ${FILES}
	rm -rf ./senob/boot/senob.bin