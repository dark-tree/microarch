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

  wire[7:0] sum = in_a + in_b;
  // Here we use a vector that's 1 bit longer,
  // because we need carry (this is used for
  // the CMP operation, which sets flags)
  wire[8:0] difference = in_a - in_b;
  wire[7:0] and_result = in_a & in_b;
  wire[7:0] nand_result = ~and_result;
  wire[7:0] xor_result = in_a ^ in_b;
  wire[7:0] shift_result = in_a >> in_immediate;


  always @(posedge clk)
  begin
    if(execute == 1'b1) begin
      if(control == 3'b100) begin
        out <= in_b;
      end
      if(control == 3'b110) begin
        out <= sum;
      end
      if(control == 3'b111) begin
        out <= difference[7:0];
        cf <= difference[8];
        zf <= !| difference[7:0];
      end
      if(control == 3'b000) begin
        out <= nand_result;
      end
      if(control == 3'b001) begin
        out <= and_result;
      end
      if(control == 3'b010) begin
        out <= xor_result;
      end
      if(control == 3'b011) begin
        out <= shift_result;
      end
    end
  end

endmodule
