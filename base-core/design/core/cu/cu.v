module cu
  (
    output[15:0] program_counter,
    input[23:0] instruction_bus,
    input instruction_ready,
    input clk,
    input[7:0] register_read_bus_a,
    input[7:0] register_read_bus_b,
    output[7:0] read_regset_a,
    output[7:0] read_regset_b,
    output[7:0] alu_immediate,
    output alu_signal,
    input [7:0] accumulator,
    input zf,
    input cf,
    output[7:0] register_write_bus,
    output[7:0] write_regset,
    output register_write_signal,
    output mmu_signal,
    output mmu_write,
    input mmu_ready,
    input[7:0] mmu_data,
    output[15:0] interrupt_return_address,
    output set_interrupt_return_address,
    input interrupt_signal,
    output trigger_cid


  );

  wire[23:0] instruction_s1;
  wire[23:0] instruction_s2;
  wire[23:0] instruction_s3;


  wire[7:0] stage_1_register_dependencies;
  wire[7:0] stage_2_registers_set;
  wire[7:0] stage_3_registers_set;
  wire[7:0] upper_stages_registers_set = stage_2_registers_set | stage_3_registers_set;
  wire[7:0] pipeline_register_overlap = upper_stages_registers_set & stage_1_register_dependencies;
  wire register_overlap_warning = |pipeline_register_overlap;

  wire stage_2_setting_flags;
  wire stage_1_using_flags;

  wire flag_overlap_warning = stage_2_setting_flags & stage_1_using_flags;

  wire stage_1_hold = flag_overlap_warning | register_overlap_warning;

  wire execute_stage_1 = !stage_1_hold;
  wire stage_1_finished;

  wire stage_2_ready;
  wire stage_2_finished;

  wire ready_for_next_instruction;

  reg[15:0] pc = 16'b1111111111111101;
  reg[7:0] ctr = 8'b00000000;

  wire set_ctr;
  wire[7:0] ctr_value;

  always @(posedge clk)
  begin
    if(ready_for_next_instruction == 1'b1) begin
      pc <= program_counter;
    end
    if(set_ctr == 1'b1) begin
      ctr <= ctr_value;
    end
  end


  pipeline p (
    .stage_1_instruction(instruction_s1),
    .stage_2_instruction(instruction_s2),
    .stage_3_instruction(instruction_s3),
    .next_instruction(instruction_bus),
    .next_instruction_available(instruction_ready),
    .stage_2_ready(stage_2_ready),
    .stage_1_ready(execute_stage_1),
    .stage_1_finished(stage_1_finished),
    .stage_2_finished(stage_2_finished),
    .ready_for_next_instruction(ready_for_next_instruction),
    .clk(clk)
  );


  id_stage_1 id1 (
    .instruction(instruction_s1),
    .execute(execute_stage_1),
    .cf(cf),
    .zf(zf),
    .alu_write_signal(alu_signal),
    .regmask_a(read_regset_a),
    .regmask_b(read_regset_b),
    .alu_immediate(alu_immediate),
    .registers_used(stage_1_register_dependencies),
    .register_bus_a(register_read_bus_a),
    .register_bus_b(register_read_bus_b),
    .current_instruction_address(pc),
    .next_instruction_address(program_counter),
    .mmu_write(mmu_write),
    .mmu_execute(mmu_signal),
    .interrupt_signal(interrupt_signal),
    .set_interrupt_return_address(set_interrupt_return_address),
    .interrupt_return_address(interrupt_return_address),
    .instruction_finished(stage_1_finished),
    .flag_dependent(stage_1_using_flags),
    .set_ctr(set_ctr),
    .ctr_value(ctr_value)
  );

  id_stage_2 id2 (
    .instruction(instruction_s2),
    .mmu_ready(mmu_ready),
    .instruction_ready(stage_2_ready),
    .instruction_finished(stage_2_finished),
    .setting_flags(stage_2_setting_flags),
    .registers_set(stage_2_registers_set)
  );


  id_stage_3 id3 (
    .instruction(instruction_s3),
    .accumulator(accumulator),
    .mmu_data_register(mmu_data),
    .regset(write_regset),
    .set_registers(register_write_signal),
    .register_bus(register_write_bus),
    .registers_set(stage_3_registers_set),
    .trigger_cid(trigger_cid)
  );







endmodule
