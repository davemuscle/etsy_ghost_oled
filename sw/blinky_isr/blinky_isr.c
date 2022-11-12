#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int intr_count=0;

ISR (TIMER0_OVF_vect){
    // fast, 16ms
    PORTB^=(1<<PB3);

    // divide more for 1 second
    //if (intr_count==63){
    //    PORTB^=(1<<PB3);
    //    intr_count=0;
    //}
    //else intr_count++; //incrementing c upto 63
}

void timer_setup() {
    DDRB |= (1<<PB3);
    TCCR0A=0x00;
    TCCR0B=0x00;
    TCCR0B |= (1<<CS00)|(1<<CS02);
    sei();
    TCNT0=0;
    TIMSK|=(1<<TOIE0);
    PORTB|=(1<<PB3);
}

int main (void){
    timer_setup();
    while(1);
}
