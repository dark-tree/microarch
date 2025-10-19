#pragma once

#include <functional>
#include <macro.hpp>

#include "token.hpp"

class CharPredicate {

	private:

		using Predicate = std::function<bool(int)>;

		int expected = 0;
		const char* what = nullptr;

		Predicate predicate;

	public:

		CharPredicate(const Predicate& predicate, const char* what);
		CharPredicate(char value);

		bool test(int chr) const;

		/**
		 * Get a human-readable description of this predicate,
		 * this method is slow and should be used only for debugging and error reporting.
		 */
		std::string str() const;

};

class CharPredicates {

	public:

		/// match any character that may reasonably be called a white space
		static CharPredicate whitespace;

		/// matches character from the set: a-z, A-Z, _
		static CharPredicate alphabetic;

		/// same as alphabetic but expanded with 0-9
		static CharPredicate alphanumeric;

		/// decimal digits (0-9)
		static CharPredicate decimal;

		/// hexadecimal digits (0-9, a-f, A-F)
		static CharPredicate hexadecimal;

		/// octal digits (0-7)
		static CharPredicate octal;

		/// binary digits (0, 1)
		static CharPredicate binary;

		/// matches standard C89 escape characters, and \e (excluding \x, \u, and \U)
		static CharPredicate escape;

};

class Lexer {

	private:

		size_t previous;
		size_t offset;
		const SourceUnit* unit;

		void assertNonEmpty(NULLABLE const CharPredicate* predicate) const;
		bool match(const char* single);

	public:

		Lexer(const SourceUnit* unit);

		/**
		 * Get the span of the next character (the one
		 * retuned by peek(), next(), etc.)
		 */
		SourceSpan nextSpan() const;

		/**
		 * Get the span of the previous character (the one
		 * previously returned by the lexer)
		 */
		SourceSpan prevSpan() const;

		/**
		 * Starts a new token (effectively clearing the current one),
		 * and sets a new rewind pos, see {@link rewind()}
		 */
		void beginToken();

		/**
		 * Rewinds the lexer to the previous {@link beginToken()} call,
		 * clearing the current token in the process.
		 */
		void rewind();

		/**
		 * Get the currently collected lexeme as a string_view
		 * this represents the same value that is used to create tokens.
		 */
		std::string_view lexeme();

		/**
		 * Create a new token (equal to the current value of {@link lexeme()},
		 * and clears the current lexeme.
		 */
		Token endToken(Token::Type type);

		/**
		 * Returns the source position of the next character in the stream,
		 * this can include the end-of-file source location if this is the stream is empty.
		 */
		SourcePos where() const;

		/**
		 * Returns the the next character in the stream,
		 * or throws a Message if there is no next character.
		 */
		char peek() const;

		/**
		 * Move to the next character in the stream, this will do
		 * nothing once we reach the end of input, silently ignoring all calls.
		 */
		void advance();

		/**
		 * Return the next character in the stream and consume it,
		 * moving to the next one. This will always return the same value as peek().
		 */
		char next();

		/**
		 * Asserts the given predicate will match the next character
		 * if that is not the cases it creates and throws an error message.
		 */
		char expect(const CharPredicate& predicate);

		/**
		 * Checks if the given predicate matches the next character,
		 * if the stream is empty 'false' is returned instead of throwing.
		 */
		bool match(const CharPredicate& predicate);

		/**
		 * Match tokens greedily until one does not match or
		 * the stream ends. Returns true if at least one character was matched.
		 */
		bool greedy(const CharPredicate& predicate);

		/**
		 * Check if the stream is empty (no more characters) or not,
		 * some methods (like. next(), peek()) throw when invoked on empty streams.
		 */
		bool empty() const;

		/**
		 * Mark a spot in the tokenization loop that should not be reached if the code was valid,
		 * will generate a "unexpected character" error for the NEXT character in the stream, the char is consumed.
		 */
		void unreachable();

		/**
		 * Implicit bool conversion,
		 * returns the inverse of empty().
		 */
		operator bool() const;

		/**
		 * Assert matching multiple characters in sequence from a set of sequences,
		 * if any sequence in the set matches true is returned. False is always returned for empty streams.
		 */
		template <typename... Args>
		bool accept(Args... args) {
			return (match(args) || ...);
		}

};
