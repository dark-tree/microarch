#include "util.hpp"

/*
 * class StringUtil
 */

std::string StringUtil::ofChar(char ascii) {

	if (ascii == '\a') return "ASCII audible bell code";
	if (ascii == '\b') return "ASCII backspace code";
	if (ascii == 0x1b) return "ASCII escape code";
	if (ascii == '\f') return "formfeed page break";
	if (ascii == '\n') return "new line";
	if (ascii == '\r') return "carrige return";
	if (ascii == '\t') return "tab";
	if (ascii == '\v') return "vertical tab";
	if (ascii == ' ') return "' ' (space)";

	if (ascii <= ' ' || ascii >= '~') {
		return std::format("\\x{:x} character", ascii);
	}

	return std::string("character '") + ascii + "'";
}

bool StringUtil::isPrintable(char c) {
	return c > ' ' && c <= '~'; // exclude space
}