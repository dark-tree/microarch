#include "token.hpp"

/*
 * class SourcePos
 */

SourcePos::SourcePos(uint32_t line, uint32_t column)
	: line(line), column(column) {
}

bool SourcePos::operator==(const SourcePos& rhs) const {
	return line == rhs.line && column == rhs.column;
}

std::ostream& operator<<(std::ostream& stream, const SourcePos& pos) {
	stream << "{" << pos.line << ":" << pos.column << "}";
	return stream;
}

/*
 * class SourceUnit
 */

std::string_view SourceUnit::line(uint32_t line, uint32_t column) const {

	const auto line_count = static_cast<int64_t>(lines.size());

	int64_t sl = line != 0 ? line - 1 : 0;
	int64_t sc = column != 0 ? column - 1 : 0;

	// the file has no lines
	if (line_count == 0) {
		return {source.c_str(), source.size()};
	}

	// upper bound
	if (sl >= line_count) sl = line_count - 1;
	const LineDesc& desc = lines.at(sl);

	if (sc >= desc.length) {
		sc = desc.length;
	}

	return {source.c_str() + desc.offset + sc, static_cast<size_t>(desc.length - sc)};
}

const char* SourceUnit::at(uint32_t line, uint32_t column) const {
	return this->line(line, column).begin();
}

const char* SourceUnit::at(SourcePos pos) const {
	return at(pos.line, pos.column);
}

const char* SourceUnit::ref(size_t offset) const {
	if (offset > source.size()) {
		throw std::range_error {"Source offsets points past the end iterator!"};
	}

	return source.c_str() + offset;
}

SourcePos SourceUnit::find(const char* point) const {
	if (point < source.c_str()) {
		return {1, 1};
	}

	const uint32_t offset = std::distance(source.c_str(), point);
	uint32_t index = 0;

	for (const LineDesc& desc : lines) {
		if (desc.offset <= offset && desc.end() > offset) {
			return {index + 1, offset - desc.offset + 1};
		}

		index ++;
	}

	// return one past the last line
	return end();
}

SourcePos SourceUnit::end() const {
	return {static_cast<uint32_t>(lines.size()) + 1, 1};
}

uint32_t SourceUnit::count() const {
	return lines.size();
}

size_t SourceUnit::size() const {
	return source.size();
}

bool SourceUnit::empty() const {
	return source.empty();
}

SourceUnit::SourceUnit(const std::string& source, const std::string& path)
	: source(source), path(path) {

	if (source.empty()) {
		return;
	}

	const int64_t last = static_cast<int64_t>(source.size()) - 1;
	lines.emplace_back(0, 0);

	for (int64_t i = 0; i <= last; i ++) {
		const char c = source[i];

		if (c == '\n') {
			lines.back().length = i - lines.back().offset + 1;
			lines.emplace_back(i + 1, 0);
		}

		if (i == last) {
			lines.back().length = i - lines.back().offset + 1;
		}
	}
}


/*
 * class SourceSpan
 */

std::string_view SourceSpan::view() const {
	return {begin, end};
}

std::string SourceSpan::str() const {
	return std::string {view()};
}

std::string SourceSpan::quote(char quote) const {
	return quote + str() + quote;
}

SourcePos SourceSpan::front() const {
	return unit->find(begin);
}

SourcePos SourceSpan::back() const {
	return empty() ? front() : unit->find(end - 1);
}

SourceSpan SourceSpan::wrap(SourceSpan& other) const {
	if (other.unit != unit) {
		throw std::runtime_error {"Unable to combine spans from two disjointed sources units '" + unit->path + "' and '" + other.unit->path + "'!"};
	}

	// make sure we never take empty spans into account
	if (empty()) return other;
	if (other.empty()) return *this;

	const char* low = std::min(begin, other.begin);
	const char* high = std::max(end, other.end);

	return {unit, low, high};
}

SourceSpan SourceSpan::wrap(uint32_t up, uint32_t down) const {
	const uint32_t start = front().line;
	const uint32_t last = back().line;

	// this still allows the result to be equal 0
	// but that is allowed by unit->at()
	if (up > start) up = start;
	if (last + down > unit->count()) down = unit->count() - last;

	return {unit, unit->line(start - up).begin(), unit->line(last + down).end()};
}

SourceSpan SourceSpan::wrap() const {
	return wrap(0, 0);
}

bool SourceSpan::empty() const {
	return begin == end;
}

size_t SourceSpan::size() const {
	return std::distance(begin, end);
}

std::vector<std::string_view> SourceSpan::lines() const {
	const uint32_t start = front().line;
	const uint32_t last = back().line;

	std::vector<std::string_view> result;
	result.reserve(last - start + 1);

	for (uint32_t i = start; i <= last; i ++) {
		result.push_back(unit->line(i));
	}

	return result;
}

SourceSpan::SourceSpan(const SourceUnit* unit, const char* begin, const char* end)
	: unit(unit), begin(begin), end(end) {
}

SourceSpan::SourceSpan(const SourceUnit* unit, const std::string_view& view)
	: unit(unit), begin(view.begin()), end(view.end()) {
}

SourceSpan::SourceSpan(const SourceUnit* unit, const char* begin, size_t length)
	: unit(unit), begin(begin), end(begin + length) {
}

/*
 * class Token
 */

const char* Token::typeToString(Type type) {
	switch (type) {
		case UNKNOWN: return "unknown";
		case IDENTIFIER: return "identifier";
		case LABEL: return "label";
		case OPERATOR: return "operator";
		case INTEGER: return "integer";
		case STRING: return "string";
		case BREAK: return "break";
		case SYMBOL: return "symbol";
		case REGSET: return "register set";
	}

	return "invalid";
}

Token::Token(Type type, const SourceSpan& source)
	: m_type(type), m_span(source) {
}

Token::Type Token::type() const {
	return m_type;
}

SourceSpan Token::source() const {
	return m_span;
}

std::string_view Token::lexeme() const {
	return m_span.view();
}

