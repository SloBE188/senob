FILES= ./build/boot.o ./build/kernel.o ./build/gdt/gdt.o ./build/gdt/gdt.s.o ./build/vga/vga.o ./build/libk/memset.o


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

./build/libk/memset.o:
	i686-elf-gcc -g -c ./kernel/libk/memset.c -o ./build/libk/memset.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf ${FILES}
	rm -rf ./senob/boot/senob.bin