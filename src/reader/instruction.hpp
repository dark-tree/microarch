#pragma once

#include <string>
#include <writer.hpp>

#include "labler.hpp"
#include "state.hpp"
#include "out/buffer/segmented.hpp"

class MicroInst {

	protected:

		/// Get condition prefix
		std::string prefix() const;

		/// Convert regset to matching string form
		std::string regset(uint8_t regset) const;

		/// Check if this instruction should execute
		bool checkFlags(const CoreState& state) const;

	public:

		const uint16_t address;

		const MicroWriter::Cond condition;
		const uint8_t a;
		const uint8_t b;

		MicroInst(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
			: address(address), condition(condition), a(a), b(b) {
		}

		virtual ~MicroInst();

		/**
		 * Apply the state to the current program state,
		 * using this method on a series of instructions emulates program execution.
		 */
		virtual void apply(CoreState& state) = 0;

		/**
		 * Convert this instruction to string,
		 * using this method on a series of instructions generates disassembly.
		 */
		virtual std::string string() = 0;

		/**
		 * If this instruction references a label it should be appended to the given labeler
		 * for them to appear in the output.
		 */
		virtual void label(Labelr& labelr);

		/**
		 * Append this instruction to the JIT buffer as native instructions,
		 * using this method on a series of instructions generates a JIT application.
		 */
		virtual void jit(asmio::SegmentedBuffer& buffer) { /* TODO, for now do nothing */ }

};

/*
 * region Implementation
 */

struct InstCmp : MicroInst {

	InstCmp(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;

		int64_t diff = state.read(a) - state.read(b);
		state.cf = std::bit_cast<uint64_t>(diff) > 0xFF;
		state.zf = diff == 0;

		state.write(a, diff);
	}

	std::string string() override {
		return prefix() + "cmp " + regset(a) + ", " + regset(b);
	}

};

struct InstAdd : MicroInst {

	InstAdd(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.read(a) + state.read(b));
	}

	std::string string() override {
		return prefix() + "add " + regset(a) + ", " + regset(b);
	}

};

struct InstMov : MicroInst {

	InstMov(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.read(b));
	}

	std::string string() override {
		return prefix() + "mov " + regset(a) + ", " + regset(b);
	}

};

struct InstShr : MicroInst {

	InstShr(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.read(a) >> b);
	}

	std::string string() override {
		return prefix() + "shr " + regset(a) + ", " + std::to_string(b);
	}

};

struct InstXor : MicroInst {

	InstXor(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.read(a) ^ state.read(b));
	}

	std::string string() override {
		return prefix() + "xor " + regset(a) + ", " + regset(b);
	}

};

struct InstAnd : MicroInst {

	InstAnd(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.read(a) & state.read(b));
	}

	std::string string() override {
		return prefix() + "and " + regset(a) + ", " + regset(b);
	}

};

struct InstNad : MicroInst {

	InstNad(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, ~(state.read(a) & state.read(b)));
	}

	std::string string() override {
		return prefix() + "nad " + regset(a) + ", " + regset(b);
	}

};

struct InstCtr : MicroInst {

	InstCtr(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		throw std::runtime_error {"CTR unimplemented - No idea what this even does anymore"};
	}

	std::string string() override {
		return prefix() + "ctr " + std::to_string(b);
	}

};

struct InstCid : MicroInst {

	InstCid(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		throw std::runtime_error {"CID unimplemented - No idea what this even does anymore"};
	}

	std::string string() override {
		return prefix() + "cid " + std::to_string(b);
	}

};

struct InstJpi : MicroInst {

	InstJpi(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.pc = a << 8 | b;
	}

	void label(Labelr& labelr) override {
		labelr.add(a << 8 | b);
	}

	std::string string() override {
		return prefix() + "jmp l_" + std::to_string(a << 8 | b);
	}

};

struct InstJpr : MicroInst {

	InstJpr(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.pc = state.read(a) << 8 | state.read(b);
	}

	std::string string() override {
		return prefix() + "jmp " + regset(a) + ", " + regset(b);
	}

};

struct InstStm : MicroInst {

	InstStm(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.ram[state.read(a)] = state.read(b);
	}

	std::string string() override {
		return prefix() + "stm " + regset(a) + ", " + regset(b);
	}

};

struct InstLdm : MicroInst {

	InstLdm(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, state.ram[state.read(b)]);
	}

	std::string string() override {
		return prefix() + "ldm " + regset(a) + ", " + regset(b);
	}

};

struct InstSet : MicroInst {

	InstSet(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(a, b);
	}

	std::string string() override {
		return prefix() + "set " + regset(a) + ", " + std::to_string(b);
	}

};

struct InstNop : MicroInst {

	InstNop(uint16_t address)
		: MicroInst(address, MicroWriter::Cond::T, 0, 0) {
	}

	void apply(CoreState& state) override {
		// do nothing
	}

	std::string string() override {
		return "nop";
	}

	void jit(asmio::SegmentedBuffer& buffer) override {
		// do nothing
	}

};