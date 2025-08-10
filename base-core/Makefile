
VERILOG_DESIGN_FILES = \
	design/top.v				\
	design/flip_flop.v

default: run

clean:
	@echo "-- Cleaning build directories --"
	rm -rf obj_dir/*
	rm -rf tracedump/*

waves:
	@echo "Launching GTK waveform visualizer"
	flatpak run io.github.gtkwave.GTKWave

run: build_main
	@echo "-- Running the microarch BaseCore SystemC main testbanch simulation --"
	@echo "Running the simulation..."
	obj_dir/Vtop +trace
	@echo "Launching GTK waveform visualizer"
	flatpak run io.github.gtkwave.GTKWave tracedump/traces.fst

unit_tests: build_tests
	@echo "-- Running the microarch BaseCore SystemC unit tests --"
	@echo "Running the simulation..."
	obj_dir/Vtop +trace

build_main: verilate_main
	@echo "-- Building microarch BaseCore testbench and verilated design --"
	@echo "Building from generated Makefile..."
	make -j -C obj_dir -f Vtop.mk

build_tests: verilate_tests
	@echo "-- Building microarch BaseCore testbench and verilated design --"
	@echo "Building from generated Makefile..."
	make -j -C obj_dir -f Vtop.mk

verilate_main:
	@echo "-- Verilating microarch BaseCore design simulation --"
	@echo "Generating object files..."
	@echo "Generating Makefile..."
	verilator -sc -exe -Wall --trace-fst $(VERILOG_DESIGN_FILES) --top top testbench/sc_main.cpp

verilate_tests:
	@echo "-- Verilating microarch BaseCore tests --"
	@echo "Generating object files..."
	@echo "Generating Makefile..."
	verilator -sc -exe -Wall --trace-fst $(VERILOG_DESIGN_FILES) --top top testbench/sc_unit_tests.cpp
