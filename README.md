# microarch

**microarch** is a minimalistic, 8-bit, extensible, RISC, Harvard, load-store computer architecture intended for small low-power (or power constrained) microcontrollers.

It offers 8-bit data address space and up to 16-bit instruction address space, allowing for up to 256 bytes of directly addressable data memory and up to 65025 instructions.

The standard defines 8 registers available to the programmer with a powerful, unique register addressing system.


##### Navigation and usage

- ```README.md``` - You are here.
- ```LICENSE``` - License in place for the project.
- ```microarch-ISA.pdf``` - Full ISA documentation for the architecture.
- ```base-core/``` - A Verilog design of an example core (a reference implementation of the ISA), with a Verilator/SystemC testbench (this part of the project has its own readme file).
- ```src/``` - A software development kit (SDK): assembler, disassembler and an emulator with debugging functions. Written in C++.
- ```entry/``` - Entry points for SDK (there are two - tests and user CLI) - separated from ```src``` so that it's easier to use the SDK as a C++ library.
- ```CMakeLists.txt``` - Build system for the SDK (there are 3 CMake targets: ```microarch-test``` for tests, ```microarch``` for the CLI program and ```microarch-common``` for use as a library)
- ```examples/``` - Example programs written in microarch assembly
