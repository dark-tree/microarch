module top
  #(
    parameter PROGRAM_HEX_FILE /*verilator public_flat_rw*/ = "../../executables/peripheal_test.hex"
  )
  (
    input clk_25mhz,
    input [6:0] btn,
    output[7:0] led,
    output wifi_gpio0
  );

  wire[7:0] gpio_a_ctr;
  wire[7:0] gpio_a_read;
  wire[7:0] gpio_a_write;
  assign wifi_gpio0 = 1'b1;
  assign led = gpio_a_write;
  reg always_one = 1'b0;
  
  reg i_clk = 1'b1;
  integer i = 0;

  microcontroller #(PROGRAM_HEX_FILE) mcu (
    .clk(i_clk),
    .interrupt_signal(btn[1]),
    .gpio_a_ctr(gpio_a_ctr),
    .gpio_a_read(gpio_a_read),
    .gpio_a_write(gpio_a_write)
  );





  always @(posedge clk_25mhz)
  begin
    i <= i+1;
    if(i >= 1) begin
      i_clk <= !i_clk;
      i <= 0;
    end
  end


endmodule
