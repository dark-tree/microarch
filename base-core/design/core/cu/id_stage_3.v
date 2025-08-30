module id_stage_3
  (
    input[23:0] instruction,
    input[7:0] accumulator,
    input[7:0] mmu_data_register,
    output[7:0] regset,
    output set_registers,
    output[7:0] register_bus,
    output trigger_cid,
    output[7:0] registers_set
  );

  assign trigger_cid = (instruction[23:20] == 4'b0110) ? 1'b1 : 1'b0;
  assign register_bus = (instruction[23] == 1'b1) ? accumulator : ((instruction[23:20] == 4'b0010) ? mmu_data_register : instruction[7:0]);
  assign set_registers = (instruction[23] == 1'b1) | (instruction[23:20] == 4'b0010) | (instruction[23:20] == 4'b0001);

  wire[7:0] _regset = (instruction[23:20] == 4'b0010) ? instruction[7:0] : instruction[15:8];
  assign regset = _regset;
  assign registers_set = (set_registers) ? _regset : ((trigger_cid) ? 8'b11110000 : 8'b00000000);
endmodule
