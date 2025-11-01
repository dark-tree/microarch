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
	return condition & state.getFlagMask();
}

MicroInst::~MicroInst() {
	// do nothing
}

void MicroInst::label(Labelr& labelr) {
	// do nothing
}
