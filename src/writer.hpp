#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

class MicroWriter {

	private:

		/// Removes the previous condition that was applied with pushCondition()
		void popCondition();

		struct ConditionScopeGuard {
			MicroWriter& writer;

			ConditionScopeGuard(MicroWriter& writer)
				: writer(writer) {
			}

			~ConditionScopeGuard() {
				writer.popCondition();
			}
		};

		// instruction flag parts
		static constexpr uint8_t ZF_TRUE = 0b1000;
		static constexpr uint8_t ZF_FALSE = 0b0100;
		static constexpr uint8_t ZF_IGNORE = 0b1100;
		static constexpr uint8_t CF_TRUE = 0b0010;
		static constexpr uint8_t CF_FALSE = 0b0001;
		static constexpr uint8_t CF_IGNORE = 0b0011;

		// instruction opcodes
		static constexpr uint8_t OP_NOP = 0b0000;
		static constexpr uint8_t OP_SET = 0b0001;
		static constexpr uint8_t OP_LDM = 0b0010;
		static constexpr uint8_t OP_STM = 0b0011;
		static constexpr uint8_t OP_JPR = 0b0100;
		static constexpr uint8_t OP_JPI = 0b0101;
		static constexpr uint8_t OP_CID = 0b0110;
		static constexpr uint8_t OP_CTR = 0b0111;
		static constexpr uint8_t OP_NAD = 0b1000;
		static constexpr uint8_t OP_AND = 0b1001;
		static constexpr uint8_t OP_XOR = 0b1010;
		static constexpr uint8_t OP_SHR = 0b1011;
		static constexpr uint8_t OP_MOV = 0b1100;
		static constexpr uint8_t OP_EXT = 0b1101; // unused, reserved for future use
		static constexpr uint8_t OP_ADD = 0b1110;
		static constexpr uint8_t OP_CMP = 0b1111;

		struct Link {
			uint32_t label;  // the unique label identifier
			uint32_t offset; // byte offset from the start of the buffer
		};

	public:

		enum Cond : uint8_t {
			F   = 0,
			T   = CF_IGNORE | ZF_IGNORE,
			NBA = ZF_FALSE | CF_FALSE,
			A   = NBA,
			NC  = CF_FALSE | ZF_IGNORE,
			NB  = NC,
			AE  = NC,
			C   = CF_TRUE | ZF_IGNORE,
			B   = C,
			NAE = C,
			NE  = CF_IGNORE | ZF_FALSE,
			NZ  = NE,
			E   = CF_IGNORE | ZF_TRUE,
			Z   = E
		};

	private:

		std::vector<Cond> stack;
		std::vector<uint8_t> bytes;
		std::vector<Link> links;
		std::unordered_map<uint32_t, uint32_t> labels; // maps label identifier to byte offset

		void putOp(uint8_t opcode, uint8_t param_1, uint8_t param_2);

	public:

		/**
		 * Do nothing.
		 */
		void putNop();

		/**
		 * Set output registry set (Argument 1) to the immediate value specified by the value in
		 * Argument 2.
		 *
		 * @param ors Output registry set
		 * @param value Immediate 8-bit value
		 */
		void putSet(uint8_t ors, uint8_t value);

		/**
		 * Load byte from memory from address specified by the input registry set (Argument
		 * 2) into the output registry set (Argument 1).
		 *
		 * @param ors Output registry set
		 * @param irs Input registry set
		 */
		void putLdm(uint8_t ors, uint8_t irs);

		/**
		 * Store byte into memory at the address specified by the output registry set
		 * (Argument 1) from the input registry set (Argument 2).
		 *
		 * @param ors Output registry set
		 * @param irs Input registry set
		 */
		void putStm(uint8_t ors, uint8_t irs);

		/**
		 * The two input register values are combined to form a 16 bit value that is stored into
		 * the PC register.
		 *
		 * @param srs Segment register set
		 * @param ars Address register set
		 */
		void putJmp(uint8_t srs, uint8_t ars);

		/**
		 * Jump to a 16 bit immediate value that is stored into the
		 * PC register
		 *
		 * @param label the identifier of a label to link this jump with
		 */
		void putJmp(uint32_t label);

		/**
		 * Return CPU information and identification. This function can be used to query the
		 * CPU for supported extensions and other details. The immediate value (Argument 2)
		 * specifies the page number to query. Registers R0 though R3 are modified and R4
		 * through R7 are left as is.
		 *
		 * @param page The page number to query
		 */
		void putCid(uint8_t page);

		/**
		 * This instruction stops code execution and depending on the arguments engages a
		 * select power saving mode.
		 *
		 * @param flags Flags to write to the internal control register
		 */
		void putCtr(uint8_t flags);

		/**
		 * Read values from both input registry sets (Argument 1 & 2) bitwise NAND them
		 * together and save the result into the output registry set (Argument 1).
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putNad(uint8_t brs, uint8_t irs);

		/**
		 * Read values from both input registry sets (Argument 1 & 2) bitwise AND them
		 * together and save the result into the output registry set (Argument 1).
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putAnd(uint8_t brs, uint8_t irs);

		/**
		 * Read values from both input registry sets (Argument 1 & 2) bitwise XOR them
		 * together and save the result into the output registry set (Argument 1).
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putXor(uint8_t brs, uint8_t irs);

		/**
		 * Shift value of an input/output registry set (Argument 1) by an offset given as an
		 * immediate value (Argument 2). Only the lower 3 bits of the offset are used, an
		 * implementation is allowed to not support offsets larger than 1.
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putShr(uint8_t brs, uint8_t irs);

		/**
		 * Copy value from input registry set (Argument 2) into the output registry set
		 * (Argument 1).
		 *
		 * @param ors Output registry set
		 * @param irs Input registry set
		 */
		void putMov(uint8_t ors, uint8_t irs);

		/**
		 * Read values from both input registry sets (Argument 1 & 2) adds them
		 * together and save the result into the output registry set (Argument 1).
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putAdd(uint8_t brs, uint8_t irs);

		/**
		 * Subtract input Argument 2 from input Argument 1, write the result to output
		 * Argument 1, set the flag CF to the value of the carry bit from the operation, set the
		 * flag ZF to 0 if the subtraction result is not equal to 0, set the flag ZF to 1 if the
		 * subtraction result is equal to zero.
		 *
		 * @param brs Input/Output registry set
		 * @param irs Input registry set
		 */
		void putCmp(uint8_t brs, uint8_t irs);

	public:

		MicroWriter();

		/// Apply the given condition code to all following instructions, until it is popped from the assembler stack using popCondition()
		ConditionScopeGuard pushCondition(Cond condition);

		/// Define a label at the current offset, label can be used before it is defined
		void putLabel(uint32_t label);

		/// Create and return a linked image of the program
		std::vector<uint8_t> bake();

};
