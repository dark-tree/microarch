
#include "Vflip_flop.h"
#include "../test_runner.h"

TESTS;

    TEST(flip_flop, test_manual_clock)
        SET_SIGNAL(clk)
        SET_SIGNAL(reset);
        SET_SIGNAL(D);
        SET_SIGNAL(Q);
    START_SIMULATION;
        clk = false;
        reset = false;

        D = true;
        STEP(10)
        assert_signal(false, Q, "bef edge 1");
        clk = true;
        STEP(10)
        clk = false;
        assert_signal(true, Q, "after edge 1");

        reset = true;
        STEP(1)
        reset = false;
        STEP(10)
        assert_signal(false, Q, "reset");
        STEP(10)
        assert_signal(false, Q, "bef edge 2");
        clk = true;
        STEP(1)
        assert_signal(true, Q, "after edge 2");
        D = false;

    END_SIMULATION;


    TEST(flip_flop, test)
        SET_CLOCK(clk, 10, 3);
        SET_SIGNAL(reset);
        SET_SIGNAL(D);
        SET_SIGNAL(Q);
    START_SIMULATION;
        D = false;
        reset = false;
        STEP(10)
        reset = true;
        STEP(1)
        reset = false;
        STEP(10)
        assert_signal(false, Q, "reset");
        D = true;
        for(int i = 0; i<5;i++)
        {
            STEP(1)
            if(clk == false)
            {
                assert_signal(false, Q, "bef edge");
            }
            else
            {
                assert_signal(true, Q, "after edge");
            }
        }
    END_SIMULATION;


END_TESTS;
