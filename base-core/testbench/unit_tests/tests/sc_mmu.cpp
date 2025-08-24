#include "Vmmu.h"
#include "../test_runner.h"

TESTS;

    TEST(mmu, consecutive_read_and_write)
        SET_SIGNAL(clk);
        SET_SIGNAL(execute);
        SET_SIGNAL(write);
        SET_SIGNAL(completed);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(memory_read_signal);
        SET_SIGNAL(memory_write_signal);
        SET_SIGNAL(operation_ongoing);
        SET_SIGNAL(memory_ready);
        SET_SIGNAL(pre_completed);
        SET_SIGNAL_VECTOR(address);
        SET_SIGNAL_VECTOR(in_data);
        SET_SIGNAL_VECTOR(out_data);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(memory_address);
        SET_SIGNAL_VECTOR(memory_in);
        SET_SIGNAL_VECTOR(memory_out);
    START_SIMULATION;

        execute = false;
        clk = false;
        write = false;
        memory_ready = false;
        set_interrupt_return_address = false;
        in_data = 0;
        interrupt_return_address = 0;
        address = 0;
        STEP(1);
        assert_signal(false, memory_read_signal, "memory read false when idle");
        assert_signal(false, memory_write_signal, "memory write false when idle");
        execute = true;
        address = 78;
        STEP(1);
        clk = true;
        STEP(1);

        clk = false;
        memory_ready = true;
        STEP(1);
        assert_signal(true, memory_read_signal, "memory read signal properly set");
        assert_signal(false, completed, "properly waiting on memory to report ready");
        assert_signal(false, memory_write_signal, "memory write signal not set when reading");
        assert_signal(true, pre_completed, "pre-completed signal indicated" );
        assert_signal_vector(78, memory_address, "properly set read address");

        memory_out = 97;

        write = true;
        in_data = 32;

        STEP(1);

        clk = true;
        STEP(1);
        assert_signal(true, completed, "completed signaling data latched in");
        assert_signal_vector(97, out_data, "proper read");
        memory_ready = false;
        clk = false;
        STEP(1);
        assert_signal(false, memory_read_signal, "memory read signal not set while writing");
        assert_signal(true, memory_write_signal, "memory write signal properly set");
        assert_signal_vector(32, memory_in, "proper write");
        assert_signal(false, pre_completed, "pre-completed signal turned off" );

        STEP(1);

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(false, pre_completed, "properly waiting until end of the long write" );
        assert_signal(false, completed, "properly waiting until end of the long write" );
        memory_ready = true;
        STEP(1);
        assert_signal(true, pre_completed, "pre_completed indicating end of write" );
        assert_signal(false, completed, "completed signal waiting for clock" );

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(true, completed, "completed signalling proper write");


    END_SIMULATION;



    TEST(mmu, reading_and_writing_over_interrupt_address_space)
        SET_SIGNAL(clk);
        SET_SIGNAL(execute);
        SET_SIGNAL(write);
        SET_SIGNAL(completed);
        SET_SIGNAL(set_interrupt_return_address);
        SET_SIGNAL(memory_read_signal);
        SET_SIGNAL(memory_write_signal);
        SET_SIGNAL(operation_ongoing);
        SET_SIGNAL(memory_ready);
        SET_SIGNAL(pre_completed);
        SET_SIGNAL_VECTOR(address);
        SET_SIGNAL_VECTOR(in_data);
        SET_SIGNAL_VECTOR(out_data);
        SET_SIGNAL_VECTOR(interrupt_return_address);
        SET_SIGNAL_VECTOR(memory_address);
        SET_SIGNAL_VECTOR(memory_in);
        SET_SIGNAL_VECTOR(memory_out);
    START_SIMULATION;


        clk = false;
        write = true;
        memory_ready = false;
        set_interrupt_return_address = false;
        in_data = 123;
        interrupt_return_address = 0;
        address = 0;
        execute = true;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(false, memory_read_signal, "memory read signal not set");
        assert_signal(false, memory_write_signal, "memory write signal not set");

        write = false;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(true, completed, "completed signaling data latched in");
        assert_signal_vector(123, out_data, "proper read of the space written through instruction");


        // Interrupt should take precenence over writes through isntruction.
        write = true;
        address = 1;
        in_data = 49;

        set_interrupt_return_address = true;
        interrupt_return_address = 0xF37D;
        STEP(1);
        assert_signal(false, memory_read_signal, "memory read signal not set");
        assert_signal(false, memory_write_signal, "memory write signal not set");


        STEP(1);

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        write = false;
        address = 0;


        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(0xF3, out_data, "reading older interrupt byte");
        assert_signal(true, completed, "completed signal set" );
        address = 1;
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(0x7D, out_data, "reading younger interrupt byte");
        assert_signal(true, completed, "completed signal set" );
        assert_signal(false, operation_ongoing, "making sure 'operation_ongoing' isnt left lingering");

    END_SIMULATION;



END_TESTS;
