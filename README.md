# Senob

Senob is a operating system with a monolithic kernel designed for the x86 architecture, with plans to support the ARM architecture in the future.

## Features

- Designed for x86 architecture
- Planned support for ARM architecture
- Open-source and freely available under the GNU General Public License v3.0

DOOM on senob:
![image](https://github.com/user-attachments/assets/d323cbfe-97a8-43c4-82af-122e2e423c9e)


### Prerequisites

- An x86-based computer or emulator (as an example QEMU)
- A GCC Cross-Compiler
- NASM
- GNU Make

### Building Senob

To build Senob, follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/SloBE188/senob.git
   cd senob
  
2. Build the operating system Senob
   ```sh
   ./build.sh
  
3. Running Senob
   ```sh
   qemu-system-i386 -machine pc-q35-5.1 -smp 4 -cdrom senob.iso
