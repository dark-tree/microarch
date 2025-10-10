#include "writer.hpp"
#include <stdexcept>

/*
 * class MicroWriter
 */

void MicroWriter::putOp(uint8_t opcode, uint8_t param_1, uint8_t param_2) {
	bytes.push_back(opcode << 4 | stack.back());
	bytes.push_back(param_1);
	bytes.push_back(param_2);
}

/**
 * Do nothing.
 */
void MicroWriter::putNop() {
	putOp(OP_NOP, 0, 0);
}

/**
 * Set output registry set (Argument 1) to the immediate value specified by the value in
 * Argument 2.
 *
 * @param ors Output registry set
 * @param value Immediate 8-bit value
 */
void MicroWriter::putSet(uint8_t ors, uint8_t value) {
	putOp(OP_SET, ors, value);
}

/**
 * Load byte from memory from address specified by the input registry set (Argument
 * 2) into the output registry set (Argument 1).
 *
 * @param ors Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putLdm(uint8_t ors, uint8_t irs) {
	putOp(OP_LDM, ors, irs);
}

/**
 * Store byte into memory at the address specified by the output registry set
 * (Argument 1) from the input registry set (Argument 2).
 *
 * @param ors Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putStm(uint8_t ors, uint8_t irs) {
	putOp(OP_STM, ors, irs);
}

/**
 * The two input register values are combined to form a 16 bit value that is stored into
 * the PC register.
 *
 * @param srs Segment register set
 * @param ars Address register set
 */
void MicroWriter::putJmp(uint8_t srs, uint8_t ars) {
	putOp(OP_JPR, srs, ars);
}

/**
 * Jump to a 16 bit immediate value that is stored into the
 * PC register
 *
 * @param label the identifier of a label to link this jump with
 */
void MicroWriter::putJmp(uint32_t label) {
	links.emplace_back(label, bytes.size() + 1);
	putOp(OP_JPI, 0xFF, 0xFF);
}

/**
 * Return CPU information and identification. This function can be used to query the
 * CPU for supported extensions and other details. The immediate value (Argument 2)
 * specifies the page number to query. Registers R0 though R3 are modified and R4
 * through R7 are left as is.
 *
 * @param page The page number to query
 */
void MicroWriter::putCid(uint8_t page) {
	putOp(OP_CID, 0, page);
}

/**
 * This instruction stops code execution and depending on the arguments engages a
 * select power saving mode.
 *
 * @param flags Flags to write to the internal control register
 */
void MicroWriter::putCtr(uint8_t flags) {
	putOp(OP_CID, flags, 0);
}

/**
 * Read values from both input registry sets (Argument 1 & 2) bitwise NAND them
 * together and save the result into the output registry set (Argument 1).
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putNad(uint8_t brs, uint8_t irs) {
	putOp(OP_NAD, brs, irs);
}

/**
 * Read values from both input registry sets (Argument 1 & 2) bitwise AND them
 * together and save the result into the output registry set (Argument 1).
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putAnd(uint8_t brs, uint8_t irs) {
	putOp(OP_AND, brs, irs);
}

/**
 * Read values from both input registry sets (Argument 1 & 2) bitwise XOR them
 * together and save the result into the output registry set (Argument 1).
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putXor(uint8_t brs, uint8_t irs) {
	putOp(OP_XOR, brs, irs);
}

/**
 * Shift value of an input/output registry set (Argument 1) by an offset given as an
 * immediate value (Argument 2). Only the lower 3 bits of the offset are used, an
 * implementation is allowed to not support offsets larger than 1.
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putShr(uint8_t brs, uint8_t irs) {
	putOp(OP_SHR, brs, irs);
}

/**
 * Copy value from input registry set (Argument 2) into the output registry set
 * (Argument 1).
 *
 * @param ors Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putMov(uint8_t ors, uint8_t irs) {
	putOp(OP_MOV, ors, irs);
}

/**
 * Read values from both input registry sets (Argument 1 & 2) adds them
 * together and save the result into the output registry set (Argument 1).
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putAdd(uint8_t brs, uint8_t irs) {
	putOp(OP_ADD, brs, irs);
}

/**
 * Subtract input Argument 2 from input Argument 1, write the result to output
 * Argument 1, set the flag CF to the value of the carry bit from the operation, set the
 * flag ZF to 0 if the subtraction result is not equal to 0, set the flag ZF to 1 if the
 * subtraction result is equal to zero.
 *
 * @param brs Input/Output registry set
 * @param irs Input registry set
 */
void MicroWriter::putCmp(uint8_t brs, uint8_t irs) {
	putOp(OP_CMP, brs, irs);
}

MicroWriter::MicroWriter() {
	stack.push_back(T);
}

/// Apply the given condition code to all following instructions, until it is popped from the assembler stack using popCondition()
void MicroWriter::pushCondition(Cond condition) {
	stack.push_back(condition);
}

/// Removes the previous condition that was applied with pushCondition()
void MicroWriter::popCondition() {
	if (!stack.empty()) {
		stack.pop_back();
	}
}

/// Define a label at the current offset, label can be used before it is defined
void MicroWriter::putLabel(uint32_t label) {
	labels[label] = bytes.size();
}

/// Create and return a linked image of the program
std::vector<uint8_t> MicroWriter::bake() {
	std::vector<uint8_t> result = bytes;

	for (auto& link : links) {
		uint16_t target = labels.at(link.label) & 0xFFFF;
		uint32_t offset = link.offset;

		if (result.size() - 1 <= offset) {
			throw std::runtime_error {"Link offset out of bounds!"};
		}

		result[offset] = (target & 0xFF00) >> 8;
		result[offset + 1] = target & 0xFF;
	}

	return result;
}