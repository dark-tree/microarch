module program_memory
  (
    output [23:0] output_bus,
    input [15:0] address,
    input clk,
    output instruction_ready
  );

  reg [23:0] data[21844:0];

  initial begin
    data[0] = 24'b000111110000000101010101;
    data[1] = 24'b110011111111111100000001;
    data[2] = 24'b111011111000000011000000;
    data[3] = 24'b001111110000011010000000;
    data[4] = 24'b001011110000001000000110;
    data[5] = 24'b000111110001100000000000;
    data[6] = 24'b011011110000000000000000;
    data[7] = 24'b010011110001000000001000;
  end

  wire[15:0] act_address = address/3;

  assign instruction_ready = 1'b1;

  assign output_bus = data[act_address[14:0]];


endmodule;
