
#include <args.hpp>
#include <assembler.hpp>
#include <file.hpp>
#include <source/tokenizer.hpp>

void assemble(const std::string& input, const std::string& output, bool use_hex) {

	if (use_hex) {
		printf("Hexadecimal output mode is currently unsupported!\n");
		exit(1);
	}

	std::string source = file::read(input);

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		for (auto& node : message.nodes()) {
			printf("\n");
			int line = node.section().front().line;

			std::string severity;

			switch (node.severity()) {
				case Message::VERBOSE: severity += "\e[1;37mVerbose\e[0m"; break;
				case Message::INFO: severity += "\e[1;36mInfo\e[0m"; break;
				case Message::WARNING: severity += "\e[1;33mWarning\e[0m"; break;
				case Message::ERROR: severity += "\e[1;31mError\e[0m"; break;
				default: severity += "\e[1;31mUnknown\e[0m"; break;
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

	file::write(output, bytes);

}

int main(int argc, char* argv[]) {

	argx::builder builder {"Usage: microarch [OPTIONS...]\nToolchain for the microarch architecture.\n"};
	builder.add_help();
	builder.add("hex", 'x').detail("Output simple hex encoded text.");
	builder.add("output", 'O').detail("Output file path.").fallback("a.bin").type(argx::string);
	builder.add("input", 'I').detail("Path to file to assemble.").type(argx::string);

	argx::parsed parsed = builder.parse(argc, argv);

	bool hex = parsed.get("hex");
	std::string output = parsed.get("output").value() + (hex ? ".hex" : "");
	std::string input = parsed.get("input").value();

	assemble(input, output, hex);

}
