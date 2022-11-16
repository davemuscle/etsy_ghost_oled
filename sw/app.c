#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "img.h"

#define BB_SCLK PB1
#define BB_MOSI PB3
#define BB_DC   PB4
#define BB_CS   PB0

#define SPI_DELAY         1
#define SCREEN_PIXELS     8192
#define SCREEN_BYTES      1024
#define FRAME_TOGGLE_MAX  3
#define SCROLL_COLUMN_MIN 5
#define SCROLL_COLUMN_MAX 95
#define SCROLL_STEP       2

volatile uint8_t frame_advance = 0;
volatile uint8_t frame_toggle = 0;

volatile uint8_t global_scroll_offset    = 0;
volatile uint8_t global_scroll_direction = 0;

// bit bang spi
void spi_write(uint8_t byte, uint8_t dc){
    uint8_t sreg = byte;
    uint8_t isolate;
    // setup dc
    if(dc) {
        PORTB |= (1 << BB_DC);
    } else {
        PORTB &= ~(1 << BB_DC);
    }
    //setup cs_n
    PORTB &= ~(1 << BB_CS);
    _delay_us(SPI_DELAY);
    for(uint8_t i = 0; i < 8; i++){
        // mask bit to xfer
        isolate = (sreg >> 7) & 1;
        sreg = sreg << 1;
        // setup data
        if(isolate) {
            PORTB |= (1 << BB_MOSI);
        } else {
            PORTB &= ~(1 << BB_MOSI);
        }
        _delay_us(SPI_DELAY);
        // rising edge
        PORTB |=  (1 << BB_SCLK);
        _delay_us(SPI_DELAY);
        // falling edge
        PORTB &= ~(1 << BB_SCLK);
    }
    // deactivate cs
    PORTB |= (1 << BB_CS);
    _delay_us(SPI_DELAY);
}

// send spi command with dc = 0
void command_write(uint8_t byte){
    spi_write(byte, 0);
}

// send an array of spi commands with dc = 1
void pixel_write(int num, uint8_t byte){
    for(int i =0; i < num+1; i++){
        spi_write(byte, 1);
    }
}

// put an image on screen using the custom format from bmp script
void display_image(const uint8_t *img, int len, int pixels){
    int total_pixels = pixels;
    int transfer_pixels;
    if(pixels > 0){
        for(int i = 0; i < len; i += 2){
            uint8_t a = pgm_read_byte(&img[i+0]);
            uint8_t b = pgm_read_byte(&img[i+1]);
            transfer_pixels = (a + 1) * 8;
            if(transfer_pixels >= total_pixels){
                pixel_write((total_pixels/8)-1, b);
                break;
            } else {
                pixel_write(a, b);
                total_pixels -= transfer_pixels;
            }
        }
    }
}

// draw blanks on the screen before displaying a portion of the image
void display_image_scrolled(const uint8_t *img, int len, int scroll){
    int scroll_pixels;
    int image_pixels;
    int scroll_m = scroll & 127;
    if(scroll_m == 0){
        scroll_pixels = 0;
    } else {
        scroll_pixels = scroll_m * 64;
        pixel_write((scroll_pixels/8)-1, 0x00);
    }
    image_pixels = SCREEN_PIXELS - scroll_pixels;
    display_image(img, len, image_pixels);
}

// wait for the interrupt to fire before displaying image
void display_image_blocking(const uint8_t *img, int len){
    while(!frame_advance);
    frame_advance = 0;
    display_image_scrolled(img, len, global_scroll_offset);
}

// timer interupt
ISR (TIMER0_OVF_vect){
    if(frame_toggle == FRAME_TOGGLE_MAX-1){
        frame_advance = 1;
        frame_toggle = 0;
    }else{
        frame_toggle++;
    }
}

// setup timer, this runs at 16 ms. Not sure how with 1024 prescaler but whatever
void timer_setup() {
    TCCR0A=0x00;
    TCCR0B=0x00;
    TCCR0B |= (1<<CS00)|(1<<CS02);
    sei();
    TCNT0=0;
    TIMSK|=(1<<TOIE0);
}

// set global column to the left with bounding checks
void scroll_left(){
    int16_t next_column;
    if(global_scroll_direction == 1){
        next_column = global_scroll_offset - SCROLL_STEP;
        if(next_column <= SCROLL_COLUMN_MIN && global_scroll_offset > SCROLL_COLUMN_MIN){
            global_scroll_direction ^= 1;
        } else {
            global_scroll_offset = next_column;
        }
    }
}

// set global column to the right with bounding checks
void scroll_right(){
    int16_t next_column;
    if(global_scroll_direction == 0){
        next_column = global_scroll_offset + SCROLL_STEP;
        if(next_column >= SCROLL_COLUMN_MAX && global_scroll_offset < SCROLL_COLUMN_MAX){
            global_scroll_direction ^= 1;
        } else {
            global_scroll_offset = next_column;
        }
    }
}

// ghost animation
void animate(){
    for(int i = 0; i < 15; i++) display_image_blocking(img_000, sizeof(img_000));
    scroll_right();
    for(int i = 0; i <  3; i++) display_image_blocking(img_008, sizeof(img_008));
    scroll_right();
    for(int i = 0; i <  2; i++) display_image_blocking(img_010, sizeof(img_010));
    scroll_right();
    for(int i = 0; i <  7; i++) display_image_blocking(img_011, sizeof(img_011));
    scroll_left();
    for(int i = 0; i <  3; i++) display_image_blocking(img_013, sizeof(img_013));
    scroll_left();
    for(int i = 0; i <  2; i++) display_image_blocking(img_014, sizeof(img_014));
    scroll_left();
    for(int i = 0; i <  2; i++) display_image_blocking(img_000, sizeof(img_000));
    for(int i = 0; i <  3; i++) display_image_blocking(img_018, sizeof(img_018));
    for(int i = 0; i <  3; i++) display_image_blocking(img_019, sizeof(img_019));
    for(int i = 0; i <  3; i++) display_image_blocking(img_020, sizeof(img_020));
    for(int i = 0; i <  3; i++) display_image_blocking(img_000, sizeof(img_000));
    for(int i = 0; i <  3; i++) display_image_blocking(img_022, sizeof(img_022));
}

int main (void){
    // outputs
    DDRB |= (1 << BB_SCLK);
    DDRB |= (1 << BB_MOSI);
    DDRB |= (1 << BB_DC);
    DDRB |= (1 << BB_CS);

    // deassert
    PORTB &= ~(1 << BB_SCLK);
    PORTB &= ~(1 << BB_MOSI);
    PORTB &= ~(1 << BB_DC);
    PORTB |=  (1 << BB_CS);

    _delay_ms(100);
    
    command_write(0xAF); // display on
    command_write(0x20); // set address mode
    command_write(0x01); // set address mode - vertical

    // these modes do not work, vertical addressing must be hardwired in the PCB
    //command_write(0x00); // set address mode - horizontal
    //command_write(0x02); // set address mode - page

    // blank screen for debug
    //pixel_write(SCREEN_BYTES-1, 0x00);

    frame_advance           = 0;
    frame_toggle            = 0;
    global_scroll_offset    = 0;
    global_scroll_direction = 0;

    timer_setup();

    while(1){
        animate();
    }
}


