all:
	nasm -f elf ./start.s -o ./build/start.o
	nasm -f elf ./senob.s -o ./build/senob.o
	i686-elf-gcc -g -c ./main.c -o ./build/main.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive -nostdlib -lgcc
	i686-elf-gcc -g -c ./print.c -o ./build/print.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive -nostdlib -lgcc
	i686-elf-gcc -g -c ./proc.c -o ./build/proc.o -std=gnu99 -O2 -Wall -Wextra -ffreestanding -fpermissive -nostdlib -lgcc
	i686-elf-ld -o start.bin -T ./linker.ld ./build/start.o ./build/main.o ./build/print.o ./build/senob.o ./build/proc.o


clean:
	rm -rf ./build/start.o
	rm -rf ./build/main.o
	rm -rf ./build/print.o
	rm -rf ./build/senob.o
	rm -rf ./start.bin
