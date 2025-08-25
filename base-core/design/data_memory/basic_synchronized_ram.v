module data_memory
  (
    input[7:0] address,
    input[7:0] in,
    output reg[7:0] out,
    input clk,
    input read_signal,
    input write_signal,
    output ready
  );

  reg[7:0] memory[255:0];
  reg[7:0] last_operation_address;

  always @(posedge clk)
  begin
    if(write_signal == 1'b1) begin
      memory[address] <= in;
      last_operation_address <= address;
    end
    if(read_signal == 1'b1) begin
      out <= memory[address];
      last_operation_address <= address;
    end
  end

  // Signalling that the operation has finished,
  // if the address has not changed since the
  // last operation.
  assign ready = !|(address ^ last_operation_address);


endmodule
