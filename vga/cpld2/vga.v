`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    09:49:49 02/25/2024 
// Design Name: 
// Module Name:    vga 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module vga(
    input pclk,
	 input rst_n,
    output [9:3] col,
    output [8:0] row,
    output reg blank,
    output reg hsync,
    output reg vsync,
	 output reg vblank_n,
    input oe_n,
	 output shload_n,
	 input [5:0] scroll
    );

reg [9:0] col_i;
reg [9:0] row_i;

// counters
always @(posedge pclk, negedge rst_n) begin
	if (~rst_n) begin
		col_i <= 0;
		row_i <= 0;
	end else begin
		col_i <= col_i + 1;
		if (col_i == 799) begin
			col_i <= 0;
			row_i <= row_i + 1;
			if (row_i == 524)
				row_i <= 0;
		end
	end
end

// blanking
always @(posedge pclk, negedge rst_n) begin
	if (~rst_n)
		vblank_n <= 0;
	else begin
		if (row_i >= 480)
			vblank_n <= 0;
		else
			vblank_n <= 1;
	end
end

always @(posedge pclk, negedge rst_n) begin
	if (~rst_n)
		blank <= 1;
	else begin
		if (col_i < 7 || col_i >= (640 - 1 + 8) || row_i >= 480)
			blank <= 1;
		else
			blank <= 0;
	end
end

// hsync
always @(posedge pclk, negedge rst_n) begin
	if (~rst_n)
		hsync <= 0;
	else begin
		if (col_i >= (640 + 16 + 8 - 1) && col_i < (640 + 16 + 8 + 96 - 1))
			hsync <= 1;
		else
			hsync <= 0;
	end
end

// vsync
always @(posedge pclk, negedge rst_n) begin
	if (~rst_n)
		vsync <= 0;
	else begin
		if (row_i >= (480 + 10) && row_i < (480 + 10 + 2))
			vsync <= 1;
		else
			vsync <= 0;
	end
end

assign col = (~oe_n) ? (col_i >> 3) : 10'bz;
assign row = (~oe_n) ? (row_i + (scroll << 3)) : 10'bz;

assign shload_n = (col_i[2:0] == 3'b111) ?  1'b0 : 1'b1;

endmodule
