#include "Vid_stage_1.h"
#include "../test_runner.h"

TESTS;

    TEST(id_stage_1, test_alu_instruction)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = false;
        instruction = 0b100011111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = false;
        zf = false;
        cf = false;
        STEP(1);
        assert_signal(false, alu_write_signal, "signals inhibited when execute is not set");
        execute = true;
        STEP(1);
        assert_signal(false, instruction_finished, "not ignoring when condition set to 1111");
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(true, alu_write_signal, "properly signalling the alu");
        assert_signal_vector(0b10101010, regmask_a, "correct regmask_a");
        assert_signal_vector(0b11110000, regmask_b, "correct regmask_b");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        assert_signal(false, instruction_finished, "signalling that the instruction is not fully processed");
        assert_signal_vector(0b11111010, registers_used, "determining which registers will be used");
        instruction = 0b100011101010101011110000;
        STEP(1);

        assert_signal(true, instruction_finished, "ignoring, because cf is false");
        assert_signal(false, alu_write_signal, "not executing, because cf is false");
        assert_signal(true, instruction_finished, "signalling that the instruction is fully processed");
        cf = true;

        STEP(1);
        assert_signal(false, instruction_finished, "not ignoring after setting cf to true");
        assert_signal(true, alu_write_signal, " executing, after setting cf to true");
        assert_signal(false, instruction_finished, "signalling that the instruction is not fully processed");
        instruction = 0b100011011010101011110000;
        STEP(1)
        assert_signal(true, instruction_finished, "ignoring, because cf is true");
        assert_signal(false, alu_write_signal, "not executing, because cf is true");
        assert_signal(true, instruction_finished, "signalling that the instruction is fully processed");
    END_SIMULATION;



    TEST(id_stage_1, test_memory_instructions)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = true;
        instruction = 0b001011111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = false;
        zf = false;
        cf = false;
        STEP(1);
        assert_signal(false, instruction_finished, "not ignoring when condition set to 1111");
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(true, mmu_execute, " mmu signal");
        assert_signal(false, mmu_write, " mmu_write not set during memory load");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0b10101010, regmask_a, "correct regmask_a (address)");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        assert_signal(false, instruction_finished, "signalling that the instruction is not fully processed");
        assert_signal_vector(0b10101010, registers_used, "determining which registers will be read");
        instruction = 0b001111111010101011110000;
        STEP(1);

        assert_signal(false, instruction_finished, "not ignoring when condition set to 1111");
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(true, mmu_execute, " mmu signal");
        assert_signal(true, mmu_write, " mmu_write set during memory store");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0b10101010, regmask_a, "correct regmask_a (address)");
        assert_signal_vector(0b11110000, regmask_b, "correct regmask_b (data)");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        assert_signal_vector(0b11111010, registers_used, "determining which registers will be read");
        assert_signal(false, instruction_finished, "signalling that the instruction is not fully processed");
    END_SIMULATION;


    TEST(id_stage_1, test_ctr)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = true;
        instruction = 0b011111111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = false;
        zf = false;
        cf = false;
        STEP(1);
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(true, set_ctr, "ctr set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0b10101010, ctr_value, "correct regmask_a");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        assert_signal_vector(0b00000000, registers_used, "determining which registers will be read");
        assert_signal(true, instruction_finished, "signalling that the instruction is fully processed");

    END_SIMULATION;


    TEST(id_stage_1, test_jumps)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = true;
        instruction = 0b010110111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = false;
        zf = true;
        cf = false;
        STEP(1);
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0b1111000010101010, next_instruction_address, "correct PC set");
        assert_signal_vector(0b00000000, registers_used, "determining which registers will be read");
        assert_signal(true, instruction_finished, "jump instruction completed");
        zf = false;
        STEP(1);

        assert_signal_vector(16, next_instruction_address, "PC incrementing normally when flag conditions is not met");
        instruction = 0b010001111010101011110000;
        register_bus_a = 0b00110011;
        register_bus_b = 0b00111100;
        assert_signal(true, instruction_finished, "signalling that the instruction is fully processed");
        STEP(1)
        assert_signal_vector(0b10101010, regmask_a, "correct regmask_a");
        assert_signal_vector(0b11110000, regmask_b, "correct regmask_b");
        assert_signal_vector(0b11111010, registers_used, "determining which registers will be read");
        assert_signal_vector(0b0011110000110011, next_instruction_address, "pc correctly set to register values");
        assert_signal(true, instruction_finished, "jump instruction completed");


    END_SIMULATION;


    TEST(id_stage_1, test_cid_set_nop)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = true;
        instruction = 0b000011111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = false;
        zf = false;
        cf = false;
        STEP(1);
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal(true, instruction_finished, "signalling that instruction doesn't need to be processed");
        assert_signal_vector(0b00000000, registers_used, "determining which registers will be read");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        instruction = 0b011011111010101011110000;
        STEP(1);
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0b00000000, registers_used, "determining which registers will be read");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
        assert_signal(false, instruction_finished, "passing instruction to the next stage");
        instruction = 0b000111111010101011110000;
        STEP(1);
        assert_signal(false, set_interrupt_return_address, "no interrupt");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal_vector(0b00000000, registers_used, "determining which registers will be read");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal(false, instruction_finished, "passing instruction to the next stage");
        assert_signal_vector(16, next_instruction_address, "correct PC incrementing");
    END_SIMULATION;



    TEST(id_stage_1, test_interrupt_during_jump)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = true;
        instruction = 0b010110111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = true;
        zf = true;
        cf = false;
        STEP(1);
        assert_signal(true, set_interrupt_return_address, "interrupt return address set");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(false, alu_write_signal, "not signalling the alu");
        assert_signal_vector(0, next_instruction_address, "correct PC set");
        assert_signal(true, instruction_finished, "jump instruction completed");
        assert_signal_vector(0b1111000010101010, interrupt_return_address, "correct return address saved");


    END_SIMULATION;


    TEST(id_stage_1, test_interrupt_during_alu_instruction)
        SET_SIGNAL(execute);
        SET_SIGNAL(cf);
        SET_SIGNAL(zf);
        SET_SIGNAL(alu_write_signal);
        SET_SIGNAL(set_ctr);
        SET_SIGNAL(flag_dependent);
        SET_SIGNAL(mmu_write);
        SET_SIGNAL(mmu_execute);
        SET_SIGNAL(interrupt_signal);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(regmask_a);
        SET_SIGNAL_VECTOR(regmask_b);
        SET_SIGNAL_VECTOR(register_bus_a);
        SET_SIGNAL_VECTOR(register_bus_b);
        SET_SIGNAL_VECTOR(ctr_value);
        SET_SIGNAL_VECTOR(current_instruction_address);
        SET_SIGNAL_VECTOR(next_instruction_address);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(alu_immediate);
        SET_SIGNAL_VECTOR(registers_used);
    START_SIMULATION;

        execute = false;
        instruction = 0b100011111010101011110000;
        current_instruction_address = 13;
        interrupt_signal = true;
        zf = false;
        cf = false;
        STEP(1);
        assert_signal(false, alu_write_signal, "signals inhibited when execute is not set");
        execute = true;
        STEP(1);
        assert_signal(false, instruction_finished, "not ignoring when condition set to 1111");
        assert_signal(true, set_interrupt_return_address, "interrupt return address set");
        assert_signal(false, mmu_execute, "no mmu signal");
        assert_signal(false, set_ctr, "ctr not set");
        assert_signal(true, alu_write_signal, "properly signalling the alu");
        assert_signal_vector(0b10101010, regmask_a, "correct regmask_a");
        assert_signal_vector(0b11110000, regmask_b, "correct regmask_b");
        assert_signal_vector(0, next_instruction_address, "correct PC set");
        assert_signal(false, instruction_finished, "passing alu instruction to the next stage");
        assert_signal_vector(16, interrupt_return_address, "correct return address saved");
    END_SIMULATION;

END_TESTS;
