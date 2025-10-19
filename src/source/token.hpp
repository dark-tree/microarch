#pragma once

#include <memory>
#include <string>
#include <vector>

/**
 * Represents a identifiable position in the source, can point at the first
 * or one-past-last character in the string. Both the line and column are one-based.
 */
struct SourcePos {

	uint32_t line = 1;
	uint32_t column = 1;

	SourcePos() = default;
	SourcePos(uint32_t line, uint32_t column);

	bool operator==(const SourcePos& rhs) const;

};

std::ostream& operator<<(std::ostream& stream, const SourcePos& pos);

/**
 * Represents a file or other source of source code to proces
 * this object can't be copied and should be referenced by a pointer
 */
class SourceUnit {

	public:

		/**
		 * String containing the whole source of the specific unit,
		 * all tokens (and SourceSpan) are just views into this string
		 * in order to limit string copying
		 */
		const std::string source;

		/**
		 * The name used to identify the unit,
		 * this will usually be the path of the file but that is
		 * not required, for example when compiling a string
		 */
		const std::string path;

		/**
		 * Get a view of a particular line starting at the given column,
		 * both the column and line indices will be truncated
		 * to maintain validity of the returned object
		 *
		 * @param[in] line 1-based line numer, 0 is also accepted and treated the same as 1
		 * @param[in] column 1-based line numer, 0 is also accepted and treated the same as 1
		 */
		std::string_view line(uint32_t line, uint32_t column = 1) const;

		/**
		 * Get pointer to the start of the given line and column,
		 * both the column and line indices will be truncated
		 * to maintain validity of the returned object
		 *
		 * @param[in] line 1-based line numer, 0 is also accepted and treated the same as 1
		 * @param[in] column 1-based line numer, 0 is also accepted and treated the same as 1
		 */
		const char* at(uint32_t line, uint32_t column = 1) const;

		/**
		 * Get pointer to the start of the given SourcePos,
		 * works the same as at(size_t, size_t) but allows
		 * you to use SourcePos in place of two size_t's
		 *
		 * @param[in] pos valid (not one past last!) SourcePos object to convert to a pointer
		 */
		const char* at(SourcePos pos) const;

		/**
		 * Get a pointer into the source at the supplied offset,
		 * the function will throw if the offset falls outside
		 * the valid range of the source.
		 *
		 * @param[in] offset byte offset into the source
		 */
		const char* ref(size_t offset) const;

		/**
		 * Locate a particular point in the source, this is a slow method
		 * and should only be used for error reporting to get back the line
		 * offset from raw pointer
		 */
		SourcePos find(const char* point) const;

		/**
		 * Return the "end iterator" of the source,
		 * it points one past the end of the source string, compare it
		 * with the SourcePos to check if that sure pos is the end iterator
		 */
		SourcePos end() const;

		/**
		 * Get line count of the source
		 */
		uint32_t count() const;

		/**
		 * Get length of source
		 */
		size_t size() const;

		/**
		 * Check if the source code is empty
		 */
		bool empty() const;

		SourceUnit(const std::string& source, const std::string& path);
		SourceUnit(SourceUnit& unit) = delete;
		SourceUnit(SourceUnit&& unit) = default;

	private:

		/// internal data structure used to track line information
		struct LineDesc {
			uint32_t offset;
			int32_t length;

			[[nodiscard]] uint32_t end() const {
				return offset + length;
			}
		};

		/**
		 * Mapping of line numbers to the corresponding byte offsets into the
		 * source string, if a line was empty the offset will point at a
		 * new line character
		 */
		std::vector<LineDesc> lines;

};

/**
 * A range span of source from SourceUnit,
 * can be converted to two SourcePoses and combined with other spans
 * that target the same SourceUnit
 */
struct SourceSpan {

	private:

		static inline SourceUnit EMPTY_UNIT {"", "<empty>"};

	public:

		// by default point to an empty unit
		const SourceUnit* unit = &EMPTY_UNIT;
		const char* begin = EMPTY_UNIT.ref(0);
		const char* end = EMPTY_UNIT.ref(0);

		/**
		 * Convert this span to a std::string_view,
		 * this is a fast operation
		 */
		std::string_view view() const;

		/**
		 * Convert this span to a std::string,
		 * this is a slow operation
		 */
		std::string str() const;

		/**
		 * Convert this span to a quoted std::string,
		 * this is a slow operation
		 */
		std::string quote(char quote = '\'') const;

		/**
		 * Get the source pos of the first character in this span,
		 * if the span is empty this should not be called
		 */
		SourcePos front() const;

		/**
		 * Get the source pos of the last character in this span,
		 * if the span is empty this should not be called
		 */
		SourcePos back() const;

		/**
		 * Create a new san that contains the content of this and the other span
		 * (including all content between them), both spans must belong to the same SourceUnit
		 */
		[[nodiscard]] SourceSpan wrap(SourceSpan& other) const;

		/**
		 * Expand the span by some distance (in lines) up and down,
		 * if two zeros are passed it will be expanded to fill the current line only
		 *
		 * @param[in] up number of lines to expand upward (towards start of the file)
		 * @param[in] down number of lines to expand downward (towards end of the file)
		 */
		[[nodiscard]] SourceSpan wrap(uint32_t up, uint32_t down) const;

		/**
		 * Shorthand for .wrap(0, 0), create a
		 * new span the contains the lines the current span resides on
		 */
		[[nodiscard]] SourceSpan wrap() const;

		/**
		 * Check if the source span is empty, if that is the case some methods -
		 * like back(), front() and lines() should not be called, as they will still work as if the span was of length 1
		 */
		bool empty() const;

		/**
		 * Get the length, in bytes, of this source span.
		 * If the returned length is 0 then the span is empty
		 */
		size_t size() const;

		/**
		 * Convert this span to a list of lines, this is very slow and should be used
		 * only for error reporting, if the span is empty this should not be called
		 */
		std::vector<std::string_view> lines() const;

		SourceSpan() = default;
		SourceSpan(const SourceUnit* unit, const char* begin, const char* end);
		SourceSpan(const SourceUnit* unit, const std::string_view& view);
		SourceSpan(const SourceUnit* unit, const char* begin, size_t length);

};

/**
 * Created by the lexer from consumed characters,
 * represents a single word of the language
 */
class Token {

	public:

		using Stream = std::vector<Token>;

		/**
		 * When adding a new type
		 * add it also to the ::toString(Type)
		 */
		enum Type {
			UNKNOWN,
			IDENTIFIER,
			LABEL,
			OPERATOR,
			INTEGER,
			STRING,
			BREAK,
			SYMBOL,
			REGSET,
		};

		/**
		 * Convert Token::Type to human readable name, this returned names are
		 * NOT uppercase, and can be used in error messages.
		 */
		static const char* typeToString(Type type);

	private:

		Type m_type = UNKNOWN;
		SourceSpan m_span;

	public:

		Token() = default;
		Token(Type type, const SourceSpan& source);

		Type type() const;
		SourceSpan source() const;
		std::string_view lexeme() const;

};
