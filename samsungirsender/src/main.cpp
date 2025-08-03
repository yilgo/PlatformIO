/*

This is simple dummy Samsung Smart TV IR Sender.
Current version there is only single button for CHANNEL_UP.
It can be extended.


T = 1/f --> T=1/40 Khz = 25µs
 12.5 µs -- High
 12.5 µs -- LOW
 1 second = 16 000 000 cycle
 1 μs     =  16 cycle
 12.5 μs×16 cycles/μs= 200 cycle

 if set prescale/8 = 200/8 = 25 ; so every 25 cycle 12,5 microsecond.

 For samsung code

 start
  4.5 ms HIGH -
  4.5 ms LOW -

  Logical 1 = 560 μs HIGH, 1690 μs LOW
  Logical 0 = 560 μs HIGH, 560 μs LOW

 */

#include <Arduino.h>


#define DEVICE_CODE   0x0707
#define CHANNEL_PLUS  0xED12



#define IR_LED PB0
#define IR_BTN PB1
#define IR_BTN_PIN PINB

void delay12_5us() {
asm volatile (
"    ldi  r20, 63	\n"
"1:  dec  r20	\n"
"    brne 1b	\n"
);
}

void idle(uint16_t idle_time){
  PORTB  &= ~(1 << IR_LED);
  uint16_t cycle = (uint16_t)(idle_time / 25.0);
  for (uint16_t i = 0 ; i < (cycle * 2); i++) {
    delay12_5us();
  }
}


void sendActiveSignal(uint16_t duration_as_microsecond){

    uint16_t cycle = (uint16_t)(duration_as_microsecond / 25.0);
    for (uint16_t i=0 ; i < cycle; i++){
        PORTB  |= (1 << IR_LED);
        delay12_5us();
        PORTB  &= ~(1 << IR_LED);
        delay12_5us();
    }
}


void sendIR(uint16_t deviceCode, uint16_t commandCode){

    for (uint8_t i = 0; i <= 15; i++){
        if (deviceCode & (1 << i)){
          sendActiveSignal(560);
          idle(1690);
        } else {
          sendActiveSignal(560);
          idle(560);
        }
    }

    for (uint8_t y = 0; y <= 15; y++){
        if (commandCode & (1 << y)){
          sendActiveSignal(560);
          idle(1690);
        } else {
          sendActiveSignal(560);
          idle(560);
        }
    }

// it is required to send last active signal which instructs receiver to end of a message.
sendActiveSignal(560);
}



void setup() {
  Serial.begin(9600);
  DDRB |= (1 << IR_LED );
  DDRB &= ~(1 << IR_BTN );
  PORTB &= ~( 1 << IR_LED);
}

void loop() {
  ///put your main code here, to run repeatedly:
    if(IR_BTN_PIN & (1 << IR_BTN )){
      delay(50);
      sendActiveSignal(4500);
      idle(4500);
      sendIR(0x0707, CHANNEL_PLUS);
    }
}

