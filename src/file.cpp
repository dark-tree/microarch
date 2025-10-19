#include "file.hpp"

#include <fstream>

std::string file::read(const std::string& path) {
	std::ifstream ifs (path);

	if (ifs.fail()) {
		printf("Unable to open input file '%s'. Aborting...\n", path.c_str());
		exit(2);
	}

	std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));
	ifs.close();
	return content;
}

void file::write(const std::string& path, const std::vector<uint8_t>& content) {
	write(path, reinterpret_cast<const char*>(content.data()), content.size());
}

void file::write(const std::string& path, const char* data, size_t size) {
	std::ofstream ofs (path);

	if (ofs.fail()) {
		printf("Unable to open output file '%s'. Aborting...\n", path.c_str());
		exit(2);
	}

	ofs.write(data, size);
	ofs.close();
}