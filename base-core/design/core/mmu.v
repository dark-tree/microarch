module mmu
  #(
    parameter IO_MEMORY_SPACE_SIZE=3

  )
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
    output reg[7:0] memory_address = 8'b00000000,
    output reg[7:0] memory_in = 8'b00000000,
    output reg memory_read_signal = 1'b0,
    output reg memory_write_signal = 1'b0,
    input[7:0] memory_out,
    input memory_ready,
    output pre_completed,
    output reg operation_ongoing = 1'b0,
    // CPU peripheals
    output reg io_registers_write = 1'b0,
    output reg io_registers_read = 1'b0,
    input io_registers_ready,
    output reg [3:0] io_registers_address,
    output reg [7:0] io_registers_write_bus,
    input [7:0] io_registers_read_bus
  );

  reg [7:0] spec_mem_interrupt[1:0];

  wire io_register_operation = (address < IO_MEMORY_SPACE_SIZE + 2);

  wire io_register_operation_completed = (io_registers_read | io_registers_write);

  wire operation_completed = ((memory_read_signal | memory_write_signal) & memory_ready) | (io_registers_ready & io_register_operation_completed) | completed;

  wire actually_execute = execute & (!operation_ongoing);

  always @(posedge clk)
  begin

    if(operation_ongoing == 1'b1) begin
      if(operation_completed == 1'b1) begin
        if(io_register_operation_completed) begin
          out_data <= io_registers_read_bus;
        end else begin
          out_data <= memory_out;
        end
        completed <= 1'b1;
        operation_ongoing <= 1'b0;
        memory_read_signal <= 1'b0;
        memory_write_signal <= 1'b0;
		io_registers_write <= 1'b0;
		io_registers_read <= 1'b0;
      end else begin
        completed <= 1'b0;
      end
    end

    if(actually_execute == 1'b1) begin
      memory_address <= address;
      io_registers_address <= address[3:0] - 2;
      if(write == 1'b1) begin
        if(|address[7:1] == 1'b0) begin
          spec_mem_interrupt[address[0]] <= in_data;
          completed <= 1'b1;
        end else begin
          operation_ongoing <= 1'b1;
          if(io_register_operation) begin
            io_registers_write <= 1'b1;
            io_registers_write_bus <= in_data;
          end else begin
            memory_write_signal <= 1'b1;
            memory_in <= in_data;
          end
        end
      end else begin
        if(|address[7:1] == 1'b0) begin
          out_data <= spec_mem_interrupt[address[0]];
          completed <= 1'b1;
        end else begin
          operation_ongoing <= 1'b1;
          if(io_register_operation) begin
            io_registers_read <= 1'b1;
          end else begin
            memory_read_signal <= 1'b1;
          end
        end
      end
    end

    if((actually_execute | operation_ongoing) == 1'b0) begin
      completed <= 1'b0;
    end

    if(set_interrupt_return_address == 1'b1) begin
      spec_mem_interrupt[0] <= interrupt_return_address[15:8];
      spec_mem_interrupt[1] <= interrupt_return_address[7:0];
    end

  end



  assign pre_completed = (operation_ongoing == 1'b1) ? operation_completed : ((actually_execute & (|address[7:1] == 1'b0)) | completed);

endmodule
