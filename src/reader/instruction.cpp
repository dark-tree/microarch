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

void MicroInst::label(Labeler& labeler) {
	// do nothing
}

void MicroInst::jitReadRegistrySet(BufferWriter& writer, Registry output, uint8_t set) {
	bool firstMoved = false;

	for (auto reg : CoreState::REGISTRY_MAPPING) {
		if (set % 2) {
			if (!firstMoved) {
				writer.put_mov(output, reg);
				firstMoved = true;
			} else {
				writer.put_or(output, reg);
			}
		}

		set = set >> 1;
	}
}

void MicroInst::jitWriteRegistrySet(BufferWriter& writer, const Location& input, uint8_t set) {
	for (auto reg : CoreState::REGISTRY_MAPPING) {
		if (set % 2) {
			writer.put_mov(reg, input);
		}

		set = set >> 1;
	}
}

void MicroInst::jitTwoArgInst(BufferWriter& writer, TwoArgumentJitInstruction instruction) const {
	jitReadRegistrySet(writer, DL, a);
	jitReadRegistrySet(writer, BL, b);
	(writer.*instruction)(DL, BL);
	jitWriteRegistrySet(writer, DL, a);
}

Label MicroInst::jitWriteConditional(BufferWriter& writer) const {
	if (condition == MicroWriter::T) {
		return {};
	}

	auto label = Label::make_unique();
	if (condition == MicroWriter::F) {
		writer.put_jmp(label);
		return label;
	}

	writer.put_push(SI);
	writer.put_popf();

	if (condition == MicroWriter::Cond::NBE) {
		writer.put_jbe(label);
		return label;
	}

	if (condition == MicroWriter::Cond::NC) {
		writer.put_jc(label);
		return label;
	}

	if (condition == MicroWriter::Cond::C) {
		writer.put_jnc(label);
		return label;
	}

	if (condition == MicroWriter::Cond::NE) {
		writer.put_je(label);
		return label;
	}

	if (condition == MicroWriter::Cond::E) {
		writer.put_jne(label);
		return label;
	}

	throw std::runtime_error {"Unknown condition code!"};
}

void MicroInst::jitConditional(BufferWriter& writer, const std::function<void()>& then) {
	auto label = jitWriteConditional(writer);

	then();

	// check if no conditional jump was needed
	if (!label.empty()) {
		writer.label(label);
	}
}
