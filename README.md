# The Riku Operating System 
The Riku Operating System is a home-made, hobby operating system for Intel x64 architectures.

It is divided into two main components :
- loader, which is the x86 bootstrap code
- kern, which is the x64 kernel code

# HowTo : Compile
Compiling should be as easy as running "make all".

# HowTo : Run
The compilation toolchain should generate a "riku.iso" file, loadable through QEMU.
