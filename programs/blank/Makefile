all:
	nasm -f elf ./blank.s -o ./build/blank.o
	i686-elf-gcc -g -T ./linker.ld -o ./bin/blank.bin -ffreestanding -O0 -nostdlib -fpic -g ./build/blank.o

clean:
	rm -rf ./build/blank.o
	rm -rf ./bin/blank.bin