FILES = ./build/blank.o
BIN_FILE = ./build/blank.bin
OBJ_FILE = ./build/blank_bin.o

all: $(OBJ_FILE)

#build blank.o from blank.s
./build/blank.o: ./blank.s
	nasm -f elf ./blank.s -o ./build/blank.o

#link blank.bin from blank.o
./build/blank.bin: $(FILES)
	i686-elf-gcc -ffreestanding -O2 -nostdlib -T userland.ld $(FILES) -o $(BIN_FILE)

#create blank_bin.o from blank.bin
$(OBJ_FILE): $(BIN_FILE)
	ld -r -b binary -o $(OBJ_FILE) $(BIN_FILE)

clean:
	rm -f ${FILES}
	rm -f ./build/blank.bin
	rm -f ./build/blank_bin.o