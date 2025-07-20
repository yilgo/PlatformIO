#include <Arduino.h>

enum keycode {
  CHANNEL__PLUS = 0x10,
  CHANNEL__DOWN = 0x11
};

const int irRecvPin = 2; // D2
const int RELAY0 = 14;   // A0
const int RELAY1 = 15;   // A1

volatile bool captured_start_bit = false;
volatile uint16_t  tm = 0x0000;
volatile uint16_t code = 0x0000;
byte counter = 0 ;
volatile bool completed = false;

void sirc(){
  if(digitalRead(irRecvPin) ){
    // Interrupt routine to capture IR receiver signal.
      tm = TCNT1;

      if (tm >= 4700 && tm <= 5000 && !captured_start_bit && !completed){
        captured_start_bit = true;
        counter ++;
      }
      else if( tm >= 1100 && tm <= 1400 && captured_start_bit && !completed){
        // 0
        code &= ~( 1 << counter );
        counter ++;
      }
      else if( tm >= 2200 && tm <= 2500 && captured_start_bit && !completed){
        // 1
        code |= (1 << counter);
        counter ++;
      }
      else {
        //Serial.println(tm);
          counter = 0;
          code = 0;
          captured_start_bit = false;
          //completed = false;
      }
      // Current version of Sirc only supports 12 bit.
      // There is also 15 and 20 bit version.
      // TODO: Add 15 and 20 bit version
      if(counter > 12 && captured_start_bit){
        // disable interrupt
        cli();
        completed = true;
      }

  } else {
      // Reset timer at falling edge.(Active low)
      TCNT1 = 0;
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(irRecvPin, INPUT);
  pinMode(RELAY0, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  digitalWrite(RELAY0, LOW);
  digitalWrite(RELAY1, LOW);
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << CS11); // Prescaler = 8 → 2 MHz timer clock (FCPU=16Mhz)
  TCNT1 = 0;
  delay(1000);
  attachInterrupt(digitalPinToInterrupt(irRecvPin), sirc, CHANGE);
  sei();
}

void loop() {

  if(completed){
    // capture only the first SIRC code.
    // some receivers sends at least two or three times.
    // Check only Command code. Ignore Device code
    code = ((code >> 1) & 0x7f);
    Serial.print("Command Code(HEX): "); Serial.println(code, HEX);
    switch(code){
      case CHANNEL__PLUS:
        digitalWrite(RELAY0, !(digitalRead(RELAY0)));
        Serial.print("RELAY0 is: ");  digitalRead(RELAY0)  ? Serial.println("OPEN") : Serial.println("CLOSED");
        break;
      case CHANNEL__DOWN:
        digitalWrite(RELAY1, !(digitalRead(RELAY1)));
        Serial.print("RELAY1 is: ");  digitalRead(RELAY1)  ? Serial.println("OPEN") : Serial.println("CLOSED");
        break;
    }
    delay(120);
    code = 0x00;
    counter = 0x00;
    completed = false;
    captured_start_bit = false;
    // enable interrupt again
    sei();
  }
}