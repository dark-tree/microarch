#include "state.hpp"

#include <out/buffer/writer.hpp>

#include "instruction.hpp"
#include "writer.hpp"

/*
 * class MicroCore
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
	for (int i=0; i<count; i++) {
		if (ctr.flags.standby_mode) {
			break;
		}
		uint16_t current_instruction = pc;
		pc += 1;
		rom[current_instruction]->apply(*this);
	}
}

asmio::ExecutableBuffer CoreState::jit() {

	using namespace asmio;

	SegmentedBuffer buffer;
	BasicBufferWriter writer {buffer};

	writer.section(BufferSegment::R | BufferSegment::X);
	writer.label("code");

	for (auto& inst : rom) {
		inst->jit(buffer);
	}

	writer.section(BufferSegment::R | BufferSegment::W);
	writer.label("data");
	writer.put_space(256);

	return to_executable(buffer);
}
