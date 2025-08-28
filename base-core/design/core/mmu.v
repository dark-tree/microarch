module mmu
  (
    // CPU-side controls.
    input write, // 0 - memory_read, 1 - write
    input[7:0] address,
    input[7:0] in_data,
    output reg[7:0] out_data,
    input execute,
    output reg completed = 1'b0,
    input clk,
    input[15:0] interrupt_return_address,
    input set_interrupt_return_address,
    // Memory-side controls, that shall be connected to the memory bus.
    output reg[7:0] memory_address,
    output reg[7:0] memory_in,
    output reg memory_read_signal = 1'b0,
    output reg memory_write_signal = 1'b0,
    input[7:0] memory_out,
    input memory_ready,
    output pre_completed,
    output reg operation_ongoing = 1'b0
  );

  reg [7:0] spec_mem_interrupt[1:0];



  always @(posedge clk)
  begin

    if(operation_ongoing == 1'b1) begin
      if(memory_ready == 1'b1) begin
        out_data <= memory_out;
        completed <= 1'b1;
        operation_ongoing <= 1'b0;
        memory_read_signal <= 1'b0;
        memory_write_signal <= 1'b0;
      end else begin
        completed <= 1'b0;
      end
    end

    if(execute == 1'b1) begin
      memory_address <= address;
      if(write == 1'b1) begin
        if(|address[7:1] == 1'b0) begin
          spec_mem_interrupt[address[0]] <= in_data;
          completed <= 1'b1;
        end else begin
          memory_write_signal <= 1'b1;
          memory_in <= in_data;
          operation_ongoing <= 1'b1;
        end
      end else begin
        if(|address[7:1] == 1'b0) begin
          out_data <= spec_mem_interrupt[address[0]];
          completed <= 1'b1;
        end else begin
          memory_read_signal <= 1'b1;
          operation_ongoing <= 1'b1;
        end
      end
    end

    if((execute | operation_ongoing) == 0'b0) begin
      completed <= 0'b0;
    end

    if(set_interrupt_return_address == 1'b1) begin
      spec_mem_interrupt[0] <= interrupt_return_address[15:8];
      spec_mem_interrupt[1] <= interrupt_return_address[7:0];
    end

  end



assign pre_completed = (operation_ongoing == 1'b1) ? memory_ready : 1'b0;

endmodule
