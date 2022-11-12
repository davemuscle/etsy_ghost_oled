// DaveMuscle

// Single SPI Master
/*
    SPI_CLKDIV - Clock divider tickrate >= 1
    SPI_WIDTH  - Word width

    CPOL - 0 = low idle clock, assert high
           1 = high idle clock, assert low
    CPHA - 0 = setup on trailing edge, hold on leading edge
    CPHA - 1 = setup on leading edge,  hold on trailing edge

    https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase
*/


module spi_master #(
    parameter int SPI_CLKDIV = 16,
    parameter int SPI_WIDTH  = 8,
    parameter bit CPOL       = 0,
    parameter bit CPHA       = 0
)(
    input  bit                 clk,
    input  bit                 rst_n,
    input  bit                 tx_valid,
    output bit                 tx_ready,
    input  bit [SPI_WIDTH-1:0] tx_data,
    output bit                 rx_valid,
    input  bit                 rx_ready,
    output bit [SPI_WIDTH-1:0] rx_data,
    output bit                 spi_sclk,
    output bit                 spi_mosi,
    output bit                 spi_cs_n,
    input  bit                 spi_miso
);

    //have to divide the generic div by two since the tick occurs for both edges
    localparam int TRUE_DIV = (SPI_CLKDIV > 1) ? SPI_CLKDIV/2 : 1;
    localparam int TICK_W = TRUE_DIV;
    localparam int DATA_W = $clog2(SPI_WIDTH);

    typedef enum {Idle, Strt, Clk0, Clk1, Done} state_t;
    state_t state;
    state_t next_state;

    bit [TICK_W-1:0]    tick_cnt;
    bit                 tick;
    bit [DATA_W-1:0]    data_cnt;
    bit                 data_last;
    bit [SPI_WIDTH-1:0] tx_buffer;
    bit [SPI_WIDTH-1:0] rx_buffer;
    bit                 cs;
    bit                 sclk;
    bit                 leading_edge;
    bit                 trailing_edge;
    bit                 setup_mosi;
    bit                 hold_miso;

    //state decode for simulation waveform
    `ifndef SYNTHESIS
    bit [255:0] state_ascii;
    always_comb begin
        case(state)
            Idle: state_ascii = "Idle";
            Strt: state_ascii = "Strt";
            Clk0: state_ascii = "Clk0";
            Clk1: state_ascii = "Clk1";
            Done: state_ascii = "Done";
        endcase 
    end
    `endif

    //tick generator
    //when SPI_CLKDIV=1 then every clock should generate a tick
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            tick_cnt <= '0;
            tick     <= 1'b0;
        end
        else if(state == Idle) begin
            tick_cnt <= '0;
            tick     <= 1'b0;
        end
        else if(tick_cnt != TICK_W'(TRUE_DIV-1)) begin
            tick_cnt <= tick_cnt + 1'b1;
            tick     <= 1'b0;
        end
        else begin
            tick_cnt <= '0;
            tick     <= 1'b1;
        end
    end

    //state transition
    always_comb begin
        case(state)
            Idle: next_state = (tx_valid & tx_ready & !rx_valid) ? Strt : Idle;
            Strt: next_state = tick ? Clk0 : Strt;
            Clk0: next_state = tick ? (data_last ? Done : Clk1) : Clk0;
            Clk1: next_state = tick ? Clk0 : Clk1;
            Done: next_state = tick ? Idle : Done;
        endcase
    end

    // state flow
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            state <= Idle;
        end
        else begin
            state <= next_state;
        end
    end

    //clock transitions
    assign trailing_edge = next_state == Clk0 & tick & cs;
    assign leading_edge  = next_state == Clk1 & tick & cs;

    //edge case needed on mosi for CPHA=0 first_n setup bit occuring when cs is asserted
    //and'd with data_last so only SPI_WIDTH pulses are shown in the waveform -- for looks
    assign setup_mosi = (CPHA ? leading_edge  : (trailing_edge | (next_state == Clk0 & tick & !cs))) & !data_last;
    //and'd with data_last so only SPI_WIDTH pulses are shown in the waveform -- actually needed
    assign hold_miso  = (CPHA ? trailing_edge : leading_edge) & !data_last;

    // transmit ready
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            tx_ready <= 1'b0;
        end
        else begin
            tx_ready <= next_state == Idle;
        end
    end

    // transmit buffer and spi_mosi
    always_ff @(posedge clk) begin
        if(tx_valid & tx_ready) begin
            tx_buffer <= tx_data;
        end
        else if(setup_mosi) begin
            tx_buffer <= {tx_buffer[SPI_WIDTH-2:0], 1'b0};
            spi_mosi  <= tx_buffer[SPI_WIDTH-1];
        end
    end

    // bit counter
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            data_cnt  <= '0;
            data_last <= '0;
        end
        else begin
            if(next_state == Done & tick) begin
                data_cnt  <= '0;
                data_last <= 1'b0;
            end
            else if(hold_miso) begin
                data_cnt  <= data_cnt + 1'b1;
                data_last <= data_cnt == SPI_WIDTH-1;
            end
        end
    end

    // receive buffer
    always_ff @(posedge clk) begin
        if(hold_miso) begin
            rx_buffer <= {rx_buffer[SPI_WIDTH-2:0], spi_miso};
        end
    end

    // chip select
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            cs <= 1'b0;
        end
        else begin
            if(next_state == Done & tick) begin
                cs <= 1'b0;
            end
            else if(next_state == Clk0 & tick) begin
                cs <= 1'b1;
            end
        end
    end

    // serial clock
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            sclk <= CPOL;
        end
        else begin
            if(next_state == Idle) begin
                sclk <= CPOL;
            end
            else if(leading_edge | trailing_edge) begin
                sclk <= !sclk;
            end
        end
    end

    // spi clock and chip-select
    assign spi_cs_n   = !cs;
    assign spi_sclk   = sclk;

    // receive valid
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            rx_valid <= 1'b0;
            rx_data  <= '0;
        end
        else begin
            if(next_state == Done & tick) begin
                rx_valid <= 1'b1;
                rx_data  <= rx_buffer;
            end
            else if(rx_valid & rx_ready) begin
                rx_valid <= 1'b0;
            end
        end
    end

endmodule
