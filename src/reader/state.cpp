#include "state.hpp"

#include <out/buffer/writer.hpp>

#include "instruction.hpp"
#include "writer.hpp"
#include "asm/x86/writer.hpp"
#include "asm/x86/argument/registry.hpp"




static void dumpJITData(ExecutableCore* executableCore) {
	asmio::ExecutableBuffer& code = executableCore->code;
	CoreState* core = executableCore->core;
	uint8_t* dataMemory = code.address(CoreState::DATA_MEMORY);
	uint8_t* registerDumpZone = code.address(CoreState::REGISTERS);
	uint16_t* flagsDumpZone = (uint16_t*)code.address(CoreState::FLAGS);
	uint16_t* pc = (uint16_t*)code.address(CoreState::PROGRAM_COUNTER);
	for (unsigned int i=0; i<CoreState::DATA_MEMORY_SIZE; i++) {
		core->ram[i] = dataMemory[i];
	}
	for (unsigned int i=0; i<CoreState::REGISTER_COUNT; i++) {
		core->regs[i] = registerDumpZone[i];
	}
	core->pc = *pc;
	core->cf = (bool)((*flagsDumpZone) & 0x0001);
	core->zf = (bool)((*flagsDumpZone) & 0x0040);
}

static void initJITData(ExecutableCore* executableCore) {
	asmio::ExecutableBuffer& code = executableCore->code;
	CoreState* core = executableCore->core;
	uint8_t* dataMemory = code.address(CoreState::DATA_MEMORY);
	uint8_t* registerDumpZone = code.address(CoreState::REGISTERS);
	uint16_t* flagsDumpZone = (uint16_t*)code.address(CoreState::FLAGS);
	uint16_t* pc = (uint16_t*)code.address(CoreState::PROGRAM_COUNTER);
	for (unsigned int i=0; i<CoreState::DATA_MEMORY_SIZE; i++) {
		dataMemory[i] = core->ram[i];
	}
	for (unsigned int i=0; i<CoreState::REGISTER_COUNT; i++) {
		registerDumpZone[i] = core->regs[i];
	}
	*pc = core->pc;
	*flagsDumpZone = core->zf ? 0x0040 : 0x0000;
	*flagsDumpZone = core->cf ? (0x0001 | *flagsDumpZone) : *flagsDumpZone;
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

	Labelr labelr;

	for (auto& inst : rom) {
		inst->label(labelr);
	}

	for (auto& inst : rom) {

		if (labelr.has(inst->address)) {
			str += "\nl_" + std::to_string(inst->address) + ":\n";
		}

		str += "\t" + inst->string();
		str += "\n";
	}

	return str;
}

void CoreState::run(size_t count) {
	ctr.flags.standby_mode = 0;
	for (unsigned int i=0; i<count; i++) {
		if (ctr.flags.standby_mode) {
			break;
		}
		uint16_t current_instruction = pc;
		pc += 1;
		rom[current_instruction]->apply(*this);
	}
}

ExecutableCore CoreState::jit(std::function<void()> stopFunction) {

	using namespace asmio;
	using namespace asmio::x86;



	SegmentedBuffer buffer;
	x86::BufferWriter writer {buffer};

	writer.section(BufferSegment::R | BufferSegment::X);
	writer.label(ExecutableCore::CODE_START);

	using namespace asmio::x86;

	// Saving pointer for accessing core state and labels
	writer.put_mov(RAX, ref(RAX));
	writer.put_mov(ref(EXECUTABLE_CORE_POINTER), RAX);

	// Initializing data memory, flags and registers with data from CoreState
	writer.put_mov(RAX, (uint64_t)(&initJITData));
	writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
	writer.put_call(RAX);
	writer.put_mov( SI, ref(FLAGS));	// Flags
	writer.put_lea(RAX, REGISTERS);
	for (unsigned int i=0;i<REGISTER_COUNT;i++) {
		// We get registers to a dedicated memory area, where tnhey were prepared by initJITData function.
		writer.put_mov(REGISTRY_MAPPING[i], ref(RAX+i));
	}

	// Jumping to the correct microarch instruction
	writer.put_mov(BX, ref(PROGRAM_COUNTER));
	writer.put_movzx(RBX, BX);
	writer.put_lea(RAX, INSTRUCTION_OFFSETS);
	writer.put_mov(RDX, ref<QWORD>( RAX + RBX*8));
	writer.put_lea(RAX, PROGRAM_MEMORY);
	writer.put_add(RAX, RDX);
	writer.put_jmp(RAX);

	writer.label(PROGRAM_MEMORY);
	auto programStartMarker = buffer.current();
	std::vector<uint64_t>instructionOffsets;
	// writer.put_mov(ref<BYTE>(Location(DATA_MEMORY)+13), 93);
	// writer.put_mov(R8L, 19);
	for (auto& inst : rom) {
		auto instructionMarker = buffer.current();
		instructionOffsets.push_back(instructionMarker.offset - programStartMarker.offset);
		inst->jit(writer);
	}



	// Saving data memory, flags and registers back to core state
	writer.label(CLEANUP_CODE);
	writer.put_mov(ref(FLAGS), SI);	// Flags
	writer.put_lea(RAX, REGISTERS);
	for (unsigned int i=0;i<REGISTER_COUNT;i++) {
		// We dump registers to a dedicated memory area, to be extracted by dumpJITData function.
		writer.put_mov(ref(RAX+i), REGISTRY_MAPPING[i]);
	}
	writer.put_mov(RAX, (uint64_t)(&dumpJITData));
	writer.put_mov(RDI, ref(EXECUTABLE_CORE_POINTER));
	writer.put_call(RAX);

	// RET instruction demanded by ASMIOV library spec to return from JIT segment.
	writer.put_ret();


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

	writer.label(INSTRUCTION_OFFSETS);
	writer.put_data(instructionOffsets.size() * sizeof(uint64_t), instructionOffsets.data());

	return ExecutableCore(this, buffer, stopFunction);
}
