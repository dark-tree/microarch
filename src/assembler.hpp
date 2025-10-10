#pragma once
#include <vector>
#include <writer.hpp>
#include <source/parser.hpp>
#include <source/token.hpp>
#include <source/util.hpp>

class Assembler {

	private:

		static inline std::unordered_map<std::string_view, MicroWriter::Cond> conditions = {
			{"f", MicroWriter::F},
			{"t", MicroWriter::T},
			{"nba", MicroWriter::NBA},
			{"a", MicroWriter::A},
			{"nc", MicroWriter::NC},
			{"nb", MicroWriter::NB},
			{"ae", MicroWriter::AE},
			{"c", MicroWriter::C},
			{"b", MicroWriter::B},
			{"nae", MicroWriter::NAE},
			{"ne", MicroWriter::NE},
			{"nz", MicroWriter::NZ},
			{"e", MicroWriter::E},
			{"z", MicroWriter::Z}
		};

		struct Label {
			uint32_t id;
			SourceSpan location;
		};

		uint32_t next_label_id = 0;

		uint8_t parseRegisterSet(Parser& parser);
		uint8_t parseImmediate(Parser& parser);
		void emitOperation(Parser parser, std::string_view mnemonic, MicroWriter& writer);
		void parseStatement(Parser parser, MicroWriter& writer);
		void parseRoot(Parser parser, MicroWriter& writer);

	public:

		/**
		 * Give a stream of tokens produce the corresponding assembly bytes
		 */
		std::vector<uint8_t> assemble(const std::vector<Token>& tokens);

};
