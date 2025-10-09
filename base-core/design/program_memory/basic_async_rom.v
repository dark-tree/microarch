module program_memory
  #(
    parameter PROGRAM_HEX_FILE
  )
  (
    output [23:0] output_bus,
    input [15:0] address,
    input clk,
    output instruction_ready
  );

  reg [23:0] data[0:21844];

  initial begin
    if(PROGRAM_HEX_FILE=="") begin
      $display("WARNING: No code file provided - initializing program memory to random values");
    end else begin
      $readmemh(PROGRAM_HEX_FILE, data, 0);
    end
  end

  wire[15:0] act_address = address/3;

  assign instruction_ready = 1'b1;

  assign output_bus = data[act_address[14:0]];


endmodule;
