module alu
  (
    input[7:0] in_a,
    input[7:0] in_b,
    input[7:0] in_immediate,
    input[2:0] control,
    input execute,
    output reg zf = 1'b0,
    output reg cf = 1'b0,
    output reg[7:0] out = 8'b00000000,
    input clk
  );

  reg [2:0] _control;
  reg [7:0] alpha;
  reg [7:0] beta;
  reg [7:0] immediate;

  wire[7:0] sum = alpha + beta;
  // Here we use a vector that's 1 bit longer,
  // because we need carry (this is used for
  // the CMP operation, which sets flags)
  wire[8:0] difference = alpha - beta;
  wire[7:0] and_result = alpha & beta;
  wire[7:0] nand_result = ~and_result;
  wire[7:0] xor_result = alpha ^ beta;
  wire[7:0] shift_result = alpha >> immediate;


  always @(posedge clk)
  begin
    if(execute == 1'b1) begin
      _control <= control;
      alpha <= in_a;
      beta <= in_b;
      immediate <= in_immediate;
    end
    if(_control == 3'b100) begin
      out <= beta;
    end
    if(_control == 3'b110) begin
      out <= sum;
    end
    if(_control == 3'b111) begin
      out <= difference[7:0];
      cf <= difference[8];
      zf <= !| difference[7:0];
    end
    if(_control == 3'b000) begin
      out <= nand_result;
    end
    if(_control == 3'b001) begin
      out <= and_result;
    end
    if(_control == 3'b010) begin
      out <= xor_result;
    end
    if(_control == 3'b011) begin
      out <= shift_result;
    end
  end

endmodule
