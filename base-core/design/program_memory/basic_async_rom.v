module program_memory
  #(
    parameter PROGRAM_HEX_FILE=""
  )
  (
    output [23:0] output_bus,
    input [15:0] address,
    input clk,
    output instruction_ready
  );

  reg [23:0] data[0:40000];

  initial begin
    if(PROGRAM_HEX_FILE=="") begin
      $display("WARNING: No code file provided - initializing program memory to random values");
    end else begin
      $readmemh(PROGRAM_HEX_FILE, data, 0);
    end
  end

  assign instruction_ready = 1'b1;

  assign output_bus = data[address];


endmodule
