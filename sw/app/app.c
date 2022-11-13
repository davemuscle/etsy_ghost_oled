//#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "img.h"

#define BB_SCLK PB1
#define BB_MOSI PB3
#define BB_DC   PB4
#define BB_CS   PB0

#define SPI_DELAY 1

#define SCREEN_BYTES 1024
#define FRAME_TOGGLE_MAX 4

volatile uint8_t frame_advance = 0;
volatile uint8_t frame_toggle = 0;

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

void command_write(uint8_t byte){
    spi_write(byte, 0);
}

void pixel_write(int num, uint8_t byte){
    for(int i =0; i < num+1; i++){
        spi_write(byte, 1);
    }
}


void display_image(const uint8_t *img, int len){
    for(int i = 0; i < len; i += 2){
        uint8_t a = pgm_read_byte(&img[i+0]);
        uint8_t b = pgm_read_byte(&img[i+1]);
        pixel_write(a, b);
    }
}

void display_image_blocking(const uint8_t *img, int len){
    while(!frame_advance);
    frame_advance = 0;
    display_image(img, len);
}

void debug_image(const uint8_t *img, int len){
    for(int i = 0; i < len; i += 2){
        uint8_t a = pgm_read_byte(&img[i+0]);
        uint8_t b = pgm_read_byte(&img[i+1]);
        command_write(a);
        spi_write(b, 1);
        //pixel_write(a, b);
    }
}

ISR (TIMER0_OVF_vect){
    if(frame_toggle == FRAME_TOGGLE_MAX-1){
        frame_advance = 1;
        frame_toggle = 0;
    }else{
        frame_toggle++;
    }
}

// 16 ms
void timer_setup() {
    TCCR0A=0x00;
    TCCR0B=0x00;
    TCCR0B |= (1<<CS00)|(1<<CS02);
    sei();
    TCNT0=0;
    TIMSK|=(1<<TOIE0);
}


void animate(){
    for(int i = 0; i <  15; i++) display_image_blocking(img_000, sizeof(img_000));
    for(int i = 0; i <  2; i++) display_image_blocking(img_008, sizeof(img_008));
    for(int i = 0; i <  1; i++) display_image_blocking(img_010, sizeof(img_010));
    for(int i = 0; i <  2; i++) display_image_blocking(img_011, sizeof(img_011));
    for(int i = 0; i <  2; i++) display_image_blocking(img_013, sizeof(img_013));
    for(int i = 0; i <  2; i++) display_image_blocking(img_014, sizeof(img_014));
    for(int i = 0; i <  2; i++) display_image_blocking(img_000, sizeof(img_000));
    for(int i = 0; i <  1; i++) display_image_blocking(img_018, sizeof(img_018));
    for(int i = 0; i <  1; i++) display_image_blocking(img_019, sizeof(img_019));
    for(int i = 0; i <  1; i++) display_image_blocking(img_020, sizeof(img_020));
    for(int i = 0; i <  1; i++) display_image_blocking(img_000, sizeof(img_000));
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

    pixel_write(SCREEN_BYTES-1, 0x00);

    timer_setup();

    while(1){
        animate();
    }
}


