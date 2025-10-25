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

  wire[17:0] current_control_signal;

  assign current_control_signal[17:10] = address;
  assign current_control_signal[9:2] = in;
  assign current_control_signal[1] = read_signal;
  assign current_control_signal[0] = write_signal;

  reg[17:0] last_control_signal = 18'b000000000000000000;


  always @(posedge clk)
  begin
    if(write_signal == 1'b1) begin
      memory[address] <= in;
    end
    if(read_signal == 1'b1) begin
      out <= memory[address];
    end
    last_control_signal <= current_control_signal;
  end

  // Signalling that the operation has finished,
  // if the address and operation type has not
  // changed since the last operation.
  // For writes we also check the written data.
  assign ready = (!(|(current_control_signal ^ last_control_signal)));


endmodule
