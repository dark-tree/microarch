#pragma once
#include <vector>

#include "lexer.hpp"
#include "token.hpp"

class Tokenizer {

	protected:

		static inline std::unordered_map<char, int> escapes {
			{'a', '\a'}, // alert/beep
			{'b', '\b'}, // backspace
			{'e', 0x1b}, // escape (non standard)
			{'f', '\f'}, // form feed
			{'n', '\n'}, // new line
			{'r', '\r'}, // carriage return
			{'t', '\t'}, // tab
			{'v', '\v'}, // vertical tab
			{'\\', '\\'},
			{'\"', '\"'},
			{'\'', '\''}
		};

		static void scanInlineComment(Lexer& lexer);
		static void scanMultilineComment(Lexer& lexer);
		static void scanIdentifierOrLabel(Lexer& lexer, Token::Stream& sink);
		static void scanCharEscape(Lexer& lexer);
		static void scanString(Lexer& lexer, Token::Stream& sink);
		static void scanDigits(Lexer& lexer, const CharPredicate& predicate);
		static void scanNumberTail(Lexer& lexer, Token::Stream& sink);
		static void scanInteger(Lexer& lexer, Token::Stream& sink);
		static void scanRegisterSet(Lexer& lexer, Token::Stream& sink);

	public:

		static std::vector<Token> tokenize(const SourceUnit* unit);

};
