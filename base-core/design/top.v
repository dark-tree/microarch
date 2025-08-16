module top
	(
		input clk,
		input reset,
		input D,
		output reg Q
	);

	always @(posedge clk or posedge reset)
	begin
		if(reset == 1'b1)
			Q <= '0;
		else
			Q <= D; 	
	end
endmodule
		

	 
