`timescale 1ns/1ps

module tb();
	reg pclk;
	reg rst_n;
	wire [9:3] col;
	wire [8:0] row;
	wire blank;
	wire vblank_n;
	wire hsync;
	wire vsync;
	wire oe_n;
	wire shload_n;
	wire [5:0] scroll;

	assign oe_n = 0;
	assign scroll = 5;

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
		.blank(blank),
		.vblank_n(vblank_n),
		.hsync(hsync),
		.vsync(vsync),
		.oe_n(oe_n),
		.shload_n(shload_n),
		.scroll(scroll)
	);

	initial begin
		$dumpfile("vga.vcd");
		$dumpvars(0,tb);
		#1000000 $finish();
	end
endmodule
