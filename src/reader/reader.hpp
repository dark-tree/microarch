#pragma once
#include <memory>
#include <vector>

#include "instruction.hpp"

struct MicroReader {

	CoreState toProgram(const std::vector<uint8_t>& bytes);

};
