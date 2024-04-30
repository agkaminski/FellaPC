`timescale 1ns/1ps

module tb();
	reg pclk;
	reg rst_n;
	wire [9:0] col;
	wire [9:0] row;
	wire blank_n;
	wire hsync;
	wire vsync;
	wire oe_n;
	wire shload_n;

	assign oe_n = 0;

	initial begin
		rst_n = 0;
		#10 rst_n = 1;
	end

	initial begin
		forever begin
			pclk = 0;
			#1 pclk = 1;
		end
	end 

	vga uut(
		.pclk(pclk),
		.rst_n(rst_n),
		.col(col),
		.row(row),
		.blank_n(blank_n),
		.hsync(hsync),
		.vsync(vsync),
		.oe_n(oe_n),
		.shload_n(shload_n)
	);

	initial begin
		$dumpfile("vga.vcd");
		$dumpvars(0,tb);
		#1000000 $finish();
	end
endmodule
