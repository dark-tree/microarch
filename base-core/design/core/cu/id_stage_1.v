module id_stage_1
  (
    input[23:0] instruction,
    input execute,
    input cf,
    input zf,
    output alu_write_signal,
    output[7:0] regmask_a,
    output[7:0] regmask_b,
    input[7:0] register_bus_a,
    input[7:0] register_bus_b,
    output[7:0] ctr_value,
    output set_ctr,
    input[15:0] current_instruction_address,
    output[15:0] next_instruction_address,
    output flag_dependent,
    output mmu_write,
    output mmu_execute,
    input interrupt_signal,
    output set_interrupt_return_address,
    output[15:0] interrupt_return_address,
    output instruction_finished
  );

  wire staging_alu_write_signal = (instruction[23] == 1'b1) ? 1'b1 : 1'b0;

  assign regmask_a = instruction[15:8];
  assign regmask_b = instruction[7:0];

  assign ctr_value = instruction[15:8];

  assign mmu_write = instruction[20];
  wire  staging_mmu_execute = (instruction[23:21] == 3'b001) ? 1'b1 : 1'b0;

  assign set_interrupt_return_address = interrupt_signal;

  wire[15:0] address_from_registers;
  assign address_from_registers[7:0] = register_bus_a;
  assign address_from_registers[15:8] = register_bus_b;

  wire [15:0] immediate_address;
  assign immediate_address [7:0] = instruction[15:8];
  assign immediate_address [15:8] = instruction[7:0];

  wire [15:0] jump_address = (instruction[20] == 1'b1) ? immediate_address : address_from_registers;


  wire staging_set_ctr = (instruction[23:20] == 4'b0111) ? 1'b1 : 1'b0;

  wire zf_comp = (zf & instruction[19]) | (!zf & instruction[18]);
  wire cf_comp = (cf & instruction[17]) | (!cf & instruction[16]);

  wire flags = zf_comp & cf_comp;

  assign flag_dependent = (instruction[19] ^ instruction[18]) | (instruction[17] ^ instruction[16]);

  wire actually_execute = flags & execute;

  assign alu_write_signal = actually_execute & staging_alu_write_signal;
  assign mmu_execute = actually_execute & staging_mmu_execute;
  assign set_ctr = actually_execute & staging_set_ctr;


  wire[15:0] next_address_staging = ((actually_execute == 1'b1) & (instruction[23:21] == 3'b010)) ? jump_address : (current_instruction_address + 3);
  assign interrupt_return_address = next_address_staging;
  assign next_instruction_address = (interrupt_signal == 1'b1) ? 16'b0000000000000000 : next_address_staging;

  assign instruction_finished = ((instruction[23:21] == 3'b010) | (instruction[23:20] == 4'b0111)) | ( !actually_execute | (instruction[23:20] == 4'b0000));
endmodule
