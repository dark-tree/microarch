#pragma once

#include <string>
#include <writer.hpp>

#include "labeler.hpp"
#include "state.hpp"
#include "asm/x86/writer.hpp"

using namespace asmio;
using namespace asmio::x86;

class MicroInst {

	protected:

		using TwoArgumentJitInstruction = void (BufferWriter::*)(Location, Location);

		/// Get condition prefix
		std::string prefix() const;

		/// Convert regset to matching string form
		std::string regset(uint8_t regset) const;

		/// Check if this instruction should execute
		bool checkFlags(const CoreState& state) const;

		/// Generate instructions, that will compute value of the given registry set and place into the given register
		static void jitReadRegistrySet(BufferWriter& writer, Registry output, uint8_t registrySet);

		/// Place value into selected registry set
		static void jitWriteRegistrySet(BufferWriter& writer, const Location& input, uint8_t registrySet);

		/// Run the provided x86 instruction on values of registry sets a and b, save the result to registry set a
		void jitTwoArgInst(BufferWriter& writer, TwoArgumentJitInstruction instruction) const;

		/// Jumps to the returned label if the condition is not fulfilled
		Label jitWriteConditional(BufferWriter& writer) const;

		/// Surround instructions writen in the operation function with a conditional jump to make a conditional instruction
		void jitConditional(BufferWriter& writer, const std::function<void()>& then);

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
		virtual void label(Labeler& labelr);

		/**
		 * Append this instruction to the JIT buffer as native instructions,
		 * using this method on a series of instructions generates a JIT application.
		 */
		virtual void jit(BufferWriter& writer, CoreState& core) {};

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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitReadRegistrySet(writer, DL, a);
			jitReadRegistrySet(writer, BL, b);
			writer.put_sub(DL, BL);
			writer.put_pushf();
			writer.put_pop(SI);
			jitWriteRegistrySet(writer, DL, a);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitTwoArgInst(writer, &BufferWriter::put_add);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitReadRegistrySet(writer, BL, b);
			jitWriteRegistrySet(writer, BL, a);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitReadRegistrySet(writer, BL, a);
			writer.put_shr(BL, b);
			jitWriteRegistrySet(writer, BL, a);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitTwoArgInst(writer, &BufferWriter::put_xor);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitTwoArgInst(writer, &BufferWriter::put_and);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitReadRegistrySet(writer, DL, a);
			jitReadRegistrySet(writer, BL, b);
			writer.put_and(DL, BL);
			writer.put_neg(DL);
			jitWriteRegistrySet(writer, DL, a);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {

			jitReadRegistrySet(writer, DL, a);
			writer.put_and(DL, b);
			writer.put_and(CL, ~b);
			writer.put_or(CL, DL);
			writer.put_movzx(RDI, CL);
			writer.put_shr(RDI, 5);
			// Saving code resume point
			writer.put_mov(ref<WORD>(CoreState::PROGRAM_COUNTER), address + 1);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer] {
			jitWriteRegistrySet(writer, 0, cidRegistrySet);
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

	void label(Labeler& labelr) override {
		labelr.add(a << 8 | b);
	}

	std::string string() override {
		return prefix() + "jmp l_" + std::to_string(a << 8 | b);
	}

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			uint16_t offset = (a << 8) | b;
			writer.put_lea(RAX, Location(CoreState::INSTRUCTION_OFFSETS) + 8 * offset);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitReadRegistrySet(writer, DL, a);
			jitReadRegistrySet(writer, BL, b);
			writer.put_mov(BH, DL);
			writer.put_movzx(RBX, BX);
			writer.put_lea(RAX, Location(CoreState::INSTRUCTION_OFFSETS));
			writer.put_lea(RAX, RAX + RBX * 8);
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
		uint8_t address = state.read(a);
		if (!state.memorySegmented() || state.ram[CoreState::SEGMENT_REGISTER_ADDRESS] == 0) {
			auto iterator = state.peripherals.find(address);
			if (iterator != state.peripherals.end()) {
				iterator->second.write(state.read(b));
				return;
			}
		}
		if (state.memorySegmented() && CoreState::SEGMENT_REGISTER_ADDRESS != address) {
			uint16_t final_address = (state.ram[CoreState::SEGMENT_REGISTER_ADDRESS] << 8) | address;
			state.ram[final_address] = state.read(b);
		} else {
			state.ram[address] = state.read(b);
		}

	}

	std::string string() override {
		return prefix() + "stm " + regset(a) + ", " + regset(b);
	}

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this, &core] {

			// If memory structure is more complicated, than a simple array of bytes (for example we have peripheral devices
			// or a segment register, that are mapped onto specific memory addresses) we use a special array containing jump
			// instructions, which jump to appropriate functions (for example function changing memory, reading from a
			// peripheral or reading from segment register). We do it all to avoid redundant branch points.
			if (core.peripherals.size() > 0 || core.memorySegmented()) {
				writer.put_push(SI);
				jitReadRegistrySet(writer, DL, a);
				jitReadRegistrySet(writer, BL, b);
				writer.put_movzx(RDI, DL);
				writer.put_movzx(RSI, BL);
				writer.put_lea(RAX, Location(CoreState::MEMORY_WRITE_MAPPING));
				writer.put_lea(RAX, RAX + RDI * 8);
				writer.put_call(RAX);
				writer.put_pop(SI);
			} else {
				jitReadRegistrySet(writer, DL, a);
				jitReadRegistrySet(writer, BL, b);
				writer.put_movzx(RDX, DL);
				writer.put_lea(RAX, CoreState::DATA_MEMORY);
				writer.put_mov(ref(RAX + RDX), BL);
			}
		});
	}
};

struct InstLdm : MicroInst {

	InstLdm(uint16_t address, MicroWriter::Cond condition, uint8_t a, uint8_t b)
		: MicroInst(address, condition, a, b) {
	}

	void apply(CoreState& state) override {
		if (!checkFlags(state)) return;
		uint8_t address = state.read(b);

		if (!state.memorySegmented() || state.ram[CoreState::SEGMENT_REGISTER_ADDRESS] == 0) {
			auto iterator = state.peripherals.find(address);
			if (iterator != state.peripherals.end()) {
				state.write(a, iterator->second.read());
				return;
			}
		}

		if (state.memorySegmented() && CoreState::SEGMENT_REGISTER_ADDRESS != address) {
			uint16_t final_address = (state.ram[CoreState::SEGMENT_REGISTER_ADDRESS] << 8) | address;
			state.write(a, state.ram[final_address]);
		} else {
			state.write(a, state.ram[address]);
		}
	}

	std::string string() override {
		return prefix() + "ldm " + regset(a) + ", " + regset(b);
	}


	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this, &core] {

			// If memory structure is more complicated, than a simple array of bytes (for example we have peripheral devices
			// or a segment register, that are mapped onto specific memory addresses) we use a special array containing jump
			// instructions, which jump to appropriate functions (for example function changing memory, reading from a
			// peripheral or reading from segment register). We do it all to avoid redundant branch points.
			if (core.peripherals.size() > 0 || core.memorySegmented()) {
				jitReadRegistrySet(writer, BL, b);
				writer.put_movzx(RDI, BL);
				writer.put_lea(RAX, Location(CoreState::MEMORY_READ_MAPPING));
				writer.put_lea(RAX, RAX + RDI * 8);
				writer.put_call(RAX);
				jitWriteRegistrySet(writer, AL, a);
			} else {
				jitReadRegistrySet(writer, BL, b);
				writer.put_movzx(RBX, BL);
				writer.put_lea(RAX, CoreState::DATA_MEMORY);
				writer.put_mov(DL, ref(RAX + RBX));
				jitWriteRegistrySet(writer, DL, a);
			}
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

	void jit(BufferWriter& writer, CoreState& core) override {
		jitConditional(writer, [&writer, this] {
			jitWriteRegistrySet(writer, b, a);
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

	void jit(BufferWriter& writer, CoreState& core) override {
		writer.put_nop();
	}

};

