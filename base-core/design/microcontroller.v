module microcontroller
  (
    input clk,
    input interrupt_signal
  );

  wire[7:0] data_bus_write;
  wire[7:0] data_bus_read;
  wire[7:0] data_address;
  wire data_write_signal;
  wire data_read_signal;
  wire data_ready;
  wire[23:0] instruction_bus;
  wire instruction_ready;
  wire[15:0] instruction_address;

  core c (
    .clk(clk),
    .data_bus_write(data_bus_write),
    .data_bus_read(data_bus_read),
    .data_address(data_address),
    .data_write_signal(data_write_signal),
    .data_read_signal(data_read_signal),
    .data_ready(data_ready),
    .instruction_bus(instruction_bus),
    .instruction_ready(instruction_ready),
    .instruction_address(instruction_address),
    .interrupt_signal(interrupt_signal)
  );


  data_memory dm (
    .address(data_address),
    .in(data_bus_write),
    .out(data_bus_read),
    .clk(clk),
    .read_signal(data_read_signal),
    .write_signal(data_write_signal),
    .ready(data_ready)
  );


  program_memory pm (
    .output_bus(instruction_bus),
    .address(instruction_address),
    .instruction_ready(instruction_ready),
    .clk(clk)
  );




endmodule
