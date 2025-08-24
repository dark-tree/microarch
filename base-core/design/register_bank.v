
module register_output_addressing_unit
  (
    input[7:0] register_values[7:0],
    input[7:0] regmask,
    output[7:0] out
  );

  wire [7:0] _out [7:0];

  genvar i;
  generate
    for(i=0; i<8;i=i+1) begin
      assign _out[i] = (regmask[i] == 1'b1) ? register_values[i] : 8'b00000000;
    end
  endgenerate
  assign out = _out[0] | _out[1] | _out[2] | _out[3] | _out[4] | _out[5] | _out[6] | _out[7];

endmodule


module register_bank
  (
    output[7:0] o_bus_a,
    input[7:0] o_regmask_a,
    output[7:0] o_bus_b,
    input[7:0] o_regmask_b,
    input[7:0] i_bus,
    input[7:0] i_regmask,
    input clk,
    input setter
  );

  reg[7:0] registers[7:0];

  integer i;

  initial begin
    for(i=0;i<8;i=i+1) begin
      registers[i] = 8'b00000000;
    end
  end

  register_output_addressing_unit roau_a (
    .register_values(registers),
    .regmask(o_regmask_a),
    .out(o_bus_a)
  );

  register_output_addressing_unit roau_b (
    .register_values(registers),
    .regmask(o_regmask_b),
    .out(o_bus_b)
  );

  always @(posedge clk)
  begin
    if(setter == 1'b1) begin
      for(i = 0; i<8;i=i+1) begin
        if(i_regmask[i] == 1'b1) begin
          registers[i] <= i_bus;
        end
      end
    end
  end

endmodule
