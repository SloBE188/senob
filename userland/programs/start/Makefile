all:
	nasm -f elf ./start.s -o ./build/start.o
	i686-elf-gcc -g -c ./main.c -o ./build/main.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive -nostdlib -lgcc
	i686-elf-ld -o start.bin -T ./linker.ld ./build/start.o ./build/main.o


clean:
	rm -rf ./build/start.o
	rm -rf ./build/main.o
	rm -rf ./start.bin