#define __AVR_ATtiny85__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#define F_CPU 8000000UL  // 8 MHz
#include <util/delay.h>

#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)
#define check_bit(var,pos) ((var) & (1<<(pos)))

#define PULSE_WIDTH 1

#define BUTTON PINB4
#define PULSE PINB1
#define SCL PINB0
#define SDA PINB2

void blink(uint8_t times);

// uint8_t cycle = 100; // ~ 148 bpm (TCCR1 = 1100)
// uint8_t cycle = 123; // ~ 120 bpm (TCCR1 = 1100)
// uint8_t cycle = 200; // ~ 74 bpm (TCCR1 = 1100)
uint8_t cycle = 255; // ~ 60 bpm (TCCR1 = 1100)

// used to track how many times overflow inerrupt is triggered...
uint8_t overflowCount = 0;

void _ms (uint16_t count) {
  while(count--) {
    _delay_ms(1);
  }
}


int main() {
  cli();
  
  DDRB set(PULSE); // pulse out

  // button stuff....
  PORTB set(PB4); // enable pull-up resistor on button...
  DDRB clr(BUTTON); // button in
  // button interrupt
  PCMSK set(PCINT4);
  GIMSK set(PCIE);
  
  TCCR1 = 0;
  TCNT1 = 0;
  GTCCR = _BV(PSR1);
  
  OCR1A = cycle;
  OCR1C = cycle;
  
  TCCR1 = 0;
  TCCR1 |= (1<<CTC1); // Clear Timer/Counter on Compare Match
  
  // 1100 = CK/2048
  TCCR1 |= (0<<CS10); // Clock Select Bit 0
  TCCR1 |= (0<<CS11); // Clock Select Bit 1
  TCCR1 |= (1<<CS12); // Clock Select Bit 2
  TCCR1 |= (1<<CS13); // Clock Select Bit 3
  
  TIMSK = (1<<OCIE1A);

  // timer 0 - clkI/O/1024 (From prescaler)
  TCCR0B = 0;
  TCCR0B set(CS02);
  TCCR0B clr(CS01);
  TCCR0B set(CS00);
  TIMSK set(TOIE0); // enable overflow interrupt

  // start interrupts...
  sei();

  
  // loop does nothing...
  for(;;) {
  }
}




// timer 1 compare interrupt...
ISR (TIMER1_COMPA_vect) {
  // this is odd, pocket operator is 2 beats per quarter note or 2ppqn (2 pulse per quarter note)
  if (check_bit(PORTB, PULSE)) {
    PORTB clr(PULSE);
    OCR1A = cycle;
    OCR1C = cycle;
  }
  else {
    PORTB set(PULSE);
    OCR1A = PULSE_WIDTH;
    OCR1C = PULSE_WIDTH;
  }
}


// timer 0 overflow interrupt...
ISR (TIM0_OVF_vect) {
  overflowCount++;
  if (overflowCount > 4) {
    overflowCount = 0;
    // cycle = 255;
  }
}

// pcint4 button press interrupt...
ISR (PCINT4_vect) {
  cycle++;
  return;
}


