

default: run

run: verilate build
	@echo "-- Running the microarch BaseCore SystemC testbench  --"
	@echo "Running the simulation..."
	obj_dir/Vtop +trace
	@echo "Launching GTK waveform visualizer"
	flatpak run io.github.gtkwave.GTKWave tracedump/traces.fst

verilate:
	@echo "-- Verilating microarch BaseCore design --"
	@echo "Generating object files..."
	@echo "Generating Makefile..."
	verilator -sc -exe -Wall --trace-fst design/top.v testbench/sc_main.cpp

build:
	@echo "-- Building microarch BaseCore testbench and verilated design --" 
	@echo "Building from generated Makefile..."	
	make -j -C obj_dir -f Vtop.mk 
