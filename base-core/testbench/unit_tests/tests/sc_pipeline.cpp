#include "Vpipeline.h"
#include "../test_runner.h"

TESTS;

    TEST(pipeline, example_pipeline)
        SET_SIGNAL(clk);
        SET_SIGNAL(stage_2_finished);
        SET_SIGNAL(stage_1_finished);
        SET_SIGNAL(stage_1_ready);
        SET_SIGNAL(stage_2_ready);
        SET_SIGNAL(ready_for_next_instruction);
        SET_SIGNAL(next_instruction_available);
        SET_SIGNAL_VECTOR(next_instruction);
        SET_SIGNAL_VECTOR(stage_1_instruction);
        SET_SIGNAL_VECTOR(stage_2_instruction);
        SET_SIGNAL_VECTOR(stage_3_instruction);
    START_SIMULATION;
        stage_2_ready = true;
        stage_1_ready = true;
        stage_1_finished = true;
        stage_2_finished = true;
        clk = false;
        next_instruction_available = true;
        next_instruction = 5000;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(5000, stage_1_instruction, "stage_1_instruction, first cycle");
        assert_signal_vector(0, stage_2_instruction, "stage_2_instruction, first_cycle");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction, first_cycle");
        stage_1_finished = false;
        next_instruction = 2000;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(2000, stage_1_instruction, "stage_1_instruction, second cycle");
        assert_signal_vector(5000, stage_2_instruction, "stage_2_instruction, second_cycle");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction, second_cycle");

        stage_2_finished = false;
        // Lets' say we're waiting for a memory operation to complete
        stage_2_ready = false;
        // Let's say we're waiting until instruction 5000 sets a register we need to read.
        stage_1_ready = false;

        STEP(1);

        assert_signal(false, ready_for_next_instruction, "stage 1 is busy, so we can't accept instructions");

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        // Waiting a few more clock cycles for stage 2 to finish

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        stage_2_ready = true;

        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(false, ready_for_next_instruction, "stage 1 is busy, so we can't accept instructions");
        assert_signal_vector(2000, stage_1_instruction, "stage_1_instruction still waiting");
        assert_signal_vector(0, stage_2_instruction, "stage_2_instruction empty, because 1 wasn't ready");
        assert_signal_vector(5000, stage_3_instruction, "stage_3_instruction");


        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(false, ready_for_next_instruction, "stage 1 is busy, so we can't accept instructions");
        assert_signal_vector(2000, stage_1_instruction, "stage_1_instruction still waiting");
        assert_signal_vector(0, stage_2_instruction, "stage_2_instruction empty, because 1 wasn't ready");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        stage_1_ready = true;
        STEP(1);
        assert_signal(true, ready_for_next_instruction, "stage 1 will be free now");
        next_instruction = 900;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(900, stage_1_instruction, "stage_1_instruction - new instruction 900");
        assert_signal_vector(2000, stage_2_instruction, "stage_2_instruction ");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");

        stage_1_finished = true;
        stage_2_ready = false;

        STEP(1);

        next_instruction = 61000;
        assert_signal(true, ready_for_next_instruction, "stage 1 will be free now, because even though it cannot move on to stage 2 (it's busy) that insturction has finished and can be deleted");

        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(61000, stage_1_instruction, "stage_1_instruction - new instruction 61000");
        assert_signal_vector(2000, stage_2_instruction, "stage_2_instruction still waiting and blocking ");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");

        stage_2_ready = true;
        stage_1_ready = true;
        stage_1_finished = false;
        next_instruction_available = false;
        stage_2_finished = true;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(0, stage_1_instruction, "stage_1_instruction no new instruction, so assuming NOP");
        assert_signal_vector(61000, stage_2_instruction, "stage_2_instruction - moved on ");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty because 2000 finished at stage 2");

        stage_2_finished = false;
        stage_2_ready = false;
        next_instruction_available = true;
        stage_1_finished = true;
        next_instruction = 15000;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);

        assert_signal_vector(15000, stage_1_instruction, "stage_1_instruction - NOP was overriden by the new instruction");
        assert_signal_vector(61000, stage_2_instruction, "stage_2_instruction - still waiting ");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        stage_1_ready = false;
        stage_1_finished = false;
        stage_2_ready = true;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        stage_2_finished = false;
        assert_signal(false, ready_for_next_instruction, "not ready, becuase stage 1 is waiting for stage 3");
        assert_signal_vector(15000, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(0, stage_2_instruction, "stage_2_instruction - previous instruction not ready, so we assume NOP");
        assert_signal_vector(61000, stage_3_instruction, "stage_3_instruction empty");
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(15000, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(0, stage_2_instruction, "stage_2_instruction - previous instruction not ready, so we assume NOP");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        // Let's assume that instruction 61000 wrote to the register 1 was waiting on, so now 1 is ready to proceed.
        stage_1_ready = true;
        STEP(1);
        assert_signal(true, ready_for_next_instruction, "ready");
        next_instruction = 10000;
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(10000, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(15000, stage_2_instruction, "stage_2_instruction");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        // Waiting a few cycles for stage 2 to finish a memory operation
        stage_2_ready = false;
        stage_2_finished = false;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal(false, ready_for_next_instruction, "not ready, becuase stage 1 is waiting for stage 2");
        assert_signal_vector(10000, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(15000, stage_2_instruction, "stage_2_instruction");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        stage_2_ready = true;
        stage_2_finished = true;
        next_instruction = 1000;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(1000, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(10000, stage_2_instruction, "stage_2_instruction");
        assert_signal_vector(0, stage_3_instruction, "stage_3_instruction empty");
        stage_2_finished = false;
        next_instruction = 100;
        STEP(1);
        clk = true;
        STEP(1);
        clk = false;
        STEP(1);
        assert_signal_vector(100, stage_1_instruction, "stage_1_instruction");
        assert_signal_vector(1000, stage_2_instruction, "stage_2_instruction");
        assert_signal_vector(10000, stage_3_instruction, "stage_3_instruction");
    END_SIMULATION;


END_TESTS;
