# Senob

Senob is a operating system designed for the x86 architecture, with plans to support the ARM architecture in the future.

## Features

- Designed for x86 architecture
- Planned support for ARM architecture
- Open-source and freely available under the GNU General Public License v3.0

### Prerequisites

- An x86-based computer or emulator
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
   qemu-system-i386 -machine pc-i440fx-5.1 -smp 4 -cdrom senob.iso
