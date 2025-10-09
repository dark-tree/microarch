module core
  (
    input clk,
    output[7:0] data_bus_write,
    input[7:0] data_bus_read,
    output[7:0] data_address,
    output data_write_signal,
    output data_read_signal,
    input data_ready,
    input[23:0] instruction_bus,
    input instruction_ready,
    output[15:0] instruction_address,
    input interrupt_signal
  );

  wire[7:0] register_bus_a;
  wire[7:0] register_bus_b;
  wire[7:0] register_write_bus;

  wire[7:0] read_regmask_a;
  wire[7:0] read_regmask_b;
  wire[7:0] write_regmask;
  wire register_write_signal;
  wire trigger_cid;

  wire[7:0] alu_immediate;
  wire[2:0] alu_control;
  wire alu_signal;
  wire[7:0] accumulator;
  wire zf;
  wire cf;


  alu a (
    .in_a(register_bus_a),
    .in_b(register_bus_b),
    .in_immediate(alu_immediate),
    .control(alu_control),
    .execute(alu_signal),
    .zf(zf),
    .cf(cf),
    .out(accumulator),
    .clk(clk)
  );

  register_bank rb (
    .o_bus_a(register_bus_a),
    .o_bus_b(register_bus_b),
    .o_regmask_a(read_regmask_a),
    .o_regmask_b(read_regmask_b),
    .i_bus(register_write_bus),
    .i_regmask(write_regmask),
    .setter(register_write_signal),
    .clk(clk),
    .trigger_cid(trigger_cid)
  );

  wire mmu_signal;
  wire mmu_write;
  wire mmu_completed;
  wire mmu_pre_completed;
  wire mmu_operation_ongoing;

  wire[7:0] mmu_address = (mmu_write == 1'b1) ? register_bus_a : register_bus_b;
  wire[7:0] mmu_in_data = (mmu_write == 1'b1) ? register_bus_b : register_bus_a;
  wire[7:0] mmu_out_data;

  wire[15:0] interrupt_return_address;
  wire set_interrupt_return_address;

  mmu m (
    .write(mmu_write),
    .address(mmu_address),
    .in_data(mmu_in_data),
    .out_data(mmu_out_data),
    .execute(mmu_signal),
    .completed(mmu_completed),
    .interrupt_return_address(interrupt_return_address),
    .set_interrupt_return_address(set_interrupt_return_address),
    .memory_address(data_address),
    .memory_in(data_bus_write),
    .memory_out(data_bus_read),
    .memory_ready(data_ready),
    .memory_read_signal(data_read_signal),
    .memory_write_signal(data_write_signal),
    .pre_completed(mmu_pre_completed),
    .operation_ongoing(mmu_operation_ongoing),
    .clk(clk)
  );


  cu c (
    .program_counter(instruction_address),
    .instruction_bus(instruction_bus),
    .instruction_ready(instruction_ready),
    .register_read_bus_a(register_bus_a),
    .register_read_bus_b(register_bus_b),
    .read_regset_a(read_regmask_a),
    .read_regset_b(read_regmask_b),
    .alu_immediate(alu_immediate),
    .alu_control(alu_control),
    .alu_signal(alu_signal),
    .accumulator(accumulator),
    .zf(zf),
    .cf(cf),
    .register_write_bus(register_write_bus),
    .write_regset(write_regmask),
    .register_write_signal(register_write_signal),
    .mmu_signal(mmu_signal),
    .mmu_write(mmu_write),
    .mmu_ready(mmu_pre_completed),
    .mmu_data(mmu_out_data),
    .interrupt_return_address(interrupt_return_address),
    .set_interrupt_return_address(set_interrupt_return_address),
    .interrupt_signal_staging(interrupt_signal),
    .trigger_cid(trigger_cid),
    .clk(clk)
  );


endmodule
