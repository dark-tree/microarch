
#include <assembler.hpp>
#include <controller.hpp>
#include <reader/reader.hpp>
#include <reader/state.hpp>
#include <source/error.hpp>
#include <source/tokenizer.hpp>

#define VSTL_TRIGGER_DEBUGGER false

#include "vstl.hpp"
#include "asm/x86/writer.hpp"
#include "out/buffer/segmented.hpp"
#include "out/elf/buffer.hpp"

TEST(tokenize_mixed) {

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		/*
		 * Tokenize this!
		 * /* Nested?? UwU */
		 * unreal
		 */

		nop;
		cmp $12, $3

		using le // inline // begin

			add $1, $
			set $01234567, 0

			loop:
				cid 0

			nz.jmp loop

		end

		// inline statements
		nop; nop; nop

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	std::vector<std::string> expected = {
		"\n","\n","\n","\n", "nop", ";", "\n", "cmp", "$12", ",", "$3", "\n",
		"\n", "using", "le", "begin", "\n", "\n", "add", "$1", ",", "$", "\n",
		"set", "$01234567", ",", "0", "\n", "\n", "loop:", "\n", "cid", "0", "\n",
		"\n", "nz.jmp", "loop", "\n", "\n", "end", "\n", "\n", "nop", ";",
		"nop", ";", "nop", "\n", "\n"
	};

	for (unsigned int i = 0; i < tokens.size(); i++) {
		const Token& token = tokens[i];

		if (token.lexeme() != expected[i]) {
			std::string error = "Token #" + std::to_string(i) + " (" + std::string(token.lexeme()) + ") is not what was expected!";
			FAIL(error);
		}
	}

};

TEST(tokenize_invalid_regset) {

	MessageSink::clear();
	MessageSink::disable();

	SourceUnit duplicate {"$11", "file"};
	SourceUnit ordering {"$21", "file"};
	SourceUnit sanity {"12", "file"};

	EXPECT_THROW(Message) {
		Tokenizer::tokenize(&duplicate);
	};

	EXPECT_THROW(Message) {
		Tokenizer::tokenize(&ordering);
	};

	Tokenizer::tokenize(&sanity);

};

TEST(assembler_basic) {

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		set $1, 100
		mov $23, $1
		nz.mov $5, $34; nop

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	CHECK(bytes.size(), 12);

	CHECK(bytes[0], 0b0001'1111);
	CHECK(bytes[1], 0b0000'0010);
	CHECK(bytes[2], 100);

	CHECK(bytes[3], 0b1100'1111);
	CHECK(bytes[4], 0b0000'1100);
	CHECK(bytes[5], 0b0000'0010);

	CHECK(bytes[6], 0b1100'0111);
	CHECK(bytes[7], 0b0010'0000);
	CHECK(bytes[8], 0b0001'1000);

	CHECK(bytes[9] & 0xF0, 0); // ignore condition bits in NOPs
	CHECK(bytes[10], 0b0000'0000);
	CHECK(bytes[11], 0b0000'0000);

};

TEST(assembler_labels) {

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		set $1, 10
		set $2, 1

		test:
			nop
			cmp $1, $2

		z.jmp test

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	CHECK(bytes.size(), 15);

	CHECK(bytes[0], 0b0001'1111);
	CHECK(bytes[3], 0b0001'1111);
	CHECK(bytes[9], 0b1111'1111);

	CHECK(bytes[12], 0b0101'1011);
	CHECK(bytes[13], 0);
	CHECK(bytes[14], 2);

};

TEST(assembler_labels_undefined) {

	int count = 0;
	MessageSink::clear();

	MessageSink::printer([&] (const Message& message) {
		count ++;

		CHECK(message.nodes().size(), 1);
		CHECK(message.nodes().front().section().front().line, 3);
	});

	SourceUnit unit {R"(

		jmp test

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	assembler.assemble(tokens);

	CHECK(count, 1);

};

TEST(assembler_labels_redefined) {

	int count = 0;
	MessageSink::clear();

	MessageSink::printer([&] (const Message& message) {
		count ++;

		CHECK(message.nodes().size(), 2);
		CHECK(message.nodes().back().section().front().line, 3);
		CHECK(message.nodes().front().section().front().line, 6);
	});

	SourceUnit unit {R"(

		test:
			nop

		test:
			mov $1, $

		jmp test

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	assembler.assemble(tokens);

	CHECK(count, 1);

};

TEST(assembler_if_block) {

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		set $1, 42
		set $4, 1
		set $5, 0

		if f begin
			mov $3, $2
			mov $4, $3
			t.mov $5, $4

			if z begin
				set $5, 0
			end

			mov $6, $5
		end

		mov $7, $5

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	CHECK(bytes.size(), 9*3);

	// check condition codes
	CHECK(bytes[0*3] & 0xF, 0b1111);
	CHECK(bytes[1*3] & 0xF, 0b1111);
	CHECK(bytes[2*3] & 0xF, 0b1111);
	CHECK(bytes[3*3] & 0xF, 0b0000);
	CHECK(bytes[4*3] & 0xF, 0b0000);
	CHECK(bytes[5*3] & 0xF, 0b1111);
	CHECK(bytes[6*3] & 0xF, 0b1011);
	CHECK(bytes[7*3] & 0xF, 0b0000);
	CHECK(bytes[8*3] & 0xF, 0b1111);

};

TEST (asmiov_sanity_check) {

	using namespace asmio;
	using namespace asmio::x86;

	SegmentedBuffer buffer;
	BufferWriter writer {buffer};

	writer.put_mov(RAX, ref(EAX + EBX * 2 + 123));
	writer.put_mov(EAX, ref(RAX + RBX * 2 + 123));

	EXPECT_ANY() { writer.put_mov(RAX, ref(RAX + EBX * 2 + 123)); };
	EXPECT_ANY() { writer.put_mov(RAX, ref(EAX + RBX * 2 + 123)); };

};

TEST(reader_disassemble) {

	MessageSink::clear();
	MessageSink::printer([&] (const Message& message) {
		FAIL("Unexpected message! " + std::string(message.what()))
	});

	SourceUnit unit {R"(

		set $1, 42
		set $4, 1
		set $5, 0

		test:
			nop
			mov $1, $

		jmp test
		mov $7, $5

	)", "file"};
	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	std::string back = state.disassemble();

	CHECK(back, R"(	set $1, 42
	set $4, 1
	set $5, 0

l_3:
	nop
	mov $1, $
	jmp l_3
	mov $7, $5
)");

};

TEST(controller_hex_input) {
	auto bytes = decodeHexString("AF2234\n778865\n\r1244AF\nEABCDA\t");

	CHECK(bytes[0], 0xAF);
	CHECK(bytes[1], 0x22);
	CHECK(bytes[2], 0x34);
	CHECK(bytes[3], 0x77);
	CHECK(bytes[4], 0x88);
	CHECK(bytes[5], 0x65);
	CHECK(bytes[6], 0x12);
	CHECK(bytes[7], 0x44);
	CHECK(bytes[8], 0xAF);
	CHECK(bytes[9], 0xEA);
};

TEST(interpreter_all_instructions) {
	SourceUnit unit {R"(

		set $01, 42
		set $23, 19
		set $45, 7
		set $67, 213
		add $0, $1
		nop
		cmp $7, $4
		mov $7, $6
		stm $6, $2
		ldm $0, $6
		nad $2, $3
		and $4, $5
		xor $4, $5
		shr $1, 2
		cid 0
		set $0, 255
		ctr $0, 7
		set $0, 98
		set $1, 213
		jmp $0, $1

	)", "file"};

	auto tokens = Tokenizer::tokenize(&unit);

	Assembler assembler;
	auto bytes = assembler.assemble(tokens);

	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	state.run(4);
	CHECK(state.regs[0], 42);
	state.run(1);
	CHECK(state.regs[0], 84);
	state.run(2);
	CHECK(state.regs[7], 206);
	state.run(1);
	CHECK(state.regs[7], 213);
	state.run(2);
	CHECK(state.regs[0], 19);
	state.run(1);
	CHECK(state.regs[2], 236);
	state.run(1);
	CHECK(state.regs[4], 7);
	state.run(1);
	CHECK(state.regs[4], 0);
	state.run(1);
	CHECK(state.regs[1], 10);
	state.run(1);
	CHECK(state.regs[0], 0);
	CHECK(state.regs[1], 0);
	CHECK(state.regs[2], 0);
	CHECK(state.regs[3], 0);
	state.run(2);
	CHECK(state.ctr.flags.reserved, 7);
	CHECK(state.ctr.flags.standby_mode, 0);
	CHECK(state.ctr.flags.interrupt, 0);
	state.run(3);
	CHECK(state.pc, 25301);
};

TEST(interpreter_fibonacci) {

	SourceUnit unit {R"(

		set $0, 10
		set $1, 1
		ptl:
			nop
		cmp $0, $1
		nz.jmp ptl
		set $7, 98

	)", "file"};

	auto tokens = Tokenizer::tokenize(&unit);
	Assembler assembler;
	auto bytes = assembler.assemble(tokens);
	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	state.run(2);
	for (int i=10; i>0; i--) {
		CHECK(state.regs[0], i);
		state.run(3);
	}
	state.run(1);
	CHECK(state.regs[7], 98);
};



TEST(jit_fibonacci) {

	SourceUnit unit {R"(

		set $0, 13
		set $1, 1
		set $2, 0
		set $3, 1
		set $5, 255
		ptl:
			ctr $5, 224
			mov $4, $3
			add $4, $2
			mov $2, $3
			mov $3, $4
		cmp $0, $1
		set $6, 0
		set $7, 5
		nz.jmp $6, $7
		ctr $5, 224

	)", "file"};

	auto tokens = Tokenizer::tokenize(&unit);
	Assembler assembler;
	auto bytes = assembler.assemble(tokens);
	MicroReader reader;
	CoreState state = reader.toProgram(bytes);

	ExecutableCore core = state.jit();

	uint8_t fibonacci_sequence [] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233};

	for (int i=0;i<14;i++) {
		core();
		CHECK(state.regs[2], fibonacci_sequence[i]);
	}


};