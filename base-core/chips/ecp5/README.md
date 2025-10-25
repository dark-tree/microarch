#### Requirements:

- ```yosys```
- ```nextpnr-ecp5```
- ```ecppack```
- ```make```
- ```fujprog``` (if your board is ULX3S - for other boards, another programmer will be required)

#### Running:

To synthesize:

```make synth```

To implement (place and route):

```make implement```

To generate bitstream:

```make bitstream```

To run on the hardware:

```make run``` 
