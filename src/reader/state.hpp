#pragma once
#include <memory>
#include <vector>
#include <out/buffer/executable.hpp>

class MicroInst;

struct CoreState {

	union ControlByte {
		struct __attribute__((packed)) Flags {
			uint8_t reserved: 4;
			uint8_t interrupt: 1;
			uint8_t standby_mode: 3;
		} flags;
		uint8_t raw_byte = 0;
	};

	std::vector<std::unique_ptr<MicroInst>> rom;

	uint8_t regs[8] = {};
	uint16_t pc = 0;
	uint8_t ram[256] = {};

	ControlByte ctr;

	uint8_t cf : 1 = 0;
	uint8_t zf : 1 = 0;

	/// Read the value from a set of registers, given a regset bitmask
	uint8_t read(uint8_t regset) const;

	/// Write a value to a set of registers, given a regset bitmask
	void write(uint8_t regset, uint8_t value);

	/// Get the current condition bitmask matching the condition bits
	uint8_t getFlagMask() const;

	/// Turn this program back to a source form
	std::string disassemble();

	/// Interpret the program
	void run(size_t count = std::numeric_limits<size_t>::max());

	/// Compile the program into a JIT executable buffer
	asmio::ExecutableBuffer jit();

};
