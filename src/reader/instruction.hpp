#pragma once

#include <string>
#include <writer.hpp>

#include "labler.hpp"
#include "state.hpp"
#include "asm/x86/writer.hpp"
#include "out/buffer/segmented.hpp"

using namespace asmio;
using namespace asmio::x86;

class MicroInst {

	protected:

		/// Get condition prefix
		std::string prefix() const;

		/// Convert regset to matching string form
		std::string regset(uint8_t regset) const;

		/// Check if this instruction should execute
		bool checkFlags(const CoreState& state) const;

		static void jitComputeRegistrySet(BufferWriter& writer, Registry output, uint8_t registrySet) {
			bool firstMoved = false;
			for (unsigned int i = 0; i<CoreState::REGISTER_COUNT; i++) {
				if (registrySet % 2) {
					if (!firstMoved) {
						writer.put_mov(output, CoreState::REGISTRY_MAPPING[i]);
						firstMoved = true;
					} else {
						writer.put_or(output, CoreState::REGISTRY_MAPPING[i]);
					}
				}
				registrySet = registrySet >> 1;
			}
		}

		static void jitWriteToRegistrySet(BufferWriter& writer, Location input, uint8_t registrySet) {
			for (unsigned int i = 0; i<CoreState::REGISTER_COUNT; i++) {
				if (registrySet % 2) {
					writer.put_mov( CoreState::REGISTRY_MAPPING[i], input);
				}
				registrySet = registrySet >> 1;
			}
		}

		typedef void(BufferWriter::*TwoArgumentJITInstruction)(Location, Location);

		void jitApplyInstructionOnRegistrySets(BufferWriter& writer, TwoArgumentJITInstruction instruction) const {
			jitComputeRegistrySet(writer, DL, a);
			jitComputeRegistrySet(writer, BL, b);
			(writer.*instruction)(DL, BL);
			jitWriteToRegistrySet(writer, DL, a);
		}

		Label jitInsertConditionalJumpIfNeeded(BufferWriter& writer) const{
			if (condition == MicroWriter::T) {
				return {};
			}
			auto label = Label::make_unique();
			if (condition == MicroWriter::F) {
				writer.put_jmp(label);
			}
			else {
				static std::unordered_map <MicroWriter::Cond, void(BufferWriter::*)(Location)> JUMP_CONDITION_MAPPING ={
					{MicroWriter::Cond::NBE, &BufferWriter::put_jbe},
					{MicroWriter::Cond::NC, &BufferWriter::put_jc},
					{MicroWriter::Cond::C, &BufferWriter::put_jnc},
					{MicroWriter::Cond::NE, &BufferWriter::put_je},
					{MicroWriter::Cond::E, &BufferWriter::put_jne},
				};
				writer.put_push(SI);
				writer.put_popf();
				auto func = JUMP_CONDITION_MAPPING.at(condition);
				(writer.*func)(label);
			}
			return label;
		}

		void jitConditionalExecute(BufferWriter& writer, std::function<void()>&& operation) {
			auto label = jitInsertConditionalJumpIfNeeded(writer);
			operation();
			if (!label.empty()) {
				writer.label(label);
			}
		}

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
		virtual void jit(BufferWriter& writer){};

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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, DL, a);
			jitComputeRegistrySet(writer, BL, b);
			writer.put_sub(DL, BL);
			writer.put_pushf();
			writer.put_pop(SI);
			jitWriteToRegistrySet(writer, DL, a);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitApplyInstructionOnRegistrySets(writer, &BufferWriter::put_add);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, BL, b);
			jitWriteToRegistrySet(writer, BL, a);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, BL, a);
			writer.put_shr(BL, b);
			jitWriteToRegistrySet(writer, BL, a);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitApplyInstructionOnRegistrySets(writer, &BufferWriter::put_xor);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitApplyInstructionOnRegistrySets(writer, &BufferWriter::put_and);
		});
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

		void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, DL, a);
			jitComputeRegistrySet(writer, BL, b);
			writer.put_and(DL, BL);
			writer.put_neg(DL);
			jitWriteToRegistrySet(writer, DL, a);
		});
	}
};

struct InstCtr : MicroInst {

	InstCtr(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.ctr.raw_byte = (state.read(a) & b) | (state.ctr.raw_byte & (!b));
	}

	std::string string() override {
		return prefix() + "ctr " + std::to_string(b);
	}

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {

			jitComputeRegistrySet(writer, DL, a);
			writer.put_and(DL, b);
			writer.put_and(CL, ~b);
			writer.put_or(CL, DL);
			writer.put_movzx(RDI, CL);
			writer.put_shr(RDI, 5);
			// Saving code resume point
			writer.put_mov(ref<WORD>(CoreState::PROGRAM_COUNTER), address+1);
			// Stopping the execution
			writer.put_jnz(CoreState::CLEANUP_CODE);
		});
	}
};

struct InstCid : MicroInst {

	static const uint8_t cidRegistrySet = 0b00001111;

	InstCid(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		state.write(cidRegistrySet, 0);
	}

	std::string string() override {
		return prefix() + "cid " + std::to_string(b);
	}

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitWriteToRegistrySet(writer, 0, cidRegistrySet);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			uint16_t offset = (a << 8) | b;
			writer.put_mov(RDX, ref<QWORD>(Location(CoreState::INSTRUCTION_OFFSETS) + 8*offset));
			writer.put_lea(RAX, CoreState::PROGRAM_MEMORY);
			writer.put_add(RAX, RDX);
			writer.put_jmp(RAX);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, DL, a);
			jitComputeRegistrySet(writer, BL, b);
			writer.put_mov(BH, DL);
			writer.put_movzx(RBX, BX);
			writer.put_lea(RAX, CoreState::INSTRUCTION_OFFSETS);
			writer.put_mov(RDX, ref<QWORD>( RAX + RBX*8));
			writer.put_lea(RAX, CoreState::PROGRAM_MEMORY);
			writer.put_add(RAX, RDX);
			writer.put_jmp(RAX);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, DL, a);
			jitComputeRegistrySet(writer, BL, b);
			writer.put_lea(RAX, CoreState::DATA_MEMORY);
			writer.put_mov(ref(RAX+DL), BL);
		});
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


	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitComputeRegistrySet(writer, BL, b);
			writer.put_lea(RAX, CoreState::DATA_MEMORY);
			writer.put_mov(DL, ref(RAX+BL));
			jitWriteToRegistrySet(writer, DL, a);
		});
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

	void jit(BufferWriter& writer) override {
		jitConditionalExecute(writer, [&writer, this]() {
			jitWriteToRegistrySet(writer, b, a);
		});
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

	void jit(BufferWriter& writer) override {
		writer.put_nop();
	}

};

