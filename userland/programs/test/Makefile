all:
	nasm -f elf ./start.s -o ./build/start.o
	i686-elf-gcc -g -c ./main.c -o ./build/main.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive
	i686-elf-gcc -T linker.ld -o test.bin -nostdlib -ffreestanding -O2 \
	./build/main.o ./build/start.o \
	-L$(HOME)/opt/cross/i686-elf/lib -lc \
	-L../stdlib/build -lsenob \
	-lgcc





clean:	
	rm -rf ./build/main.o
	rm -rf ./test.bin