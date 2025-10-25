


module register_bank
  (
    output[7:0] o_bus_a,
    input[7:0] o_regmask_a,
    output[7:0] o_bus_b,
    input[7:0] o_regmask_b,
    input[7:0] i_bus,
    input[7:0] i_regmask,
    input clk,
    input setter,
    input trigger_cid
  );

  reg[7:0] registers[7:0];

  integer j;

  initial begin
    for(j=0;j<8;j=j+1) begin
      registers[j] = 8'b00000000;
    end
  end

  wire [7:0] _out_a [7:0];
  wire [7:0] _out_b [7:0];

  genvar i;
  generate
    for(i=0; i<8;i=i+1) begin
      assign _out_a[i] = (o_regmask_a[i] == 1'b1) ? registers[i] : 8'b00000000;
      assign _out_b[i] = (o_regmask_b[i] == 1'b1) ? registers[i] : 8'b00000000;
    end
  endgenerate

  assign o_bus_a = _out_a[0] | _out_a[1] | _out_a[2] | _out_a[3] | _out_a[4] | _out_a[5] | _out_a[6] | _out_a[7];
  assign o_bus_b = _out_b[0] | _out_b[1] | _out_b[2] | _out_b[3] | _out_b[4] | _out_b[5] | _out_b[6] | _out_b[7];

  always @(posedge clk)
  begin
    if(setter == 1'b1) begin
      if(i_regmask[0] == 1'b1) begin
        registers[0] <= i_bus;
      end
      if(i_regmask[1] == 1'b1) begin
        registers[1] <= i_bus;
      end
      if(i_regmask[2] == 1'b1) begin
        registers[2] <= i_bus;
      end
      if(i_regmask[3] == 1'b1) begin
        registers[3] <= i_bus;
      end
      if(i_regmask[4] == 1'b1) begin
        registers[4] <= i_bus;
      end
      if(i_regmask[5] == 1'b1) begin
        registers[5] <= i_bus;
      end
      if(i_regmask[6] == 1'b1) begin
        registers[6] <= i_bus;
      end
      if(i_regmask[7] == 1'b1) begin
        registers[7] <= i_bus;
      end
    end
    if(trigger_cid == 1'b1) begin
      registers[0] <=  8'b00000000;
      registers[1] <=  8'b00000000;
      registers[2] <=  8'b00000000;
      registers[3] <=  8'b00000000;
    end
  end

endmodule
