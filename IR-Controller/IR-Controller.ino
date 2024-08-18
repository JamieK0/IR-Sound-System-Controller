/*
 *. Using the IRremote Library:
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  MIT License
 */


/*  ROTARY ENCODER:
   Based on Oleg Mazurov's code for rotary encoder interrupt service routines for AVR micros
   here https://chome.nerpa.tech/mcu/reading-rotary-encoder-on-arduino/
   and using interrupts https://chome.nerpa.tech/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/

   This example does not use the port read method. Tested with Nano and ESP32
   both encoder A and B pins must be connected to interrupt enabled pins, see here for more info:
   https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
*/
#include <Arduino.h>

// Define rotary encoder pins
#define ENC_A 2
#define ENC_B 3

// Define IR Pins
#include "PinDefinitionsAndMore.h" //Define macros for input and output pin etc.
#include <IRremote.hpp>
#define IR_SEND_PIN 4

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 0;

void setup() {

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);

  // Start the serial monitor to show output
  Serial.begin(115200);
  // Print Arduino program
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  IrSender.begin(); // Start with IR_SEND_PIN as send pin and if NO_LED_FEEDBACK_CODE is NOT defined, enable feedback LED at default feedback LED pin
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);

}
// Forward IR Code
uint16_t AddressOne = 0x5343;
uint8_t CommandOne = 0x33;
uint8_t ReapeatsOne = 0;

// Backwards IR Code
uint16_t AddressTwo = 0x5343;
uint8_t CommandTwo = 0x3B;
uint8_t ReapeatsTwo = 0;

void loop() {
  static int lastCounter = 0;
  // If count has changed print the new value to serial
  if(counter != lastCounter){
    Serial.println(counter);
    lastCounter = counter;
  }
}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    ///////////////////////
    //      FORWARDS     //
    ///////////////////////   
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    //IR Transmission
    Serial.print(F("Send now: address=0x"));
    Serial.print(AddressOne, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(CommandOne, HEX);
    Serial.print(F(" repeats="));
    Serial.print(ReapeatsOne);
    Serial.println();

    Serial.println(F("Send Samsung with 16 bit address"));
    Serial.flush();

    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendSamsung(AddressOne, CommandOne, ReapeatsOne);
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    ///////////////////////
    //    BACKWARDS      //
    ///////////////////////
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    //IR Transmission
    Serial.print(F("Send now: address=0x"));
    Serial.print(AddressTwo, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(CommandTwo, HEX);
    Serial.print(F(" repeats="));
    Serial.print(ReapeatsTwo);
    Serial.println();

    Serial.println(F("Send Samsung with 16 bit address"));
    Serial.flush();

    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendSamsung(AddressTwo, CommandTwo, ReapeatsTwo);
    encval = 0;
  }
} 