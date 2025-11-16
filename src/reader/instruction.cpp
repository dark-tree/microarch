#include "instruction.hpp"

/*
 * class MicroInst
 */

std::string MicroInst::prefix() const {
	if (condition == MicroWriter::Cond::F) return "f.";
	if (condition == MicroWriter::Cond::T) return ""; // if the nstruction is not conditional ommit the prefix
	if (condition == MicroWriter::Cond::NBE) return "nbe.";
	if (condition == MicroWriter::Cond::NC) return "nc.";
	if (condition == MicroWriter::Cond::C) return "c.";
	if (condition == MicroWriter::Cond::NE) return "ne.";
	if (condition == MicroWriter::Cond::E) return "e.";

	return std::bitset<4>(condition).to_string() + ".";
}

std::string MicroInst::regset(uint8_t regset) const {
	uint32_t mask = 1;
	std::string value = "$";

	for (uint32_t i = 0; i < 8; i++) {
		if (regset & mask) {
			value += std::to_string(i);
		}
		mask <<= 1;
	}

	return value;
}

bool MicroInst::checkFlags(const CoreState& state) const {
	uint8_t fdf = condition & state.getFlagMask();
	return (fdf & MicroWriter::CF_IGNORE) && (fdf & MicroWriter::ZF_IGNORE);
}

MicroInst::~MicroInst() {
	// do nothing
}

void MicroInst::label(Labler& labelr) {
	// do nothing
}

void MicroInst::jitComputeRegistrySet(BufferWriter& writer, Registry output, uint8_t registrySet) {
	bool firstMoved = false;
	for (unsigned int i = 0; i < CoreState::REGISTER_COUNT; i++) {
		if (registrySet % 2) {
			if (!firstMoved) {
				writer.put_mov(output, CoreState::REGISTRY_MAPPING[i]);
				firstMoved = true;
			} else {
				writer.put_or(output, CoreState::REGISTRY_MAPPING[i]);
			}
		}
		registrySet = registrySet >> 1;
	}
}

void MicroInst::jitWriteToRegistrySet(BufferWriter& writer, Location input, uint8_t registrySet) {
	for (unsigned int i = 0; i < CoreState::REGISTER_COUNT; i++) {
		if (registrySet % 2) {
			writer.put_mov(CoreState::REGISTRY_MAPPING[i], input);
		}
		registrySet = registrySet >> 1;
	}
}

void MicroInst::jitApplyInstructionOnRegistrySets(BufferWriter& writer, TwoArgumentJitInstruction instruction) const {
	jitComputeRegistrySet(writer, DL, a);
	jitComputeRegistrySet(writer, BL, b);
	(writer.*instruction)(DL, BL);
	jitWriteToRegistrySet(writer, DL, a);
}

Label MicroInst::jitInsertConditionalJumpIfNeeded(BufferWriter& writer) const {
	if (condition == MicroWriter::T) {
		return {};
	}
	auto label = Label::make_unique();
	if (condition == MicroWriter::F) {
		writer.put_jmp(label);
	} else {
		static std::unordered_map<MicroWriter::Cond, void (BufferWriter::*)(Location)> JUMP_CONDITION_MAPPING = {
				{MicroWriter::Cond::NBE, &BufferWriter::put_jbe},
				{MicroWriter::Cond::NC,  &BufferWriter::put_jc},
				{MicroWriter::Cond::C,   &BufferWriter::put_jnc},
				{MicroWriter::Cond::NE,  &BufferWriter::put_je},
				{MicroWriter::Cond::E,   &BufferWriter::put_jne},
		};
		writer.put_push(SI);
		writer.put_popf();
		auto func = JUMP_CONDITION_MAPPING.at(condition);
		(writer.*func)(label);
	}
	return label;
}

void MicroInst::jitConditionalExecute(BufferWriter& writer, std::function<void()>&& operation) {
	auto label = jitInsertConditionalJumpIfNeeded(writer);
	operation();
	if (!label.empty()) {
		writer.label(label);
	}
}
