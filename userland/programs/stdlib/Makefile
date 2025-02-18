CC = i686-elf-gcc
ASM = nasm
AR = ar
CFLAGS = -g -std=gnu99 -O2 -Wall -Wextra -ffreestanding
ASMFLAGS = -f elf

LIB_SRC = ./senob.s ./print.c ./proc.c
LIB_OBJ = ./build/syscall.o ./build/senob.o ./build/print.o ./build/proc.o
LIBRARY = ./build/libsenob.a

all: $(LIBRARY)

$(LIBRARY): $(LIB_OBJ)
	$(AR) rcs $@ $(LIB_OBJ)

./build/senob.o: ./senob.s
	$(ASM) $(ASMFLAGS) $< -o $@

./build/print.o: ./print.c
	$(CC) $(CFLAGS) -c $< -o $@

./build/proc.o: ./proc.c
	$(CC) $(CFLAGS) -c $< -o $@

./build/vbe.o: ./vbe.c
	$(CC) $(CFLAGS) -c $< -o $@

./build/syscall.o: ./syscall.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf ./build/*.o $(LIBRARY)
