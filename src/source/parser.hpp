#pragma once

#include <functional>
#include <macro.hpp>

#include "error.hpp"
#include "token.hpp"

class TokenPredicate {

	private:

		using Predicate = std::function<bool(const Token&)>;
		using Descriptor = std::function<std::string()>;

		Predicate predicate;
		Descriptor descriptor;

	public:

		TokenPredicate(const char* lexeme)
			: TokenPredicate([lexeme] (const auto& token) { return token.lexeme() == lexeme; }, [lexeme] { return "'" + std::string(lexeme) + "'"; }) {
		}

		TokenPredicate(Token::Type type)
			: TokenPredicate([type] (const auto& token) { return token.type() == type; }, [type] { return Token::typeToString(type); }) {
		}

		TokenPredicate(const Predicate& predicate, const char* description)
			: predicate(predicate), descriptor([description] { return description; }) {
		}

		TokenPredicate(const Predicate& predicate, const Descriptor& descriptor)
			: predicate(predicate), descriptor(descriptor) {
		}

		bool test(const Token& token) const {
			return predicate(token);
		}

		/**
		 * Get a human-readable description of this predicate,
		 * this method is slow and should be used only for debugging and error reporting.
		 */
		std::string str() const {
			return descriptor();
		}

};

class Parser {

	private:

		const std::vector<Token>* tokens;
		const char* name;

		uint32_t begin;
		uint32_t end;
		uint32_t offset;

		void assertNonEmpty(NULLABLE const TokenPredicate* predicate) const {
			if (empty()) {
				std::string message = "Unexpected end of ";
				message += name;

				if (predicate) {
					message += ", expected " + predicate->str();
				}

				Message::of().source(prevSpan()).error(message).raise();
			}
		}

	public:

		Parser(const std::vector<Token>* tokens)
			: Parser(tokens, 0, tokens->size(), 0, "input") {
		}

		Parser(const std::vector<Token>* tokens, size_t begin, size_t end, size_t offset, const char* name)
			: tokens(tokens), name(name), begin(begin), end(end), offset(offset) {
		}

		/**
		 * Get the span of the previous character (the one
		 * previously returned by the lexer)
		 */
		SourceSpan prevSpan() const {
			if (offset > begin) { // ok the tokens falls within the valid stream range
				return tokens->at(offset - 1).source();
			}

			if (offset > 0) { // was the stream empty? looking before stream start
				return tokens->at(offset - 1).source();
			}

			if (!tokens->empty()) { // was the input empty? This will most likely be incorrect
				return tokens->begin()->source();
			}

			// empty span, there is no input
			return {};
		}

		/**
		 * Returns the the next token in the stream,
		 * or throws a Message if there is no next token.
		 */
		const Token& peek() const {
			assertNonEmpty(nullptr);
			return tokens->at(offset);
		}

		/**
		 * Move to the next token in the stream, this will do
		 * nothing once we reach the end of input, silently ignoring all calls.
		 */
		void advance() {

			// when we reach the end we will point at the null-byte
			if (!empty()) {
				offset ++;
			}
		}

		/**
		 * Return the next token in the stream and consume it,
		 * moving to the next one. This will always return the same value as peek().
		 */
		const Token& next() {
			const Token& token = peek();
			advance();
			return token;
		}

		/**
		 * Asserts the given predicate will match the next token
		 * if that is not the cases it creates and throws an error message.
		 */
		const Token& expect(const TokenPredicate& predicate) {
			assertNonEmpty(&predicate);
			const Token& token = peek();

			if (predicate.test(token)) {
				advance();
				return token;
			}

			Message::of().source(token.source()).error("Unexpected token '" + token.source().str() + "', expected " + predicate.str()).raise();
		}

		/**
		 * Checks if the given predicate matches the next token,
		 * if the stream is empty 'false' is returned instead of throwing.
		 */
		bool match(const TokenPredicate& predicate) {
			if (empty()) {
				return false;
			}

			const Token& token = peek();
			if (predicate.test(token)) {
				advance();
				return true;
			}

			return false;
		}

		/**
		 * Checks if the given predicate matches the next token,
		 * returns either the token pointer if it matches or nullptr otherwise.
		 */
		const Token* accept(const TokenPredicate& predicate) {
			const Token& token = peek();

			if (match(predicate)) {
				return &token;
			}

			return nullptr;
		}

		/**
		 * Check if the stream is empty (no more tokens) or not,
		 * some methods (like. next(), peek()) throw when invoked on empty streams.
		 */
		bool empty() const {
			return offset >= end;
		}

		/**
		 * Implicit bool conversion,
		 * returns the inverse of empty().
		 */
		operator bool() const {
			return !empty();
		}

		/**
		 * Returns a new sub-parser that contains the tokens enclosed in the defiend block,
		 * this method expects the first token of the method (the initial opening) to be alredy
		 * consumed. The block is defined using a pair of symbols, one opening and one closing,
		 * the block is finished when the two tokens balance themselves.
		 *
		 * @param begin Opening token, the frst one should already be consumed
		 * @param end Closing token
		 * @param name Name of the created sub-parser to use in error messages
		 */
		Parser block(TokenPredicate begin, TokenPredicate end, const char* name) {
			int depth = 1;
			size_t start = offset;

			while (!empty() && depth > 0) {

				if (match(begin)) {
					depth ++;
					continue;
				}

				if (match(end)) {
					depth --;
					continue;
				}

				advance();
			}

			// doesn't include the first opening nor the last closing
			return {tokens, start, offset - 1, start, name};
		}

		/**
		 * Returns a new sub-parser that contains the tokens from the
		 * current position to the specified terminal token. The terminal token is
		 * included in the sub-parser, all the tokens (including the terminal) are
		 * consumed in this stream.
		 *
		 * @param terminal The predicate specifying the last token to match
		 * @param name Name of the created sub-parser to use in error messages
		 */
		Parser until(TokenPredicate terminal, const char* name) {
			size_t start = offset;

			while (!empty()) {

				if (match(terminal)) {
					break;
				}

				advance();
			}

			// doesn't include the specified terminal token
			return {tokens, start, offset - 1, start, name};
		}

		/**
		 * Mark a spot in the parser that should not be reached if the code was valid,
		 * will generate a "unexpected token" error for the NEXT character in the stream, the char is consumed.
		 */
		[[noreturn]] void unreachable(const char* expected = nullptr) {
			assertNonEmpty(nullptr);

			const Token& token = next();
			std::string suffix = "";

			if (expected != nullptr) {
				suffix = ", expected ";
				suffix += expected;
			}

			Message::of().source(token.source()).error("Unexpected '" + token.source().str() + "'" + suffix).raise();
		}

		/**
		 * Make sure that there are no more tokens in the current line,
		 * otherwise will create and raise an error Message
		 */
		void expectLineBreak() const {

			// we can't know if we where called as a first thing
			// TODO: maybe make it batter in the future
			if (offset <= begin) {
				return;
			}

			// if there is no next token then there is no next token at the same line
			if (empty()) {
				return;
			}

			SourceSpan prev = prevSpan();
			SourceSpan next = peek().source();

			auto prevLine = prev.front().line;
			auto nextLine = next.front().line;

			if (nextLine <= prevLine) {
				Message::of()
					.source(next)
					.error("Unexpected token '" + next.str() + "', expected end of line after " + prev.str())
					.raise();
			}
		}

};