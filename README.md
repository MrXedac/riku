# The Riku Operating System 
The Riku Operating System is a home-made, hobby operating system for Intel x64 architectures.

It is divided into two main components :
- loader, which is the x86 bootstrap code
- kern, which is the x64 kernel code

# Required stuff
In order to compile and run Riku, some standard development environment is required :
- x86_64-elf-gcc toolchain (mandatory)
- grub2 (grub-mkrescue to be honest, to generate the riku.iso bootable image)
- nasm (my favourite assembler)
- qemu-system-x86_64 (a virtual machine to run the kernel on is always a good thing)
- GNU make (yep)
- Tons of coffee (don't tell me you don't need this, I won't believe you)

Also, I know the Makefile are still a bit crappy, but I'm too lazy to do a clean thing. Feel free to make it cleaner by contributing!

# HowTo : Compile
Compiling should be as easy as running "make all".

# HowTo : Run
The compilation toolchain should generate a "riku.iso" file, loadable through QEMU.
