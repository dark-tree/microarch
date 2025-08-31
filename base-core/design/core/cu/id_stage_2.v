module id_stage_2
  (
    input[23:0] instruction,
    input mmu_ready,
    output instruction_ready,
    output instruction_finished,
    output setting_flags,
    output[7:0] registers_set
  );

  wire memory_instruction = (instruction[23:21] == 3'b001) ? 1'b1 : 1'b0;

  assign instruction_ready = (~memory_instruction) | mmu_ready;
  assign setting_flags = (instruction[23:20] == 4'b1111) ? 1'b1 : 1'b0;
  assign instruction_finished = (instruction[23:20] == 4'b0000) | ((instruction[23:20] == 4'b0011) & instruction_ready);
  assign registers_set = ((instruction[23] == 1'b1) | (instruction[23:20] == 4'b0010) | (instruction[23:20] == 4'b0001)) ? instruction[15:8] : ((instruction[23:20] == 4'b0110) ? 8'b11110000 : 8'b00000000);

endmodule
