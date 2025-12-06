#pragma once

#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>
#include <out/buffer/executable.hpp>

#include "asm/x86/writer.hpp"
#include "asm/x86/argument/registry.hpp"

class MicroInst;
struct CoreState;

struct ExecutableCore {

	static constexpr const char* CODE_START = "code";

	CoreState* core;
	asmio::ExecutableBuffer code;

	bool retrieveMemory = true;

	ExecutableCore(CoreState* core, asmio::SegmentedBuffer& codeBuffer)
		: core(core), code(asmio::to_executable(codeBuffer)) {
	}

	void operator()() {
		code.scall<void>(CODE_START, this);
	}

};

struct Peripheral {

	using WriteFunction = std::function<void(uint8_t)>;
	using ReadFunction = std::function<uint8_t()>;

	WriteFunction write = [] (uint8_t) noexcept {};
	ReadFunction read = [] () noexcept { return 0; };

	Peripheral() = default;

	Peripheral(const WriteFunction& write, const ReadFunction& read)
		: write(write), read(read) {
	}

};


struct CoreState {

	// Constants for JIT labels
	static constexpr const char* CLEANUP_CODE = "exit";
	static constexpr const char* PROGRAM_MEMORY = "program";
	static constexpr const char* DATA_MEMORY = "data";
	static constexpr const char* REGISTERS = "registers"; // Registers are not constantly stored in this memory, but are dumped there at the end of each JIT run.
	static constexpr const char* EXECUTABLE_CORE_POINTER = "exec_core_ptr";
	static constexpr const char* INSTRUCTION_OFFSETS = "instruction_offsets";
	static constexpr const char* PROGRAM_COUNTER = "instruction_to_run";
	static constexpr const char* FLAGS = "flags";
	static constexpr const char* MEMORY_WRITE_FUNCTION = "memory_write_func";
	static constexpr const char* MEMORY_READ_FUNCTION = "memory_read_func";
	static constexpr const char* PERIPHERAL_WRITE_FUNCTION = "peripheral_write_func";
	static constexpr const char* PERIPHERAL_READ_FUNCTION = "peripheral_read_func";
	static constexpr const char* MEMORY_WRITE_MAPPING = "memory_write_mapping";
	static constexpr const char* MEMORY_READ_MAPPING = "memory_read_mapping";
	static constexpr const char* SEGMENT_REGISTER_WRITE_FUNCTION = "segment_register_write_func";
	static constexpr const char* SEGMENT_REGISTER_READ_FUNCTION = "segment_register_read_func";
	static constexpr uint16_t SEGMENT_REGISTER_ADDRESS = 0x0003;

	static constexpr int DATA_MEMORY_SEGMENT_SIZE = 256;
	static constexpr int REGISTER_COUNT = 8;

	static constexpr asmio::x86::Registry REGISTRY_MAPPING[] = {
		asmio::x86::R8L,
		asmio::x86::R9L,
		asmio::x86::R10L,
		asmio::x86::R11L,
		asmio::x86::R12L,
		asmio::x86::R13L,
		asmio::x86::R14L,
		asmio::x86::R15L,
	};

	std::unordered_set<uint16_t> breakpoints;

	unsigned int data_memory_size = 2 * DATA_MEMORY_SEGMENT_SIZE;
	std::unordered_map<uint16_t, Peripheral> peripherals;

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
	uint8_t ram[DATA_MEMORY_SEGMENT_SIZE * 256] = {0};

	ControlByte ctr;

	uint8_t cf : 1 = 0;
	uint8_t zf : 1 = 0;

	bool memorySegmented() const;

	/// Read the value from a set of registers, given a regset bitmask
	uint8_t read(uint8_t regset) const;

	/// Write a value to a set of registers, given a regset bitmask
	void write(uint8_t regset, uint8_t value);

	/// Get the current condition bitmask matching the condition bits
	uint8_t getFlagMask() const;

	/// Turn this program back to a source form
	std::string disassemble() const;

	/// Interpret the program
	void run(size_t count = std::numeric_limits<size_t>::max());

	/// Compile the program into a JIT executable buffer
	ExecutableCore jit();

	/// Get register value as binary string
	std::string reg(int regnum) const;

	/// Reset core state back into a default state
	void reset();

	private:

	static void jitPadCode(asmio::x86::BufferWriter& writer, asmio::SegmentedBuffer& buffer, size_t size, const std::function<void()>& code);

};
