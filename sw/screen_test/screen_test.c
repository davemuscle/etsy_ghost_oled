#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define BB_SCLK PB1
#define BB_MOSI PB3
#define BB_DC   PB4

#define SPI_DELAY 1

uint8_t dc = 0;

void spi_write(uint8_t byte, uint8_t dc){
    uint8_t sreg = byte;
    uint8_t isolate;

    // setup dc
    if(dc) {
        PORTB |= (1 << BB_DC);
    } else {
        PORTB &= ~(1 << BB_DC);
    }

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
}

void command_write(uint8_t byte){
    spi_write(byte, 0);
}

//void pixel_write(uint8_t num, uint8_t byte){
//    for(int i = 0; i <= num; i++){
//        spi_write(byte, 1);
//    }
//}

void pixel_write(int num, uint8_t byte){
    for(int i =0; i <= num; i++){
        spi_write(byte, 1);
    }
}

//uint8_t test [32][32];
//const uint8_t __attribute__((section(".text"))) test [16][32] = {
//};

#include "img.h"

ISR (TIMER0_OVF_vect){
    //spi_write(0x7F, dc);
    //dc = !dc;
}

void timer_setup() {
    TCCR0A=0x00;
    TCCR0B=0x00;
    TCCR0B |= (1<<CS00)|(1<<CS02);
    sei();
    TCNT0=0;
    TIMSK|=(1<<TOIE0);
}

int main (void){
    // outputs
    DDRB |= (1 << BB_SCLK);
    DDRB |= (1 << BB_MOSI);
    DDRB |= (1 << BB_DC);

    // zeros
    PORTB &= ~(1 << BB_SCLK);
    PORTB &= ~(1 << BB_MOSI);
    PORTB &= ~(1 << BB_DC);

    _delay_ms(100);
    
    command_write(0xAF); // display on
    command_write(0x20); // set address mode
    command_write(0x01); // set address mode - vertical

    //pixel_write(1, 255);
    pixel_write(((128*64)/8)-1, 0);
    pixel_write(((128*64)/8)-1, 255);

    for(int i = 0; i < 622; i += 2){
        uint8_t a = pgm_read_byte(&Cropped_000[i+0]);
        uint8_t b = pgm_read_byte(&Cropped_000[i+1]);
        pixel_write(a, b);
    }

    timer_setup();
    while(1);
}


