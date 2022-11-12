
module top(

    input  logic        clk,
    input  logic [1:0]  key,
    input  logic [1:0]  sw,
    output logic [7:0]  led,
    inout  logic [3:0]  gpio0
);

    bit avalon_read;
    bit avalon_write;
    bit avalon_waitrequest;
    bit avalon_readdatavalid;

    bit [31:0] avalon_writedata;
    bit [31:0] avalon_readdata;

    bit tx_ready;
    bit rx_valid;
    bit rx_ready;
    bit [31:0] rx_data;

    bit sclk;
    bit mosi;
    bit miso;

    assign rx_ready = 1;

    jtag_avalon jtag (
        .clk_clk              (clk),
        .clk_reset_reset      (1'b0),
		.master_address       (),
		.master_readdata      (avalon_readdata),
		.master_read          (avalon_read),
		.master_write         (avalon_write),
		.master_writedata     (avalon_writedata),
		.master_waitrequest   (avalon_waitrequest),
		.master_readdatavalid (avalon_readdatavalid),
		.master_byteenable    (),
		.master_reset_reset   (1'b0)
    );

    assign avalon_waitrequest = !tx_ready;

    spi_master #(
        .SPI_CLKDIV(300),
        .SPI_WIDTH(32),
        .CPOL(0),
        .CPHA(0)
    ) u_spi_master (
        .clk(clk),
        .rst_n(key[0]),
        .tx_valid(avalon_write),
        .tx_ready(tx_ready),
        .tx_data(avalon_writedata),
        .rx_valid(rx_valid),
        .rx_ready(rx_ready),
        .rx_data(rx_data),
        .spi_sclk( sclk ),
        .spi_mosi( mosi ),
        .spi_miso( miso ),
        .spi_cs_n()
    );

    always_ff @(posedge clk or negedge key[0]) begin
        if(!key[0]) begin
            avalon_readdatavalid <= '0;
            avalon_readdata <= '0;
        end
        else begin
            avalon_readdatavalid <= avalon_read & !avalon_waitrequest;
            if(rx_valid & rx_ready) begin
                avalon_readdata <= rx_data;
            end
        end
    end

    assign led[7:1] = '1;
    assign led[0] = sw[0]; // light on means you can program

    assign gpio0[0] = !sw[0];   // reset, inverted for NPN level shifter
    assign gpio0[1] = sw[0] ? 1'bZ : !sclk;    // sclk, inverted for NPN level shifter
    assign gpio0[2] = sw[0] ? 1'bZ : !mosi;    // mosi, inverted for NPN level shifter
    assign miso     = gpio0[3]; // miso

endmodule
