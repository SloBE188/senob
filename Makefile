FILES= ./build/boot.o ./build/kernel.o ./build/gdt/gdt.o ./build/gdt/gdt.s.o ./build/vga/vga.o


all: $(FILES) ./senob/boot/senob.bin
	grub-mkrescue -o senob.iso senob/



./senob/boot/senob.bin:
	i686-elf-gcc -T linker.ld -o ./senob/boot/senob.bin -ffreestanding -O2 -nostdlib $(FILES) -lgcc

./build/kernel.o:
	i686-elf-gcc -c ./kernel/arch/x86-32/kernel.c -o ./build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/boot.o:
	nasm -f elf ./kernel/arch/x86-32/boot/boot.s -o ./build/boot.o

./build/gdt/gdt.o:
	i686-elf-gcc -c ./kernel/arch/x86-32/gdt/gdt.c -o ./build/gdt/gdt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

./build/gdt/gdt.s.o:
	nasm -f elf ./kernel/arch/x86-32/gdt/gdt.s -o ./build/gdt/gdt.s.o

./build/vga/vga.o:
	i686-elf-gcc -c ./drivers/video/vga/vga.c -o ./build/vga/vga.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf ${FILES}
	rm -rf ./senob/boot/senob.bin