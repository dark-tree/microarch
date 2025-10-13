module peripheal_control
  #(
    parameter IO_MEMORY_SPACE_SIZE=3
  )
  (
    input [7:0] io_registers_write[0:IO_MEMORY_SPACE_SIZE-1],
    output [7:0] io_registers_read[0:IO_MEMORY_SPACE_SIZE-1],
    input [7:0] gpio_a_read,
    output [7:0] gpio_a_write,
    output [7:0] gpio_a_ctr
  );

  assign gpio_a_ctr = io_registers_read[2];
  assign gpio_a_write = io_registers_read[1];

  genvar i;

  generate
    for(i=0; i<IO_MEMORY_SPACE_SIZE;i=i+1) begin
      assign io_registers_read[i] = (i == 0) ? gpio_a_read : io_registers_write[i];
    end
  endgenerate



endmodule
