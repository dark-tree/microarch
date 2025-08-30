module pipeline
  (
    output reg[23:0] stage_1_instruction = 24'b000000000000000000000000,
    output reg[23:0] stage_2_instruction = 24'b000000000000000000000000,
    output reg[23:0] stage_3_instruction = 24'b000000000000000000000000,
    input[23:0] next_instruction,
    input next_instruction_available,
    output ready_for_next_instruction,
    input stage_2_ready,
    input stage_1_ready,
    input stage_1_finished,
    input stage_2_finished,
    input clk

  );

  wire[23:0] stage_1_input = next_instruction_available ? next_instruction : 24'b000000000000000000000000;
  wire[23:0] stage_2_input = (stage_1_finished | (!stage_1_ready)) ? 24'b000000000000000000000000 : stage_1_instruction;
  wire[23:0] stage_3_input = (stage_2_finished | (!stage_2_ready)) ? 24'b000000000000000000000000 : stage_2_instruction;

  wire advance_stage_2 = stage_2_ready;
  wire advance_stage_1 = (stage_2_ready | stage_1_finished) & stage_1_ready;

  assign ready_for_next_instruction = advance_stage_1;

  always @(posedge clk)
  begin
    stage_3_instruction <= stage_3_input;
    if(advance_stage_2 ==1'b1) begin
      stage_2_instruction <= stage_2_input;
    end
    if(advance_stage_1 == 1'b1) begin
      stage_1_instruction <= stage_1_input;
    end
  end

endmodule
