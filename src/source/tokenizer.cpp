#include "tokenizer.hpp"
#include "util.hpp"
#include "error.hpp"

/*
 * class Tokenizer
 */

void Tokenizer::scanInlineComment(Lexer& lexer) {
	while (lexer) {

		if (lexer.match('\n')) {
			return;
		}

		// inline termination
		if (lexer.accept("//")) {
			return;
		}

		// consume the rest
		lexer.next();

	}
}

void Tokenizer::scanMultilineComment(Lexer& lexer) {
	int depth = 1;

	while (lexer) {

		if (depth <= 0) {
			return;
		}

		if (lexer.accept("/*")) {
			depth ++;
			continue;
		}

		if (lexer.accept("*/")) {
			depth --;
			continue;
		}

		lexer.next();

	}
}

void Tokenizer::scanIdentifierOrLabel(Lexer& lexer, Token::Stream& sink) {

	lexer.greedy(CharPredicates::alphanumeric);

	if (lexer.match(':')) {
		sink.push_back(lexer.endToken(Token::LABEL));
		return;
	}

	if (lexer.match('.')) {
		lexer.greedy(CharPredicates::alphanumeric);
	}

	sink.push_back(lexer.endToken(Token::IDENTIFIER));
}

void Tokenizer::scanCharEscape(Lexer& lexer) {

	if (lexer.match(CharPredicates::escape)) {
		return;
	}

	if (lexer.match('x')) {
		lexer.match(CharPredicates::hexadecimal);
		lexer.match(CharPredicates::hexadecimal);
		return;
	}

	SourceSpan span = lexer.nextSpan();

	// do this first to throw if the lexer is empty
	char c = lexer.next();
	std::string message;

	if (StringUtil::isPrintable(c)) {
		message = "Unknown escape code \\";
		message.push_back(c);
	} else {
		message = "Unexpected " + StringUtil::ofChar(c) + " in escape code";
	}

	Message::of().source(span).warn(message).report();

}

void Tokenizer::scanString(Lexer& lexer, Token::Stream& sink) {

	while (lexer) {

		if (lexer.match('\n')) {
			SourceSpan span = lexer.prevSpan();
			Message::of().source(span).error("Unexpected new line, expected end of string").raise();
		}

		if (lexer.match('\\')) {
			try {
				scanCharEscape(lexer);
			} catch ([[maybe_unused]] const Message& ignore) {} // message is already reported
		}

		if (lexer.match('"')) {
			break;
		}

		lexer.next();

	}

	sink.push_back(lexer.endToken(Token::STRING));

}

void Tokenizer::scanDigits(Lexer& lexer, const CharPredicate& predicate) {
	while (lexer.greedy(predicate)) {
		if (!lexer.match('\'')) break;
	}
}

void Tokenizer::scanNumberTail(Lexer& lexer, Token::Stream& sink) {

	if (lexer.match(CharPredicates::alphanumeric)) {
		SourceSpan span = lexer.prevSpan();
		Message::of().source(span).error("Unknown number suffix character").raise();
	}

	sink.push_back(lexer.endToken(Token::INTEGER));
}

void Tokenizer::scanInteger(Lexer& lexer, Token::Stream& sink) {

	lexer.rewind();

	if (lexer.match('0')) {

		if (lexer.match('x')) {
			scanDigits(lexer, CharPredicates::hexadecimal);
			scanNumberTail(lexer, sink);
			return;
		}

		if (lexer.match('o')) {
			scanDigits(lexer, CharPredicates::octal);
			scanNumberTail(lexer, sink);
			return;
		}

		if (lexer.match('b')) {
			scanDigits(lexer, CharPredicates::binary);
			scanNumberTail(lexer, sink);
			return;
		}
	}

	// to not have to deal with the fact one digit is already consumed
	// just start over, this simplified the logic for . and ' a lot
	lexer.rewind();

	while (true) {

		if (!lexer.greedy(CharPredicates::decimal)) {
			break;
		}

		if (lexer.match('\'')) {
			// expect *at least* one digit after separator
			lexer.expect(CharPredicates::decimal);
		}

	}

	scanNumberTail(lexer, sink);
}

void Tokenizer::scanRegisterSet(Lexer& lexer, Token::Stream& sink) {

	int previous = '0' - 1;

	// this allows 0 digits after $, this is intended and defines an empty regset
	while (true) {

		char chr = lexer.peek();

		if (chr >= '0' && chr <= '9') {
			if (chr <= previous) {
				SourceSpan span = lexer.prevSpan();

				Message::of()
					.source(span)
					.error("Register $" + StringUtil::toString(chr) + " specified out-of-order after register $" + StringUtil::toString(chr) + " in register set, registers must be specified in ascending order")
					.raise();
			}

			previous = static_cast<unsigned char>(chr);

			lexer.advance();
			continue;
		}

		// identifier may not immediately follow a register set
		if (lexer.match(CharPredicates::alphanumeric)) {
			SourceSpan span = lexer.prevSpan();

			Message::of()
				.source(span)
				.error("Unexpected " + span.quote() + " in register set")
				.raise();
		}

		break;
	}

	sink.push_back(lexer.endToken(Token::REGSET));
}

std::vector<Token> Tokenizer::tokenize(const SourceUnit* unit) {
	Lexer lexer {unit};
	Token::Stream sink;

	while (lexer) {

		lexer.beginToken();

		if (lexer.match('\n') || lexer.match(';')) {
			sink.push_back(lexer.endToken(Token::BREAK));
			continue;
		}

		if (lexer.match(CharPredicates::whitespace)) {
			continue;
		}

		if (lexer.accept("//")) {
			scanInlineComment(lexer);
			continue;
		}

		if (lexer.accept("/*")) {
			scanMultilineComment(lexer);
			continue;
		}

		if (lexer.match(CharPredicates::alphabetic)) {
			scanIdentifierOrLabel(lexer, sink);
			continue;
		}

		if (lexer.match('"')) {
			scanString(lexer, sink);
			continue;
		}

		if (lexer.match(CharPredicates::decimal)) {
			scanInteger(lexer, sink);
			continue;
		}

		if (lexer.match('$')) {
			scanRegisterSet(lexer, sink);
			continue;
		}

		if (lexer.match(',')) {
			sink.push_back(lexer.endToken(Token::SYMBOL));
			continue;
		}

		lexer.unreachable();

	}

	return sink;
}