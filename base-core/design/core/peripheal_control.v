module peripheal_control
  #(
    parameter IO_MEMORY_SPACE_SIZE=3
  )
  (
    input io_registers_write,
    input io_registers_read,
    output reg io_registers_ready = 1'b1,
    input [7:0] io_registers_write_bus,
    output reg [7:0] io_registers_read_bus = 8'b00000000,
    input [7:0] gpio_a_read,
    output reg [7:0] gpio_a_write = 8'b00000000,
    output reg [7:0] gpio_a_ctr = 8'b00000000,
    input [3:0] io_registers_address,
    input clk
  );


  always @(posedge clk)
  begin
    io_registers_ready <= 1'b0;
    if(io_registers_read == 1'b1) begin
      if(io_registers_address == 4'b0000) begin
        io_registers_read_bus <= gpio_a_read;
        io_registers_ready <= 1'b1;
      end
      if(io_registers_address == 4'b0001) begin
        io_registers_read_bus <= gpio_a_write;
        io_registers_ready <= 1'b1;
      end
      if(io_registers_address == 4'b0010) begin
        io_registers_read_bus <= gpio_a_ctr;
        io_registers_ready <= 1'b1;
      end
    end
    if(io_registers_write == 1'b1) begin
      if(io_registers_address == 4'b0001) begin
        gpio_a_write <= io_registers_write_bus;
        io_registers_ready <= 1'b1;
      end
      if(io_registers_address == 4'b0010) begin
        gpio_a_ctr <= io_registers_write_bus;
        io_registers_ready <= 1'b1;
      end
    end
  end

endmodule
