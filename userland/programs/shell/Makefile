all:
	nasm -f elf ./start.s -o ./build/start.o
	i686-elf-gcc -g -c ./shell.c -o ./build/shell.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive
	i686-elf-gcc -T linker.ld -o shell.bin -nostdlib -ffreestanding -O2 -O0 \
	./build/shell.o ./build/start.o \
	-L$(HOME)/opt/cross/i686-elf/lib -lc \
	-L../stdlib/build -lsenob \
	-lgcc





clean:	
	rm -rf ./build/shell.o
	rm -rf ./shell.bin