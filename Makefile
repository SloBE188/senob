FILES= ./build/boot.o ./build/vbe/vbe.o ./build/mm/heap/heap.o ./build/vbe/font.o ./build/mm/memory.o ./build/kernel.o ./build/gdt/gdt.o ./build/libk/stdiok.o ./build/interrupts/pit.o ./build/drivers/keyboard.o ./build/gdt/gdt.s.o ./build/vga/vga.o ./build/libk/memory.o ./build/interrupts/idt.o ./build/interrupts/idt.s.o ./build/io/io.s.o


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


./build/libk/stdiok.o:
	i686-elf-gcc -g -c ./kernel/libk/stdiok.c -o ./build/libk/stdiok.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra


./build/interrupts/idt.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/idt.c -o ./build/interrupts/idt.o -std=gnu99 -O2 -Wall -Wextra

./build/interrupts/idt.s.o:
	nasm -f elf ./kernel/arch/x86-32/interrupts/idt.s -o ./build/interrupts/idt.s.o

./build/interrupts/pit.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/interrupts/pit.c -o ./build/interrupts/pit.o -std=gnu99 -O2 -Wall -Wextra

./build/io/io.s.o:
	nasm -f elf ./kernel/arch/x86-32/io/io.s -o ./build/io/io.s.o


./build/drivers/keyboard.o:
	i686-elf-gcc -g -c ./drivers/keyboard/keyboard.c -o ./build/drivers/keyboard.o -std=gnu99 -O2 -Wall -Wextra

./build/mm/memory.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/mm/memory.c -o ./build/mm/memory.o -std=gnu99 -O2 -Wall -Wextra

./build/vbe/vbe.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/vbe.c -o ./build/vbe/vbe.o -std=gnu99 -O2 -Wall -Wextra

./build/vbe/font.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/font.c -o ./build/vbe/font.o -std=gnu99 -O2 -Wall -Wextra

./build/vbe/wm/window.o:
	i686-elf-gcc -g -c ./drivers/video/vbe/wm/window.c -o ./build/vbe/wm/window.o -std=gnu99 -O2 -Wall -Wextra


./build/mm/heap/heap.o:
	i686-elf-gcc -g -c ./kernel/arch/x86-32/mm/heap/heap.c -o ./build/mm/heap/heap.o -std=gnu99 -O2 -Wall -Wextra


clean:
	rm -rf ${FILES}
	rm -rf ./senob/boot/senob.bin