#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define LED 1
#define LED2 3
#define BUTTON 2
#define SIGNAL_PIN 0

#define set(x) |= (1<<x)
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)
#define chkBit(x) & (1<<(x)))

void setupTimers() {

  // timer 0 - used for ticks...
  TCCR0A set(WGM01); // CTC mode. set WGM01
  
  // divide by 64
  TCCR0B set(CS00);
  TCCR0B set(CS01);
  TCCR0B clr(CS02);
  
  OCR0A = 125; // 8,000,000 / 64 / 125 = 1ms
  TIMSK set(OCIE0A); // set timer to interrupt
}

void setupButton() {
  // button stuff....
  DDRB clr(BUTTON); // button is input...
  PORTB set(BUTTON);
  GIMSK set(INT0);
  MCUCR clr(ISC00);
  MCUCR clr(ISC01);
}

void setupSignal() {
  DDRB set(SIGNAL_PIN);  
}

void setupLed() {
  // led is output...
  DDRB set(LED);
  DDRB set(LED2);
}

void setup(void) {
  // stop interrupts...
  cli();
  
  setupLed();
  setupTimers();
  setupButton();

  // start interrupts...
  sei();

}

void loop() {
  for(;;) {
  }
}

int main() {
  setup();
  loop();
}

volatile uint32_t mills;
volatile uint32_t pOptick = 0;
volatile uint8_t pOpPulse = 0;
volatile uint16_t pOpBpmValue = 261;
volatile uint32_t buttonOffTill = 0;
volatile uint32_t lastClickTime;
volatile uint8_t consecutiveClicks = 0;

ISR (TIMER0_COMPA_vect) {
  PORTB inv(LED2);
  mills++;
  pOptick++;
  // timer for Pocket Operator synce signal
  if (pOptick == pOpBpmValue) {
    pOptick = 0;
    pOpPulse = 10; // length of Pocket Operator sync signal - just something that works....
    PORTB set(SIGNAL_PIN);
  }
  // timer for length of signal 
  if (pOpPulse == 0) {
    PORTB clr(SIGNAL_PIN);
  }
  else {
    pOpPulse--;
  }
  
  // timer for button debounce...
  if (buttonOffTill < mills) {
    GIMSK set(INT0); // turn the button interrupt on after debounce...
    buttonOffTill = 0;
  }
  
  // timer for wait period between bpm clicks - wait (about?) 2 seconds and reset bpm clicker
  if (mills > lastClickTime + 2000) {
    consecutiveClicks = 0;
    PORTB set(LED);
  }
}

volatile uint32_t firstClickTime;
volatile uint16_t totalTime;

// PIN 2 is INT0
ISR (INT0_vect) {
  PORTB clr(LED);
  lastClickTime = mills;
  if (consecutiveClicks > 0) {
    totalTime = mills - firstClickTime;
    pOpBpmValue = round((totalTime / consecutiveClicks) / 2); // Pocket Operator seems to sync on 2 beats per measure...
  }
  else {
    firstClickTime = mills;
  }
  consecutiveClicks++;
  GIMSK clr(INT0); // turn the button interrupt off for debounce...
  buttonOffTill = mills + 200; // time to debounce...
}






