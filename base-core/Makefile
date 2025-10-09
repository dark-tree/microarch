
VERILOG_DESIGN_FILES = 				\
	design/core/mmu.v						\
	design/core/alu.v						\
	design/core/register_bank.v	\
	design/core/cu/id_stage_1.v	\
	design/core/cu/id_stage_2.v	\
	design/core/cu/id_stage_3.v	\
	design/core/cu/pipeline.v		\
	design/core/cu/cu.v					\
	design/core/core.v					\
	design/microcontroller.v

DATA_MEMORY_SELECTED = design/data_memory/basic_synchronized_ram.v

PROGRAM_MEMORY_SELECTED = design/program_memory/basic_async_rom.v

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
	obj_dir/Vmicrocontroller +trace
	@echo "Launching GTK waveform visualizer"
	gtkwave tracedump/traces.fst

unit_tests:
	@echo "-- Running the microarch BaseCore SystemC unit tests --"
	@echo "Running the simulation..."
	./unit_tests.sh $(VERILOG_DESIGN_FILES) $(DATA_MEMORY_SELECTED) $(PROGRAM_MEMORY_SELECTED)

build_main: verilate_main
	@echo "-- Building microarch BaseCore testbench and verilated design --"
	@echo "Building from generated Makefile..."
	make -j -C obj_dir -f Vmicrocontroller.mk

verilate_main:
	@echo "-- Verilating microarch BaseCore design simulation --"
	@echo "Generating object files..."
	@echo "Generating Makefile..."
	verilator -sc -exe --trace-fst -GPROGRAM_HEX_FILE="\"$(TARGET)\"" $(VERILOG_DESIGN_FILES) $(DATA_MEMORY_SELECTED) $(PROGRAM_MEMORY_SELECTED) --top microcontroller testbench/sc_main.cpp
