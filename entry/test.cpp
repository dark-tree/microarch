
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
		"\n", "nz", ".", "jmp", "loop", "\n", "\n", "end", "\n", "\n", "nop", ";",
		"nop", ";", "nop", "\n", "\n"
	};

	for (int i = 0; i < tokens.size(); i++) {
		const Token& token = tokens[i];

		if (token.lexeme() != expected[i]) {
			std::string error = "Token #" + std::to_string(i) + " (" + std::string(token.lexeme()) + ") is not what was expected!";
			FAIL(error);
		}
	}

	MessageSink::disable();

};

TEST(tokenize_invalid_regset) {

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