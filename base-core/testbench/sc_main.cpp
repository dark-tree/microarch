#include <systemc>
#include <verilated.h>
#include <verilated_fst_sc.h>
#include <memory>



#include "Vcu.h"

using namespace sc_core;
using namespace sc_dt;


#define SET_SIGNAL(name)	\
		sc_signal<bool>name;	\
		top->name(name);


#define SET_SIGNAL_VECTOR(name)	\
		sc_signal<unsigned int>name;	\
		top->name(name);


#define STEP(time)	\
		sc_start(time, SC_NS);


int sc_main(int argc, char* argv[]) {
		Verilated::debug(0);			//Debug level
		Verilated::traceEverOn(true);		//We need to tell verilator, that tracing will be used
		Verilated::randReset(2); 		//Reset randomization policy
		Verilated::commandArgs(argc, argv); 	//Passing command args (required)
		//
		// sc_clock clk{
		// 		"clk",	// Name
		// 		10,			// Period
		// 		SC_NS,	// Unit of the period
		// 		0.5,		// Duty cycle
		// 		3,			// Delay (time until first edge)
		// 		SC_NS,	// Unit of delay
		// 		true		// Is the rising edge first
		// };

		const std::unique_ptr<Vcu> top{new Vcu{"cu"}};

		SET_SIGNAL_VECTOR(program_counter);
		SET_SIGNAL_VECTOR(instruction_bus);
		SET_SIGNAL(instruction_ready);
		SET_SIGNAL(clk);
		SET_SIGNAL_VECTOR(register_read_bus_a);
		SET_SIGNAL_VECTOR(register_read_bus_b);
		SET_SIGNAL_VECTOR(read_regset_a);
		SET_SIGNAL_VECTOR(read_regset_b);
		SET_SIGNAL_VECTOR(alu_immediate);
		SET_SIGNAL(alu_signal);
		SET_SIGNAL_VECTOR(accumulator);
		SET_SIGNAL(zf);
		SET_SIGNAL(cf);
		SET_SIGNAL_VECTOR(register_write_bus);
		SET_SIGNAL_VECTOR(write_regset);
		SET_SIGNAL(register_write_signal);
		SET_SIGNAL(mmu_signal);
		SET_SIGNAL(mmu_write);
		SET_SIGNAL(mmu_ready);
		SET_SIGNAL_VECTOR(mmu_data);
		SET_SIGNAL_VECTOR(interrupt_return_address);
		SET_SIGNAL(set_interrupt_return_address);
		SET_SIGNAL(interrupt_signal);
		SET_SIGNAL(trigger_cid);

		sc_start(SC_ZERO_TIME);

		auto tfp = new VerilatedFstSc;
		top->trace(tfp, 99); 	//Telling SystemC to trace up to 99 levels of hierarchy.
		tfp->open("tracedump/traces.fst");

		clk = false;
		instruction_ready = true;
		zf = true;
		cf = true;
		register_read_bus_a = 27;
		register_read_bus_b = 0;
		unsigned int instructions[] = {
				0b111011110100000010000000,	//add
				0b001111110000011010000000,	//stm
				0b111111110001000000000001,	//cmp
				0b000110101000000000111001,	//set
				0b111011111000000010001000,	//add
				0b010011110000000101000000,	//jpr
				0b111011110100000010000000,	//add (should not execute)
				0,
				0,
				0b000110101000000000111001,	//set
				0,
		};
		unsigned int accumulator_values[] = {
				0,
				0,
				49,
				49,
				18,
				18,
				98,
				0,
				0,
				0,
				0,
		};

		bool mmu_ready_values[] = {
				false,
				false,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
		};

		unsigned int mmu_data_values[] =
		{
				0,
				0,
				0,
				17,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
		};

		mmu_ready = true;

		int step = 0;
		while(step < sizeof(instructions) / sizeof(unsigned int))
		{

				step = program_counter/3;
				instruction_bus = instructions[step];
				if (sc_time_stamp() > sc_time(3, SC_NS) && sc_time_stamp() < sc_time(8, SC_NS)) {

						mmu_ready = false;
				}
				else
				{
						mmu_ready = true;
				}
				mmu_data = mmu_data_values[step];
				accumulator = accumulator_values[step];
				STEP(1);
				clk = true;
				STEP(1);

				clk = false;
		}

		STEP(1);
		top->final();

		tfp->close();

		return 0;

}
