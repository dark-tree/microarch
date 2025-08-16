#include <systemc>
#include <verilated.h>
#include <verilated_fst_sc.h>
#include <memory>



#include "Valu.h"

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

		const std::unique_ptr<Valu> top{new Valu{"alu"}};
		sc_signal<bool> execute;
		sc_signal<bool> zf;
		sc_signal<bool> cf;
		sc_signal<unsigned int> in_a;
		sc_signal<unsigned int> in_b;
		sc_signal<unsigned int> out;
		sc_signal<unsigned int> control;

		top->clk(clk);
		top->zf(zf);
		top->execute(execute);
		top->cf(cf);
		top->in_a(in_a);
		top->in_b(in_b);
		top->out(out);
		top->control(control);
		sc_start(SC_ZERO_TIME);

		auto tfp = new VerilatedFstSc;
		top->trace(tfp, 99); 	//Telling SystemC to trace up to 99 levels of hierarchy.
		tfp->open("tracedump/traces.fst");

		control = 0b110;
		in_a = 5;
		in_b = 9;
		execute = 0;

		for(int i = 0; i<100;i++)
		{
				if (sc_time_stamp() > sc_time(28, SC_NS) && sc_time_stamp() < sc_time(30, SC_NS)) {
						execute = 1;
				}

				sc_start(1, SC_NS);


		}

		top->final();

		tfp->close();

		return 0;

}
