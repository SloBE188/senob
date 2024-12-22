all:
	i686-elf-gcc -g -c ./main.c -o ./build/main.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive -nostdlib -lgcc
	i686-elf-ld -o test.bin -T ./linker.ld  ./build/main.o ../stdlib/build/libsenob.a

clean:
	rm -rf ./build/main.o
	rm -rf ./test.bin