
#include <args.hpp>
#include <assembler.hpp>
#include <file.hpp>
#include <controller.hpp>

int main(int argc, char* argv[]) {

	argx::builder builder {"Usage: microarch [OPTIONS...]\nToolchain for the microarch architecture.\n"};
	builder.add_help();
	builder.add("hex", 'x').detail("Output simple hex encoded text.");
	builder.add("output", 'O').detail("Output file path.").fallback("a.bin").type(argx::string);
	builder.add("input", 'I').detail("Path to file to assemble.").type(argx::string);
	builder.add("assemble", 'a').detail("Assemble text input into machine code.").type(argx::flag).conflicts("disassemble");
	builder.add("disassemble", 'd').detail("Disassemble binary input back into text.").type(argx::flag).conflicts("output").conflicts("assemble");

	argx::parsed parsed = builder.parse(argc, argv);

	bool hex = parsed.get("hex");
	std::string output = parsed.get("output").value() + (hex ? ".hex" : "");
	std::string input = parsed.get("input").value();

	if (parsed.get("assemble")) {
		assemble(input, output, hex);
		return 0;
	}

	if (parsed.get("disassemble")) {
		disassemble(input, hex);
		return 0;
	}

	printf("No task selected, to assemble use '-a',\nsee --help for more information.\n");
	return 1;

}
