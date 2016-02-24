#define __AVR_ATtiny85__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#define F_CPU 8000000UL  // 8 MHz
#include <util/delay.h>

#include <LiquidCrystal_attiny.h>


#define set(x) |= (1<<x) 
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)
#define check_bit(var,pos) ((var) & (1<<(pos)))

#define PULSE_WIDTH 1

void blink(uint8_t times);

// uint8_t cycle = 100; // ~ 148 bpm (TCCR1 = 1100)
// uint8_t cycle = 123; // ~ 120 bpm (TCCR1 = 1100)
// uint8_t cycle = 200; // ~ 74 bpm (TCCR1 = 1100)
uint8_t cycle = 255; // ~ 60 bpm (TCCR1 = 1100)

// used to track how many times overflow inerrupt is triggered...
uint8_t overflowCount = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address

void delay_ms (uint16_t count) {
  while(count--) {
    _delay_ms(1);
  }
}


int main() {
  
  DDRB set(PINB1); // port b pin 1 to output
  DDRB set(PINB2); // port b pin 2 to output
  DDRB set(PINB4); // port b pin 4 to output
  
  cli();
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
  sei();

  // timer 0 - clkI/O/1024 (From prescaler)
  TCCR0B = 0;
  TCCR0B set(CS02);
  TCCR0B clr(CS01);
  TCCR0B set(CS00);
  TIMSK set(TOIE0); // enable overflow interrupt

  // button interrupt
  PCMSK set(PCINT0);
  GIMSK set(PCIE);
  
  // lcd stuff...
  lcd.init();
  lcd.backlight();
  lcd.clear();
  for(int i = 0; i< 3; i++) {
    lcd.backlight();
    delay_ms(250);
    lcd.noBacklight();
    delay_ms(250);
  }
  lcd.backlight(); // finish with backlight on
  lcd.setCursor(0, 0);
  lcd.printstr("hello...");
  delay_ms(200);
  
  // loop does nothing...
  for(;;) {
    lcd.setCursor(0, 1);
    lcd.print(overflowCount);
    lcd.print(" - ");
    lcd.print(cycle);
    lcd.print(" - ");
    lcd.print(TCNT0);
    delay_ms(10);
  }
}

// timer 1 compare interrupt...
ISR (TIMER1_COMPA_vect) {
  // this is odd, pocket operator is 2 beats per quarter note or 2ppqn (2 pulse per quarter note)
  if (check_bit(PORTB, PINB1)) {
    PORTB clr(PINB1);
    OCR1A = cycle;
    OCR1C = cycle;
  }
  else {
    PORTB set(PINB1);
    OCR1A = PULSE_WIDTH;
    OCR1C = PULSE_WIDTH;
  }
}

// timer 0 overflow interrupt...
ISR (TIM0_OVF_vect) {
  overflowCount++;
  if (overflowCount > 4) {
    overflowCount = 0;
  }
  PORTB inv(PIN2);
  PORTB clr(PIN4);
}

// pcint0 button press interrupt...
ISR (PCINT0_vect) {
  // blink(overflowCount);
  uint16_t cycleNumber;

  cycleNumber = TCNT0 + (overflowCount * 255);
  cycle = cycleNumber / 4;
  TCNT0 = 0;
  PORTB set(PIN4);
}


