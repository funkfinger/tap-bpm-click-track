// this is currently straight avr-gcc but someday may be an Arduino project...
// #ifdef ENERGIA
//   #include "Energia.h"
// #else
//   #include "Arduino.h"
// #endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define PORTB_MASK  (1 << PB1)

#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)

#define LED PB1
#define LED_PORT PORTB
#define LED_DDR DDRB

void delay_ms(uint16_t ms);
void init();

int main() {
  init();  
  for(;;) {
    LED_PORT set(LED);
    delay_ms(500);
    LED_PORT clr(LED);
    delay_ms(500);
  }
}

void delay_ms(uint16_t ms) {
  while (ms) {
    _delay_ms(1);
    ms--;
  }
}

void init(void) {
  sei();
}

ISR (TIM0_OVF_vect) {
}