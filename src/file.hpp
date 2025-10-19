#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace file {

	std::string read(const std::string& path);
	void write(const std::string& path, const std::vector<uint8_t>& content);
	void write(const std::string& path, const char* data, size_t size);

};