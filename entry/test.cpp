
#include <assembler.hpp>
#include <source/error.hpp>
#include <source/tokenizer.hpp>

#include "vstl.hpp"

TEST(tokenize_mixed) {
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		/*
		 * Tokenize this!
		 * /* Nested?? UwU */
		 * unreal
		 */

		nop;
		cmp $12, $3

		using le // inline // begin

			add $1, $
			set $01234567, 0

			loop:
				cid 0

			nz.jmp loop

		end

		// inline statements
		nop; nop; nop

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	std::vector<std::string> expected = {
		"\n","\n","\n","\n", "nop", ";", "\n", "cmp", "$12", ",", "$3", "\n",
		"\n", "using", "le", "begin", "\n", "\n", "add", "$1", ",", "$", "\n",
		"set", "$01234567", ",", "0", "\n", "\n", "loop:", "\n", "cid", "0", "\n",
		"\n", "nz.jmp", "loop", "\n", "\n", "end", "\n", "\n", "nop", ";",
		"nop", ";", "nop", "\n", "\n"
	};

	for (int i = 0; i < tokens.size(); i++) {
		const Token& token = tokens[i];

		if (token.lexeme() != expected[i]) {
			std::string error = "Token #" + std::to_string(i) + " (" + std::string(token.lexeme()) + ") is not what was expected!";
			FAIL(error);
		}
	}

};

TEST(tokenize_invalid_regset) {

	MessageSink::disable();

	SourceUnit duplicate {"$11", "file"};
	SourceUnit ordering {"$21", "file"};
	SourceUnit sanity {"12", "file"};

	EXPECT_THROW(Message) {
		Tokenizer::tokenize(&duplicate);
	};

	EXPECT_THROW(Message) {
		Tokenizer::tokenize(&ordering);
	};

	Tokenizer::tokenize(&sanity);

};

TEST(assembler_basic) {

	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		set $1, 100
		mov $23, $1
		nz.mov $5, $34; nop

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	CHECK(bytes.size(), 12);

	CHECK(bytes[0], 0b0001'1111);
	CHECK(bytes[1], 0b0000'0010);
	CHECK(bytes[2], 100);

	CHECK(bytes[3], 0b1100'1111);
	CHECK(bytes[4], 0b0000'1100);
	CHECK(bytes[5], 0b0000'0010);

	CHECK(bytes[6], 0b1100'0111);
	CHECK(bytes[7], 0b0010'0000);
	CHECK(bytes[8], 0b0001'1000);

	CHECK(bytes[9] & 0xF0, 0); // ignore condition bits in NOPs
	CHECK(bytes[10], 0b0000'0000);
	CHECK(bytes[11], 0b0000'0000);

};