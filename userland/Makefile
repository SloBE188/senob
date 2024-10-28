FILES = ./build/blank.o
TARGET = ./build/blank.bin

all: $(TARGET)

./build/blank.o: ./blank.s
	nasm -f elf ./blank.s -o ./build/blank.o

./build/blank.bin: $(FILES)
	i686-elf-gcc -ffreestanding -O2 -nostdlib -T userland.ld $(FILES) -o ./build/blank.bin


clean:
	rm -f ${FILES}
	rm -f ./build/blank.bin