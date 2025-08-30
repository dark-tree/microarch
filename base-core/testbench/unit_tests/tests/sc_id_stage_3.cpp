#include "Vid_stage_3.h"
#include "../test_runner.h"

TESTS;

    TEST(id_stage_3, test_shr)
        SET_SIGNAL(set_registers);
        SET_SIGNAL(trigger_cid);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(accumulator);
        SET_SIGNAL_VECTOR(mmu_data_register);

        SET_SIGNAL_VECTOR(regset);
        SET_SIGNAL_VECTOR(register_bus);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b101111110111001101010101;
        accumulator = 213;
        STEP(1);

        assert_signal(true, set_registers, "set_registers");
        assert_signal(false, trigger_cid, "trigger_cid");
        assert_signal_vector(0b01110011, registers_set, "registers_set");
        assert_signal_vector(0b01110011, regset, "regset");
        assert_signal_vector(213, register_bus, "regset");

    END_SIMULATION;


    TEST(id_stage_3, test_cid)
        SET_SIGNAL(set_registers);
        SET_SIGNAL(trigger_cid);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(accumulator);
        SET_SIGNAL_VECTOR(mmu_data_register);

        SET_SIGNAL_VECTOR(regset);
        SET_SIGNAL_VECTOR(register_bus);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b011011110111001101010101;
        accumulator = 213;
        STEP(1);

        assert_signal(false, set_registers, "set_registers");
        assert_signal(true, trigger_cid, "trigger_cid");
        assert_signal_vector(0b11110000, registers_set, "regset");


    END_SIMULATION;


    TEST(id_stage_3, test_ldm)
        SET_SIGNAL(set_registers);
        SET_SIGNAL(trigger_cid);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(accumulator);
        SET_SIGNAL_VECTOR(mmu_data_register);

        SET_SIGNAL_VECTOR(regset);
        SET_SIGNAL_VECTOR(register_bus);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b001011110111001101010101;
        mmu_data_register = 97;
        STEP(1);

        assert_signal(true, set_registers, "set_registers");
        assert_signal(false, trigger_cid, "trigger_cid");
        assert_signal_vector(0b01010101, registers_set, "registers_set");
        assert_signal_vector(0b01010101, regset, "regset");
        assert_signal_vector(97, register_bus, "regset");

    END_SIMULATION;


    TEST(id_stage_3, test_ldm)
        SET_SIGNAL(set_registers);
        SET_SIGNAL(trigger_cid);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(accumulator);
        SET_SIGNAL_VECTOR(mmu_data_register);

        SET_SIGNAL_VECTOR(regset);
        SET_SIGNAL_VECTOR(register_bus);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b001011110111001101010101;
        mmu_data_register = 97;
        STEP(1);

        assert_signal(true, set_registers, "set_registers");
        assert_signal(false, trigger_cid, "trigger_cid");
        assert_signal_vector(0b01010101, registers_set, "registers_set");
        assert_signal_vector(0b01010101, regset, "regset");
        assert_signal_vector(97, register_bus, "regset");

    END_SIMULATION;


    TEST(id_stage_3, test_nop)
        SET_SIGNAL(set_registers);
        SET_SIGNAL(trigger_cid);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(accumulator);
        SET_SIGNAL_VECTOR(mmu_data_register);

        SET_SIGNAL_VECTOR(regset);
        SET_SIGNAL_VECTOR(register_bus);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b000011110111001101010101;
        mmu_data_register = 97;
        STEP(1);

        assert_signal(false, set_registers, "set_registers");
        assert_signal(false, trigger_cid, "trigger_cid");
        assert_signal_vector(0, registers_set, "registers_set");
  

    END_SIMULATION;

END_TESTS;
