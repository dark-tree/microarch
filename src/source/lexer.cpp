#include "lexer.hpp"

#include "error.hpp"
#include "util.hpp"

/*
 * class CharPredicate
 */

CharPredicate::CharPredicate(const Predicate& predicate, const char* what)
	: what(what), predicate(predicate) {
}

CharPredicate::CharPredicate(char value)
	: expected(value), predicate([value] (char next) noexcept -> bool { return next == value; }) {
}

bool CharPredicate::test(int chr) const {
	return predicate(chr);
}

std::string CharPredicate::str() const {
	if (what) return what;
	if (expected) return StringUtil::ofChar(expected);
	return "<invalid predicate>";
}

/*
 * class CharPredicates
 */

CharPredicate CharPredicates::whitespace {[] (char c) noexcept -> bool {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}, "whitespace character"};

CharPredicate CharPredicates::alphabetic {[] (char c) noexcept -> bool {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}, "alphabetic character"};

CharPredicate CharPredicates::alphanumeric {[] (char c) noexcept -> bool {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
}, "alphanumeric character"};

CharPredicate CharPredicates::decimal {[] (char c) noexcept -> bool {
	return c >= '0' && c <= '9';
}, "decimal digit"};

CharPredicate CharPredicates::octal {[] (char c) noexcept -> bool {
	return c >= '0' && c <= '7';
}, "octal digit"};

CharPredicate CharPredicates::binary {[] (char c) noexcept -> bool {
	return c == '0' || c == '1';
}, "binary digit"};

CharPredicate CharPredicates::hexadecimal {[] (char c) noexcept -> bool {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}, "hexadecimal digit"};

CharPredicate CharPredicates::escape {[] (char c) noexcept -> bool {
	return (c == 'a') || (c == 'b') || (c == 'e') || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't') || (c == 'v') || (c == '\\') || (c == '\'') || (c == '\"');
}, "escape code"};

/*
 * class Lexer
 */

void Lexer::assertNonEmpty(NULLABLE const CharPredicate* predicate) const {
	if (empty()) {
		SourceSpan span = prevSpan();
		std::string message = "Unexpected end of input";

		if (predicate) {
			message += ", expected " + predicate->str();
		}

		Message::of().source(span).error(message).raise();
	}
}

bool Lexer::match(const char* single) {
	size_t start = offset;

	while (*single) {

		if (!match(*single)) {
			offset = start;
			return false;
		}

		single ++;
	}

	return true;
}

Lexer::Lexer(const SourceUnit* unit)
	: previous(0), offset(0), unit(unit) {
}

SourceSpan Lexer::nextSpan() const {
	// safe, at worst we will point at the \0
	return {unit, unit->ref(offset), 1};
}

SourceSpan Lexer::prevSpan() const {
	auto i = offset;

	if (i > 0) {
		i --;
	}

	return {unit, unit->ref(i), 1};
}

void Lexer::beginToken() {
	previous = offset;
}

void Lexer::rewind() {
	offset = previous;
}

std::string_view Lexer::lexeme() {
	return {unit->ref(previous), unit->ref(offset)};
}

Token Lexer::endToken(Token::Type type) {
	const Token token {type, {unit, lexeme()}};
	beginToken();
	return token;
}

SourcePos Lexer::where() const {
	return unit->find(unit->ref(offset));
}

char Lexer::peek() const {
	assertNonEmpty(nullptr);
	return *unit->ref(offset);
}

void Lexer::advance() {

	// when we reach the end we will point at the null-byte
	if (offset < unit->size()) {
		offset ++;
	}
}

char Lexer::next() {
	char c = peek();
	advance();
	return c;
}

char Lexer::expect(const CharPredicate& predicate) {
	assertNonEmpty(&predicate);
	char c = peek();

	if (predicate.test(c)) {
		advance();
		return c;
	}

	SourceSpan span = nextSpan();
	Message::of().source(span).error("Unexpected " + StringUtil::ofChar(c) + ", expected " + predicate.str()).raise();
}

bool Lexer::match(const CharPredicate& predicate) {
	char c = *unit->ref(offset);

	if (predicate.test(c)) {
		advance();
		return true;
	}

	return false;
}

bool Lexer::greedy(const CharPredicate& predicate) {
	bool matched = false;
	while (match(predicate)) matched = true;
	return matched;
}

bool Lexer::empty() const {
	return offset >= unit->size();
}

void Lexer::unreachable() {
	assertNonEmpty(nullptr);

	SourceSpan span = nextSpan();
	char c = next();
	Message::of().source(span).error("Unexpected " + StringUtil::ofChar(c)).raise();
}

Lexer::operator bool() const {
	return !empty();
}
