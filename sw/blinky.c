#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>

//int x = 10;

int main(void){
    DDRB |= _BV(DDB3);
    while(1){
        PORTB |= _BV(PORTB3);
        _delay_ms(500);
        PORTB &= ~_BV(PORTB3);
        _delay_ms(500);
    }
}
