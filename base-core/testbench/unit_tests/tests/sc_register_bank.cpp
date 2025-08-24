#include "Vregister_bank.h"
#include "../test_runner.h"

TESTS;

    TEST(register_bank, test_single_register_writes_and_reads)
        SET_SIGNAL(clk)
        SET_SIGNAL(setter);
        SET_SIGNAL_VECTOR(o_regmask_b);
        SET_SIGNAL_VECTOR(o_bus_b);
        SET_SIGNAL_VECTOR(o_regmask_a);
        SET_SIGNAL_VECTOR(o_bus_a);
        SET_SIGNAL_VECTOR(i_bus);
        SET_SIGNAL_VECTOR(i_regmask)
    START_SIMULATION;
        i_bus = 46;
        i_regmask = 0b01000000;
        clk = false;
        setter = false;
        o_regmask_a = 0b01000000;
        o_regmask_b = 0;
        STEP(1);
        assert_signal_vector(0, o_bus_a, "not setting value until clock");
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(0, o_bus_a, "not setting value without setter");
        setter = true;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(46, o_bus_a, "value 1 properly set and read");
        assert_signal_vector(0, o_bus_b, "bus b empty");
        for(int i=2; i<8;i++)
        {
            i_regmask = 0b10000000 >> i;
            i_bus = i;
            STEP(1);
            clk = true;
            STEP(1);
            clk = false;
            o_regmask_b = 0b01000000;
            o_regmask_a = i_regmask;
            STEP(1);

            assert_signal_vector(46, o_bus_b, "value 1 retained");
            assert_signal_vector(i, o_bus_a, "next value properly set and read");
        }

    END_SIMULATION;

    TEST(register_bank, test_multi_register_reads)
        SET_SIGNAL(clk)
        SET_SIGNAL(setter);
        SET_SIGNAL_VECTOR(o_regmask_b);
        SET_SIGNAL_VECTOR(o_bus_b);
        SET_SIGNAL_VECTOR(o_regmask_a);
        SET_SIGNAL_VECTOR(o_bus_a);
        SET_SIGNAL_VECTOR(i_bus);
        SET_SIGNAL_VECTOR(i_regmask)
    START_SIMULATION;
        i_bus = 0;
        i_regmask = 0;
        clk = false;
        setter = true;
        o_regmask_a = 0b11111111;
        o_regmask_b = 0;

        unsigned int total_or = 0;
        for(int i=0;i<8;i++)
        {
            STEP(1);
            unsigned int next_value = 1 << i;
            o_regmask_b = next_value;
            i_regmask = next_value;
            i_bus = next_value;
            total_or = total_or | next_value;
            STEP(1);
            clk = true;
            STEP(1);
            clk = false;
            STEP(1);
            assert_signal_vector(next_value, o_bus_b, "checking whether value was properly set");
            assert_signal_vector(total_or, o_bus_a, "checking whether multi-line read on register output bus a is working");
        }
        i_bus = 0b10100000;
        i_regmask = 0b10000000;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        o_regmask_b = 0b10000111;
        o_regmask_a = i_regmask;
        STEP(1);
        assert_signal_vector(i_bus, o_bus_a, "checking single-register read on bus a");
        assert_signal_vector(0b10100111, o_bus_b, "multi-register read on bus b");
    END_SIMULATION;

END_TESTS;
