#pragma once

#include <string>
#include <format>

struct StringUtil {

	/**
	 * Returns a human-readable description of the given ASCII character,
	 * the returned string will not contain any non-printable character or new-lines.
	 */
	static std::string ofChar(char ascii);

	/**
	 * Check if the character is a simple printable ASCII, and
	 * not some control-code or extended ascii. (Excludes space character)
	 */
	static bool isPrintable(char c);

};
