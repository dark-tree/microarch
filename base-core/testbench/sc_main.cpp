#include <systemc>
#include <verilated.h>
#include <verilated_fst_sc.h>
#include <memory>



#include "Vtop.h"

using namespace sc_core;
using namespace sc_dt;


int sc_main(int argc, char* argv[]) {
		Verilated::debug(0);			//Debug level
		Verilated::traceEverOn(true);		//We need to tell verilator, that tracing will be used
		Verilated::randReset(2); 		//Reset randomization policy
		Verilated::commandArgs(argc, argv); 	//Passing command args (required)

		sc_clock clk{
				"clk",	// Name
				10,			// Period
				SC_NS,	// Unit of the period
				0.5,		// Duty cycle
				3,			// Delay (time until first edge)
				SC_NS,	// Unit of delay
				true		// Is the rising edge first
		};

		const std::unique_ptr<Vtop> top{new Vtop{"top"}};
		sc_signal<bool> reset;
		sc_signal<bool> D;
		sc_signal<bool> Q;


		top->clk(clk);
		top->reset(reset);
		top->Q(Q);
		top->D(D);

		sc_start(SC_ZERO_TIME);

		auto tfp = new VerilatedFstSc;
		top->trace(tfp, 99); 	//Telling SystemC to trace up to 99 levels of hierarchy.
		tfp->open("tracedump/traces.fst");

		for(int i = 0; i<100;i++)
		{
				if (sc_time_stamp() > sc_time(28, SC_NS) && sc_time_stamp() < sc_time(30, SC_NS)) {
		            		reset = true;
		        	} else {
		            		reset = false;
		        	}
				if (sc_time_stamp() > sc_time(15, SC_NS) && sc_time_stamp() < sc_time(50, SC_NS)) {
		            		D = true;
		        	} else {
		            		D = false;
		        	}


				sc_start(1, SC_NS);


		}

		top->final();

		tfp->close();

		return 0;

}
