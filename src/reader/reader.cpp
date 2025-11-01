#include "reader.hpp"

CoreState MicroReader::toProgram(const std::vector<uint8_t>& bytes) {

	if (bytes.size() % 3 != 0) {
		throw std::runtime_error {"Program buffer is not 3-aligned!"};
	}

	CoreState state;

	for (size_t i = 0; i < bytes.size(); i += 3) {

		const uint8_t head = bytes[i];
		const uint8_t a = bytes[i + 1];
		const uint8_t b = bytes[i + 2];

		const auto cond = static_cast<MicroWriter::Cond>(head & 0b1111);
		const uint8_t opcode = (head & 0b1111'0000) >> 4;

		switch (opcode) {
			case MicroWriter::OP_NOP: state.rom.push_back(std::make_unique<InstNop>(i)); break;
			case MicroWriter::OP_SET: state.rom.push_back(std::make_unique<InstSet>(i, cond, a, b)); break;
			case MicroWriter::OP_LDM: state.rom.push_back(std::make_unique<InstLdm>(i, cond, a, b)); break;
			case MicroWriter::OP_STM: state.rom.push_back(std::make_unique<InstStm>(i, cond, a, b)); break;
			case MicroWriter::OP_JPR: state.rom.push_back(std::make_unique<InstJpr>(i, cond, a, b)); break;
			case MicroWriter::OP_JPI: state.rom.push_back(std::make_unique<InstJpi>(i, cond, a, b)); break;
			case MicroWriter::OP_CID: state.rom.push_back(std::make_unique<InstCid>(i, cond, a, b)); break;
			case MicroWriter::OP_CTR: state.rom.push_back(std::make_unique<InstCtr>(i, cond, a, b)); break;
			case MicroWriter::OP_NAD: state.rom.push_back(std::make_unique<InstNad>(i, cond, a, b)); break;
			case MicroWriter::OP_AND: state.rom.push_back(std::make_unique<InstAnd>(i, cond, a, b)); break;
			case MicroWriter::OP_XOR: state.rom.push_back(std::make_unique<InstXor>(i, cond, a, b)); break;
			case MicroWriter::OP_SHR: state.rom.push_back(std::make_unique<InstShr>(i, cond, a, b)); break;
			case MicroWriter::OP_MOV: state.rom.push_back(std::make_unique<InstMov>(i, cond, a, b)); break;
			case MicroWriter::OP_EXT: throw std::runtime_error {"Extension point used in standard program!"};
			case MicroWriter::OP_ADD: state.rom.push_back(std::make_unique<InstAdd>(i, cond, a, b)); break;
			case MicroWriter::OP_CMP: state.rom.push_back(std::make_unique<InstCmp>(i, cond, a, b)); break;
			default: throw std::runtime_error {"Unknown opcode!"};
		}

	}

	return state;

}
