
VERILOG_DESIGN_FILES = \
	design/flip_flop.v	\
	design/alu.v

DATA_MEMORY_SELECTED = design/basic_synchronized_ram.v

default: run

clean:
	@echo "-- Cleaning build directories --"
	rm -rf obj_dir/*
	rm -rf tracedump/*

waves:
	@echo "Launching GTK waveform visualizer"
	gtkwave

run: build_main
	@echo "-- Running the microarch BaseCore SystemC main testbanch simulation --"
	@echo "Running the simulation..."
	obj_dir/Valu +trace
	@echo "Launching GTK waveform visualizer"
	gtkwave tracedump/traces.fst

unit_tests:
	@echo "-- Running the microarch BaseCore SystemC unit tests --"
	@echo "Running the simulation..."
	./unit_tests.sh $(VERILOG_DESIGN_FILES) $(DATA_MEMORY_SELECTED)

build_main: verilate_main
	@echo "-- Building microarch BaseCore testbench and verilated design --"
	@echo "Building from generated Makefile..."
	make -j -C obj_dir -f Valu.mk

verilate_main:
	@echo "-- Verilating microarch BaseCore design simulation --"
	@echo "Generating object files..."
	@echo "Generating Makefile..."
	verilator -sc -exe -Wall --trace-fst $(VERILOG_DESIGN_FILES) $(DATA_MEMORY_SELECTED) --top alu testbench/sc_main.cpp
