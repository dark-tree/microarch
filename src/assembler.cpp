#include "assembler.hpp"

/*
 * class Assembler
 */

Assembler::Label& Assembler::getOrCreateLabel(std::string_view name) {
	static uint32_t next = 0;

	auto lit = labels.find(name);

	if (lit != labels.end()) {
		return lit->second;
	}

	Label& label = labels[name];
	label.id = next ++;

	return label;
}

uint8_t Assembler::parseRegisterSet(Parser& parser) {
	uint8_t set = 0;
	auto lexeme = parser.expect(Token::REGSET).lexeme();

	// skip initial '$'
	for (int i = 1; i < lexeme.length(); i++) {
		int reg = lexeme.at(i) - '0';
		set |= 1 << reg;
	}

	return set;
}

uint8_t Assembler::parseImmediate(Parser& parser) {
	auto lexeme = parser.expect(Token::INTEGER).lexeme();

	if (lexeme.length() >= 2) {
		if (lexeme[1] == 'x') return StringUtil::parseIntWithBase(lexeme, 16);
		if (lexeme[1] == 'o') return StringUtil::parseIntWithBase(lexeme, 8);
		if (lexeme[1] == 'b') return StringUtil::parseIntWithBase(lexeme, 2);
	}

	return StringUtil::parseIntWithBase(lexeme, 10);
}

void Assembler::parseOperation(Parser parser, std::string_view mnemonic, MicroWriter& writer) {

	if (mnemonic == "nop") {
		writer.putNop();
		return;
	}

	if (mnemonic == "set") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseImmediate(parser);

		writer.putSet(arg1, arg2);
		return;
	}

	if (mnemonic == "ldm") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putLdm(arg1, arg2);
		return;
	}

	if (mnemonic == "stm") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putStm(arg1, arg2);
		return;
	}

	if (mnemonic == "jmp") {
		if (parser.peek().type() == Token::REGSET) {
			uint8_t arg1 = parseRegisterSet(parser);
			parser.expect(",");
			uint8_t arg2 = parseRegisterSet(parser);

			writer.putJmp(arg1, arg2);
			return;
		}

		const Token& token = parser.expect(Token::IDENTIFIER);
		Label& label = getOrCreateLabel(token.lexeme());
		label.last_usage = token.source();
		writer.putJmp(label.id);
		return;
	}

	if (mnemonic == "cid") {
		writer.putCid(parseImmediate(parser));
		return;
	}

	if (mnemonic == "ctr") {
		writer.putCtr(parseImmediate(parser));
		return;
	}

	if (mnemonic == "nad") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putNad(arg1, arg2);
		return;
	}

	if (mnemonic == "and") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putAnd(arg1, arg2);
		return;
	}

	if (mnemonic == "xor") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putXor(arg1, arg2);
		return;
	}

	if (mnemonic == "shr") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putShr(arg1, arg2);
		return;
	}

	if (mnemonic == "mov") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putMov(arg1, arg2);
		return;
	}

	if (mnemonic == "add") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putAdd(arg1, arg2);
		return;
	}

	if (mnemonic == "cmp") {
		uint8_t arg1 = parseRegisterSet(parser);
		parser.expect(",");
		uint8_t arg2 = parseRegisterSet(parser);

		writer.putCmp(arg1, arg2);
		return;
	}

	parser.unreachable();

}

void Assembler::parseStatement(Parser parser, MicroWriter& writer) {

	// mnemonic
	const Token& token = parser.expect(Token::IDENTIFIER);
	std::string_view lexeme = token.lexeme();

	std::string_view condition = "t";
	std::string_view mnemonic = lexeme;

	auto dot = lexeme.find('.');

	if (dot != std::string_view::npos) {
		condition = lexeme.substr(0, dot);
		mnemonic = lexeme.substr(dot+1);
	}

	auto cid = conditions.find(condition);

	if (cid == conditions.end()) {
		Message::of().source(token.source()).error("Unknown condition code '" + std::string(condition) + "'").report();
		return;
	}

	auto guard = writer.pushCondition(cid->second);
	parseOperation(parser, mnemonic, writer);

}

void Assembler::parseRoot(Parser parser, MicroWriter& writer) {
	while (parser) {

		if (parser.match(Token::BREAK)) {
			continue;
		}

		if (const Token* token = parser.accept(Token::LABEL)) {
			std::string_view lexeme = token->lexeme();
			std::string_view name = lexeme.substr(0, lexeme.length() - 1);

			Label& label = getOrCreateLabel(name);

			if (label.defined) {
				Message::of().source(token->source())
					.error("Label '" + std::string(name) + "' already declared")
					.next().source(label.definition).info("Previous declaration here")
					.report();

				continue;
			}

			label.defined = true;
			label.definition = token->source();
			writer.putLabel(label.id);
			continue;
		}

		parseStatement(parser.until(Token::BREAK, "statement"), writer);
	}

}

std::vector<uint8_t> Assembler::assemble(const std::vector<Token>& tokens) {
	Parser parser {&tokens};
	MicroWriter writer;

	parseRoot(parser, writer);

	// we verify it here as writer will just throw an unhelpful out-of-bounds error
	// while here we can actually display the label name to hte user
	for (const auto& pair : labels) {
		if (!pair.second.defined) {
			Message::of().source(pair.second.last_usage).error("Label '" + std::string(pair.first) + "' used but not declared").report();
		}
	}

	// bail out early if something went wrong
	if (MessageSink::error()) {
		return {};
	}

	return writer.bake();
}