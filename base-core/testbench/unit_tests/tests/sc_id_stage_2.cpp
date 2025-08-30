#include "Vid_stage_2.h"
#include "../test_runner.h"

TESTS;

    TEST(id_stage_2, test_shr)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b101111110111001101010101;

        STEP(1);

        assert_signal(true, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0b01110011, registers_set, "registers_set");

    END_SIMULATION;



    TEST(id_stage_2, test_cmp)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b111111110111001101010101;

        STEP(1);

        assert_signal(true, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(true, setting_flags, "setting_flags");
        assert_signal_vector(0b01110011, registers_set, "registers_set");

    END_SIMULATION;



    TEST(id_stage_2, test_ldm)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b001011110111001101010101;
        mmu_ready = false;
        STEP(1);

        assert_signal(false, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0b01110011, registers_set, "registers_set");

        mmu_ready = true;
        STEP(1);
        assert_signal(false, instruction_finished, "instruction_finished after mmu reported ready");
        assert_signal(true, instruction_ready, "instruction_ready after mmu reported ready");
    END_SIMULATION;


    TEST(id_stage_2, test_stm)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b001111110111001101010101;
        mmu_ready = false;
        STEP(1);

        assert_signal(false, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0, registers_set, "registers_set");

        mmu_ready = true;
        STEP(1);
        assert_signal(true, instruction_finished, "instruction_finished after mmu reported ready");
        assert_signal(true, instruction_ready, "instruction_ready after mmu reported ready");
    END_SIMULATION;



    TEST(id_stage_2, test_nop)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b000011110111001101010101;
        STEP(1);

        assert_signal(true, instruction_ready, "instruction_ready");
        assert_signal(true, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0, registers_set, "registers_set");

    END_SIMULATION;



    TEST(id_stage_2, test_set)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b000111110111001101010101;
        STEP(1);

        assert_signal(true, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0b01110011, registers_set, "registers_set");

    END_SIMULATION;



    TEST(id_stage_2, test_cid)
        SET_SIGNAL(mmu_ready);
        SET_SIGNAL(instruction_ready);
        SET_SIGNAL(instruction_finished);
        SET_SIGNAL(setting_flags);

        SET_SIGNAL_VECTOR(instruction);
        SET_SIGNAL_VECTOR(registers_set);
    START_SIMULATION;
        instruction = 0b011011110111001101010101;
        STEP(1);

        assert_signal(true, instruction_ready, "instruction_ready");
        assert_signal(false, instruction_finished, "instruction_finished");
        assert_signal(false, setting_flags, "setting_flags");
        assert_signal_vector(0b11110000, registers_set, "registers_set");

    END_SIMULATION;

END_TESTS;
