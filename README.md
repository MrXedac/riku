# The Riku Operating System 
The Riku Operating System is a home-made, hobby operating system for Intel x64 architectures.

It is divided into two main components :
- loader, which is the x86 bootstrap code
- kern, which is the x64 kernel code

The userland stuff, such as the components of Michiru, the software stack built on top of Riku, is composed of two parts as well :
- libc, which is the source code of rLibC, the Riku's standard library implementation
- michiru, which contains the userland stuff code

# Required stuff
In order to compile and run Riku, some standard development environment is required :
- x86_64-elf-gcc toolchain (mandatory)
- grub2 (grub-mkrescue to be honest, to generate the riku.iso bootable image)
- nasm (my favourite assembler)
- qemu-system-x86_64 (a virtual machine to run the kernel on is always a good thing)
- GNU make (yep)
- libtool (for KConfig)
- Tons of coffee (don't tell me you don't need this, I won't believe you)

# HowTo : Compile
## Userland
First you need to compile userland.

You can compile rLibC by entering the "libc" directory and running "make".

The userland can then be compiled by entering the "michiru" directory and running make in every sub-directory. 
You can also run the `./make_world.sh` script in the "michiru" directory instead.

## Kernel configuration
Compile (only once) KConfig by running `./bootstrap`, `./configure` and `make` in the `kconfig/` subfolder.

Configure the kernel by running `make menuconfig` and `make config`.

## Kernel compilation
Compiling the kernel then should be as easy as running "make all".

# HowTo : Run
## Hard drive image
Create a hard drive image by running `make img` (which is a shortcut to `qemu-img create -f raw 4G`).
This image can be manipulated by mounting it as a loopback device on Linux hosts, by running `make mount` (or `make umount` to unmount it).

## QEMU emulator
Run Riku in QEMU by running `make run`, or `make debug` to run QEMU alongside GDB for debugging.
