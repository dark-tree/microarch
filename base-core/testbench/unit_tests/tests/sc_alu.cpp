#include "Valu.h"
#include "../test_runner.h"

TESTS;

    TEST(alu, test_add)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;

        control = 0b110;
        clk = false;
        execute = false;
        in_a = 5;
        in_b = 7;

        STEP(1);
        assert_signal_vector(0, out, "default value");
        execute = true;
        STEP(1);
        assert_signal_vector(0, out, "keeping default value after setting 'execute' to true");
        clk = true;
        STEP(1);
        assert_signal_vector(5+7, out, "calculation");
        clk = false;
        in_a = 128;
        in_b = 128;
        STEP(1);
        assert_signal_vector(5+7, out, "result staying after change in inputs");
        clk = true;
        STEP(1);
        assert_signal_vector(0, out, "overflowing calculation");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");
        STEP(1);

    END_SIMULATION;


    TEST(alu, test_cmp)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;

        control = 0b111;
        clk = false;
        execute = false;
        in_a = 5;
        in_b = 7;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(254, out, "calculation 1 (negative result)");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(true, cf, "checking if CF was set");

        execute = false;
        clk = false;
        in_a = 200;
        in_b = 200;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(254, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(true, cf, "checking if CF was retained after a clock cycle");

        execute = true;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0, out, "calculation 2 (result 0)");

        assert_signal(true, zf, "checking if ZF was set 2");
        assert_signal(false, cf, "checking if CF was not set 2");

        in_a = 200;
        in_b = 100;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(100, out, "calculation 3 (positive result)");

        assert_signal(false, zf, "checking if ZF was set 3");
        assert_signal(false, cf, "checking if CF was not set 3");


    END_SIMULATION;



    TEST(alu, test_mov)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;
        control = 0b100;
        clk = false;
        execute = false;
        in_a = 5;
        in_b = 218;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(5, out, "calculation 1 ");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");

        execute = false;
        clk = false;
        in_a = 231;
        in_b = 168;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(5, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(false, cf, "checking if CF was retained after a clock cycle");

        STEP(1);

    END_SIMULATION;


    TEST(alu, test_and)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;
        control = 0b001;
        clk = false;
        execute = false;
        in_a = 0b10101010;
        in_b = 0b11110000;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(0b10100000, out, "calculation");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");

        execute = false;
        clk = false;
        in_a = 231;
        in_b = 168;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0b10100000, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(false, cf, "checking if CF was retained after a clock cycle");

        STEP(1);

    END_SIMULATION;


    TEST(alu, test_nand)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;
        control = 0b000;
        clk = false;
        execute = false;
        in_a = 0b10101010;
        in_b = 0b11110000;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(0b01011111, out, "calculation");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");

        execute = false;
        clk = false;
        in_a = 231;
        in_b = 168;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0b01011111, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(false, cf, "checking if CF was retained after a clock cycle");

        STEP(1);

    END_SIMULATION;


    TEST(alu, test_xor)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;
        control = 0b010;
        clk = false;
        execute = false;
        in_a = 0b10101010;
        in_b = 0b11110000;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(0b01011010, out, "calculation");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");

        execute = false;
        clk = false;
        in_a = 231;
        in_b = 168;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0b01011010, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(false, cf, "checking if CF was retained after a clock cycle");

        STEP(1);

    END_SIMULATION;

    TEST(alu, test_shift)
        SET_SIGNAL(clk)
        SET_SIGNAL(execute);
        SET_SIGNAL(zf);
        SET_SIGNAL(cf);
        SET_SIGNAL_VECTOR(in_a);
        SET_SIGNAL_VECTOR(in_b);
        SET_SIGNAL_VECTOR(out);
        SET_SIGNAL_VECTOR(control);
    START_SIMULATION;
        control = 0b011;
        clk = false;
        execute = false;
        in_a = 0b10100000;
        in_b = 4;

        STEP(1);
        execute = true;
        clk = true;
        STEP(1);
        assert_signal_vector(0b00001010, out, "calculation 1");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");

        execute = false;
        clk = false;
        in_b = 168;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0b00001010, out, "checking if value was retained after a clock cycle");
        assert_signal(false, zf, "checking if ZF was retained after a clock cycle");
        assert_signal(false, cf, "checking if CF was retained after a clock cycle");
        execute= true;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0, out, "calculation 2");
        assert_signal(false, zf, "checking if ZF was not set");
        assert_signal(false, cf, "checking if CF was not set");
    END_SIMULATION;

END_TESTS;
