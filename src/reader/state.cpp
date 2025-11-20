#include "state.hpp"

#include <out/buffer/writer.hpp>

#include "instruction.hpp"
#include "writer.hpp"
#include "asm/aarch64/argument/condition.hpp"
#include "asm/x86/writer.hpp"
#include "asm/x86/argument/registry.hpp"


static void dumpJITData(ExecutableCore* executableCore) {
	asmio::ExecutableBuffer& code = executableCore->code;
	CoreState* core = executableCore->core;
	uint8_t* dataMemory = code.address(CoreState::DATA_MEMORY);
	uint8_t* registerDumpZone = code.address(CoreState::REGISTERS);
	uint16_t* flagsDumpZone = (uint16_t*) code.address(CoreState::FLAGS);
	uint16_t* pc = (uint16_t*) code.address(CoreState::PROGRAM_COUNTER);
	 for (unsigned int i = 0; i < core->DATA_MEMORY_SIZE; i++) {
	 	core->ram[i] = dataMemory[i];
	 }
	for (unsigned int i = 0; i < CoreState::REGISTER_COUNT; i++) {
		core->regs[i] = registerDumpZone[i];
	}
	core->pc = *pc;
	core->cf = (bool) ((*flagsDumpZone) & 0x0001);
	core->zf = (bool) ((*flagsDumpZone) & 0x0040);
}

static void initJITData(ExecutableCore* executableCore) {
	asmio::ExecutableBuffer& code = executableCore->code;
	CoreState* core = executableCore->core;
	uint8_t* dataMemory = code.address(CoreState::DATA_MEMORY);
	uint8_t* registerDumpZone = code.address(CoreState::REGISTERS);
	uint16_t* flagsDumpZone = (uint16_t*) code.address(CoreState::FLAGS);
	uint16_t* pc = (uint16_t*) code.address(CoreState::PROGRAM_COUNTER);
	 for (unsigned int i = 0; i < core->DATA_MEMORY_SIZE; i++) {
	 	dataMemory[i] = core->ram[i];
	 }
	for (unsigned int i = 0; i < CoreState::REGISTER_COUNT; i++) {
		registerDumpZone[i] = core->regs[i];
	}
	*pc = core->pc;
	*flagsDumpZone = core->zf ? 0x0040 : 0x0000;
	*flagsDumpZone = core->cf ? (0x0001 | *flagsDumpZone) : *flagsDumpZone;
}


static uint64_t readPeripheral(ExecutableCore* executableCore, uint64_t address) {
	auto iterator = executableCore->core->peripherals.find(address);
	if (iterator != executableCore->core->peripherals.end()) {
		return iterator->second.readCallback();
	}
	return 0;
}

static void writePeripheral(ExecutableCore* executableCore, uint64_t address, uint64_t value) {
	auto iterator = executableCore->core->peripherals.find(address);
	if (iterator != executableCore->core->peripherals.end()) {
		iterator->second.writeCallback(value);
	}
}

/*
 * class CoreState
 */

uint8_t CoreState::read(uint8_t regset) const {
	uint32_t mask = 1;
	uint8_t acc = 0;

	for (uint32_t i = 0; i < 8; i++) {
		if (regset & mask) {
			acc |= regs[i];
		}

		mask <<= 1;
	}

	return acc;
}

void CoreState::write(uint8_t regset, uint8_t value) {
	uint32_t mask = 1;

	for (uint32_t i = 0; i < 8; i++) {
		if (regset & mask) {
			regs[i] = value;
		}

		mask <<= 1;
	}
}

uint8_t CoreState::getFlagMask() const {
	return (zf ? MicroWriter::ZF_TRUE : MicroWriter::ZF_FALSE) | (cf ? MicroWriter::CF_TRUE : MicroWriter::CF_FALSE);
}

std::string CoreState::disassemble() {
	std::string str = "";

	Labler labelr;

	for (auto& inst: rom) {
		inst->label(labelr);
	}

	for (auto& inst: rom) {

		if (labelr.has(inst->address)) {
			str += "\nl_" + std::to_string(inst->address) + ":\n";
		}

		str += "\t" + inst->string();
		str += "\n";
	}

	return str;
}

bool CoreState::memorySegmented() const {
	return (DATA_MEMORY_SIZE > DATA_MEMORY_SEGMENT_SIZE);
}

void CoreState::run(size_t count) {
	ctr.flags.standby_mode = 0;
	for (unsigned int i = 0; i < count; i++) {
		if (ctr.flags.standby_mode) {
			break;
		}
		uint16_t current_instruction = pc;

		uint16_t next_instruction = pc + 1;
		pc = next_instruction;
		if (current_instruction >= rom.size()) {
			ctr.flags.standby_mode = 1;
			break;
		}

		rom[current_instruction]->apply(*this);

	}
}

void CoreState::jitPadInstructions(asmio::x86::BufferWriter& writer, SegmentedBuffer& buffer, unsigned int size,
								   const std::function<void()>& code) {
	auto marker = buffer.current();
	code();
	unsigned int currentSize = buffer.current().offset - marker.offset;
	for (unsigned int i = 0; i < (size - currentSize); i++) {
		writer.put_nop();
	}
}


ExecutableCore CoreState::jit(std::function<void()> stopFunction) {

	using namespace asmio;
	using namespace asmio::x86;


	SegmentedBuffer buffer;
	x86::BufferWriter writer{buffer};

	writer.section(BufferSegment::R | BufferSegment::X);
	writer.label(ExecutableCore::CODE_START);

	using namespace asmio::x86;

	// Saving pointer for accessing core state and labels
	writer.put_mov(RAX, ref(RAX));
	writer.put_mov(ref(EXECUTABLE_CORE_POINTER), RAX);

	// Initializing data memory, flags and registers with data from CoreState
	writer.put_mov(RAX, (uint64_t) (&initJITData));
	writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
	writer.put_call(RAX);
	writer.put_mov(SI, ref(FLAGS));    // Flags
	writer.put_lea(RAX, REGISTERS);
	for (unsigned int i = 0; i < REGISTER_COUNT; i++) {
		// We get registers to a dedicated memory area, where tnhey were prepared by initJITData function.
		writer.put_mov(REGISTRY_MAPPING[i], ref(RAX + i));
	}

	// Jumping to the correct microarch instruction
	writer.put_mov(BX, ref(PROGRAM_COUNTER));
	writer.put_movzx(RBX, BX);
	writer.put_lea(RAX, Location(CoreState::INSTRUCTION_OFFSETS));
	writer.put_lea(RAX, RAX + RBX * 8);
	writer.put_jmp(RAX);

	writer.label(PROGRAM_MEMORY);
	std::vector<Label> instructionOffsets;
	for (auto& inst: rom) {
		auto instructionLabel = Label::make_unique();
		writer.label(instructionLabel);
		instructionOffsets.push_back(instructionLabel);
		inst->jit(writer, *this);
	}



	// Saving data memory, flags and registers back to core state
	writer.label(CLEANUP_CODE);
	writer.put_mov(ref(FLAGS), SI);    // Flags
	writer.put_lea(RAX, REGISTERS);
	for (unsigned int i = 0; i < REGISTER_COUNT; i++) {
		// We dump registers to a dedicated memory area, to be extracted by dumpJITData function.
		writer.put_mov(ref(RAX + i), REGISTRY_MAPPING[i]);
	}
	writer.put_mov(RAX, (uint64_t) (&dumpJITData));
	writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
	writer.put_call(RAX);

	// RET instruction demanded by ASMIOV library spec to return from JIT segment.
	writer.put_ret();

	// Since every microarch instruction translates to a different number of bytes of x86 code, we use an address map
	// to be able to jump to proper microarch instructions. To improve performance, instead of storing addresses, we
	// have a table with jump instructions that jump to those addresses.
	writer.label(INSTRUCTION_OFFSETS);
	for (unsigned int i = 0; i < instructionOffsets.size(); i++) {
		// Each jump instruction is 5 bytes, we pad it to 8 bytes.
		jitPadInstructions(writer, buffer, 8, [&]() { writer.put_jmp(instructionOffsets[i]); });
	}

	// The section below is used to conduct fast memory operations. Essentially we have an array with 256 jump instructions
	// corresponding to each address. These jumps dispatch operations to appropriate functions, for example reading or writing
	// memory, peripherals etc.

	if(memorySegmented() || !peripherals.empty()) {


		// This function takes the segment register (which is mapped at a fixed memory address) and applies it to a memory address,
		// stored in register RDI

		auto APPLY_SEGMENT_REGISTER = [this, &writer]() {
			if (memorySegmented()) {
				writer.put_mov(RDX, RDI);
				writer.put_mov(DH, ref(Location(CoreState::DATA_MEMORY) + CoreState::SEGMENT_REGISTER_ADDRESS));
				writer.put_movzx(RDI, DX);
			}
		};


		// Below functions push/pop all registers, that we use to store microarch registers/flags, and that are not preserved by functions
		// according to Linux x8664 calling convention.
		auto PUSH_MAPPED_REGISTERS = [this, &writer]() {
			writer.put_push(RSI);
			writer.put_push(R8);
			writer.put_push(R9);
			writer.put_push(R10);
			writer.put_push(R11);
		};

		auto POP_MAPPED_REGISTERS = [this, &writer]() {
			writer.put_pop(R11);
			writer.put_pop(R10);
			writer.put_pop(R9);
			writer.put_pop(R8);
			writer.put_pop(RSI);
		};

		auto JUMP_IF_SEGMENT_NON_ZERO = [this, &writer](Location target) {
			writer.put_mov(RAX, ref(Location(CoreState::DATA_MEMORY) + CoreState::SEGMENT_REGISTER_ADDRESS));
			writer.put_add(RAX, 0);
			writer.put_jnz(target);
		};

		// Function reads from a memory address specified by RDI, into RAX
		writer.label(MEMORY_READ_FUNCTION);
		writer.put_lea(RAX, Location(DATA_MEMORY));
		APPLY_SEGMENT_REGISTER();
		writer.put_mov(RAX, ref(RAX + RDI));
		writer.put_ret();

		// Function stores RSI at memory address specified by RDI
		writer.label(MEMORY_WRITE_FUNCTION);
		writer.put_lea(RAX, Location(DATA_MEMORY));
		APPLY_SEGMENT_REGISTER();
		writer.put_mov(ref(RAX + RDI), RSI);
		writer.put_ret();

		// Functions for reading and writing from segment register, which is mapped at a fixed memory address in
		// every segment
		writer.label(SEGMENT_REGISTER_WRITE_FUNCTION);
		writer.put_mov(ref(Location(DATA_MEMORY) + SEGMENT_REGISTER_ADDRESS), RSI);
		writer.put_ret();

		writer.label(SEGMENT_REGISTER_READ_FUNCTION);
		writer.put_mov(RAX, ref(Location(DATA_MEMORY) + SEGMENT_REGISTER_ADDRESS));
		writer.put_ret();

		// Functions below push registers to stack (because we are calling a C++ function) and call the function dispatching
		// peripheral operations.
		writer.label(PERIPHERAL_READ_FUNCTION);
		// All peripherals must be mapped at segment 0, so if segment is not 0 we do a normal memory operation.
		JUMP_IF_SEGMENT_NON_ZERO(MEMORY_READ_FUNCTION);
		PUSH_MAPPED_REGISTERS();
		writer.put_mov(RSI, RDI);
		writer.put_mov(RAX, (uint64_t) (&readPeripheral));
		writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
		writer.put_call(RAX);
		POP_MAPPED_REGISTERS();
		writer.put_ret();

		writer.label(PERIPHERAL_WRITE_FUNCTION);
		// All peripherals must be mapped at segment 0, so if segment is not 0 we do a normal memory operation.
		JUMP_IF_SEGMENT_NON_ZERO(MEMORY_WRITE_FUNCTION);
		PUSH_MAPPED_REGISTERS();
		writer.put_mov(RDX, RSI);
		writer.put_mov(RSI, RDI);
		writer.put_mov(RAX, (uint64_t) (&writePeripheral));
		writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
		writer.put_call(RAX);
		POP_MAPPED_REGISTERS();
		writer.put_ret();

		// Arrays with 256 jump instructions each, padded to 8 bytes, that dispatch calls to the above 6 functions.
		writer.label(MEMORY_READ_MAPPING);
		for (unsigned int i = 0; i < DATA_MEMORY_SEGMENT_SIZE; i++) {
			jitPadInstructions(writer, buffer, 8, [&]() {
				if (i == SEGMENT_REGISTER_ADDRESS) {
					writer.put_jmp(SEGMENT_REGISTER_READ_FUNCTION);
				} else if (peripherals.find(i) == peripherals.end()) {
					writer.put_jmp(MEMORY_READ_FUNCTION);
				} else {
					writer.put_jmp(PERIPHERAL_READ_FUNCTION);
				}
			});
		}
		writer.label(MEMORY_WRITE_MAPPING);
		for (unsigned int i = 0; i < DATA_MEMORY_SEGMENT_SIZE; i++) {
			jitPadInstructions(writer, buffer, 8, [&]() {
				if (i == SEGMENT_REGISTER_ADDRESS) {
					writer.put_jmp(SEGMENT_REGISTER_WRITE_FUNCTION);
				} else if (peripherals.find(i) == peripherals.end()) {
					writer.put_jmp(MEMORY_WRITE_FUNCTION);
				} else {
					writer.put_jmp(PERIPHERAL_WRITE_FUNCTION);
				}
			});
		}
	}
	writer.section(BufferSegment::R | BufferSegment::W);

	// Data memory
	writer.label(DATA_MEMORY);
	writer.put_space(DATA_MEMORY_SIZE);

	// Dedicated area to initialize/dump registers at the beginning/end of execution.
	writer.label(REGISTERS);
	writer.put_space(REGISTER_COUNT);

	writer.label(FLAGS);
	writer.put_dword();

	writer.label(EXECUTABLE_CORE_POINTER);
	writer.put_qword();

	// Memory storing the number of microarch instruction, that is to be run next time the JIT code is called.
	// Initially after buffer's creation it's 0, later it is updated every time the execution halts.
	writer.label(PROGRAM_COUNTER);
	writer.put_word(pc);

	return ExecutableCore(this, buffer, stopFunction);
}

std::string CoreState::reg(int regnum) const {
	const std::string bits[16] = {
		"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111",
	};

	return bits[regnum & 0xF] + bits[(regnum >> 4) & 0xF];
}

