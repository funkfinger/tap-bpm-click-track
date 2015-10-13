// this is currently straight avr-gcc but someday may be an Arduino project...
// #ifdef ENERGIA
//   #include "Energia.h"
// #else
//   #include "Arduino.h"
// #endif

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL // 8 MHz
#include <util/delay.h>

#define PORTB_MASK  (1 << PB1)

#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)

#define LED PB1
#define LED_PORT PORTB
#define LED_DDR DDRB

// void delay_ms(uint16_t ms);
void init();

uint8_t cycle = 244; // 8000000/16384/0 = 4hz / 60 BPM
//uint8_t cycle = 122; // 120 bpm
//uint8_t cycle = 61;  // 240 bpm?


int main() {
  init();  
  for(;;) {
  }
}

void delay_ms(uint16_t ms) {
  while (ms) {
    _delay_ms(1);
    ms--;
  }
}

void init(void) {  
  TCCR1 = 0;                  //stop the timer
  TCNT1 = 0;                  //zero the timer
  GTCCR = _BV(PSR1);          //reset the prescaler
  OCR1A = cycle;                //set the compare value
  OCR1C = cycle;
  TIMSK = _BV(OCIE1A);        //interrupt on Compare Match A
  //TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS10); // 8000000/4096/244 = 4hz or 60 bpm but PO-12 sees this doubled
  //TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS11); // 8000000/8192/244 = 4hz or 60 bpm but PO-12 sees this doubled
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS11) | _BV(CS10); // 8000000/16384/244 = 2hz or 30 bpm
  sei();
}

ISR (TIMER1_COMPA_vect) {
  // cli();
  LED_PORT set(LED);
  delay_ms(60);
  cycle--;
  OCR1A = cycle;                //set the compare value
  OCR1C = cycle;
  LED_PORT clr(LED);
  // reti();
}