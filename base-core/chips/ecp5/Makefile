
synth:
	mkdir -p build
	yosys microarch_synth.ys

implement: synth
	nextpnr-ecp5 --12k --json build/microarch.json --lpf constraints.lpf --textcfg build/microarch_out.config

bitstream: implement
	ecppack build/microarch_out.config build/microarch.bit

run: bitstream
	fujprog build/microarch.bit

clean:
	rm -rf build/*
