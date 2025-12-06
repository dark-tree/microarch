
#include <args.hpp>
#include <assembler.hpp>
#include <file.hpp>
#include <controller.hpp>

int main(int argc, char* argv[]) {

	argx::builder builder {"Usage: microarch [OPTIONS...]\nToolchain for the Microarch architecture.\n"};
	builder.add_help();
	builder.add("hex", 'x').detail("Output simple hex encoded text.");
	builder.add("output", 'O').detail("Output file path.").fallback("a.bin").type(argx::string);
	builder.add("input", 'I').detail("Path to file to assemble.").type(argx::string);
	builder.add("assemble", 'a').detail("Assemble text input into machine code.").type(argx::flag).conflicts("disassemble");
	builder.add("disassemble", 'd').detail("Disassemble binary input back into text.").type(argx::flag).conflicts("output").conflicts("assemble").conflicts("run");
	builder.add("run", 'r').detail("Emulate a Microarch program.").type(argx::flag).conflicts("output").conflicts("disassemble");

	argx::parsed parsed = builder.parse(argc, argv);

	bool hex = parsed.get("hex");
	std::string output = parsed.get("output").value() + (hex ? ".hex" : "");
	std::string input = parsed.get("input").value();

	bool no_output = !parsed.get("output") && parsed.get("run");

	if (parsed.get("assemble") && !no_output) {
		assemble(input, output, hex);
		return 0;
	}

	if (parsed.get("disassemble")) {
		disassemble(input, hex);
		return 0;
	}

	if (parsed.get("run")) {
		std::vector<uint8_t> bytes;

		if (parsed.get("assemble")) {
			auto opt = assemble(input);

			if (!opt) {
				return 1;
			}

			bytes = *opt;
		} else {
			bytes = loadBinaryInput(input, hex);
		}

		run(bytes);
		return 0;
	}

	printf("No task selected, to assemble use '-a',\nsee --help for more information.\n");
	return 1;

}
