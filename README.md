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
- Tons of coffee (don't tell me you don't need this, I won't believe you)

Also, I know the Makefile are still a bit crappy, but I'm too lazy to do a clean thing. Feel free to make it cleaner by contributing!

# HowTo : Compile
First you need to compile userland.

You can compile rLibC by entering the "libc" directory and running "make."

The userland can then be compiled by entering the "michiru" directory and running make in every sub-directory.

This should be modified soon, as I'll be integrating a "cpio-as-rootfs" component in Riku asap.

Right now, you can spawn a single /init binary by putting it in the "boot/boot/bin" directory.

Compiling the kernel then should be as easy as running "make all".

# HowTo : Run
The compilation toolchain should generate a "riku.iso" file, loadable through QEMU.
