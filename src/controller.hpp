#pragma once

#include <string>
#include <vector>
#include <cstdint>

std::vector<uint8_t> decodeHexString(const std::string& data);
std::vector<uint8_t> loadBinaryInput(const std::string& input, bool use_hex);

void assemble(const std::string& input, const std::string& output, bool use_hex);
void disassemble(const std::string& input, bool use_hex);