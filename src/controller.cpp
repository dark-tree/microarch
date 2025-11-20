#include "controller.hpp"

#include <assembler.hpp>
#include <chrono>
#include <cstring>
#include <file.hpp>
#include <reader/reader.hpp>
#include <reader/state.hpp>
#include <source/tokenizer.hpp>

std::vector<uint8_t> decodeHexString(const std::string& data) {
	std::vector<uint8_t> bytes;

	try {
		for (size_t i = 0; i + 1 < data.size(); i += 1) {
			if (std::isxdigit(data.at(i)) && std::isxdigit(data.at(i+1))) {
				const auto view = data.substr(i, 2);
				const int value = std::stoi(view, nullptr, 16);

				bytes.push_back(value);
				i += 1;
			}
		}
	} catch (const std::exception& e) {
		throw std::runtime_error("Invalid input data format!\n");
	}

	return bytes;
}

std::vector<uint8_t> loadBinaryInput(const std::string& input, bool use_hex) {
	std::string data = file::read(input);

	if (!use_hex) {
		std::vector<uint8_t> bytes;
		bytes.resize(data.size());
		std::memcpy(bytes.data(), data.data(), data.size());
		return bytes;
	}

	return decodeHexString(data);
}

void assemble(const std::string& input, const std::string& output, bool use_hex) {

	std::string source = file::read(input);

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		for (auto& node : message.nodes()) {
			printf("\n");
			int line = node.section().front().line;

			std::string severity;

			switch (node.severity()) {
				case Message::VERBOSE: severity += "\033[1;37mVerbose\033[0m"; break;
				case Message::INFO: severity += "\033[1;36mInfo\033[0m"; break;
				case Message::WARNING: severity += "\033[1;33mWarning\033[0m"; break;
				case Message::ERROR: severity += "\033[1;31mError\033[0m"; break;
				default: severity += "\033[1;31mUnknown\033[0m"; break;
			}

			printf("%d | %s", line, node.section().wrap().str().c_str());
			printf("%s: %s %s\n", severity.c_str(), node.where().c_str(), node.message().c_str());
		}
	});

	SourceUnit unit {source, input};
	auto tokens = Tokenizer::tokenize(&unit);

	if (MessageSink::error()) {
		printf("\nCompilation aborted due to errors; no output produced.\n");
		return;
	}

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	if (MessageSink::error()) {
		printf("\nCompilation aborted due to errors; no output produced.\n");
		return;
	}

	if (use_hex) {
		char const hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		std::string result;

		for (size_t i = 0, c = 0; i < bytes.size(); i ++) {
			char const byte = bytes[i];

			result += hex[(byte & 0xF0) >> 4];
			result += hex[(byte & 0x0F) >> 0];

			if (++ c >= 3) {
				result += '\n';
				c = 0;
			}
		}

		file::write(output, result.data(), result.size());
		return;
	}

	file::write(output, bytes);

}

void disassemble(const std::string& input, bool use_hex) {
	std::vector<uint8_t> bytes = loadBinaryInput(input, use_hex);

	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	std::string back = state.disassemble();
	printf("%s\n", back.c_str());
}

void run(const std::string& input, bool use_hex) {
	std::vector<uint8_t> bytes = loadBinaryInput(input, use_hex);

	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	auto time = [] (const std::function<void()>& benchmark) {
		auto start = std::chrono::high_resolution_clock::now();

		benchmark();

		auto end = std::chrono::high_resolution_clock::now();
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

		printf("Done in %ldns\n", ns);
	};

	while (true) {

		std::string command;
		std::cin >> command;

		if (command == "help" || command == "h" || command == "?") {
			printf("Valid commands:\n");
			printf(" help, h - Show this help page\n");
			printf(" run,  r - Run program using interpreter\n");
			printf(" jit,  j - Run program using JIT\n");
			printf(" step, s - Single step forward\n");
			printf(" regs, r - Print registers\n");
			printf(" list, l - Print program\n");
			printf(" quit, q - Quit microarch emulator\n");
			printf(" init, i - Reset the simulation\n");
			continue;
		}

		if (command == "run" || command == "r") {
			time([&] {
				state.run();
			});

			continue;
		}

		if (command == "jit" || command == "j") {
			auto executable = state.jit();

			time([&] {
				executable();
			});

			continue;
		}

		if (command == "step" || command == "s") {
			state.run(1);
			continue;
		}

		if (command == "regs" || command == "r") {
			printf("R0=%s  R4=%s\n", state.reg(0).c_str(), state.reg(4).c_str());
			printf("R1=%s  R5=%s\n", state.reg(1).c_str(), state.reg(5).c_str());
			printf("R2=%s  R6=%s\n", state.reg(2).c_str(), state.reg(6).c_str());
			printf("R3=%s  R7=%s\n", state.reg(3).c_str(), state.reg(7).c_str());
			printf("PC=0x%04x CF=%d ZF=%d\n", state.pc, state.cf, state.zf);
			continue;
		}

		if (command == "init" || command == "i"){
			state.reset();
			continue;
		}

		if (command == "list" || command == "l") {
			std::string back = state.disassemble();
			printf("%s\n", back.c_str());
			continue;
		}

		if (command == "quit" || command == "q") {
			break;
		}

	}

	printf("Goodbye!\n");
}