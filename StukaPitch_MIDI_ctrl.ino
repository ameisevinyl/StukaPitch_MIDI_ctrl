/*
 * StukaPitch_MIDI_ctrl.ino
 *
 * Created: 22.05.2021 
 * Author: ameise (Martin Sukale)
 * based on MIDIUSB examples from MIDIUSB arduino lib by Gary Grewal
 * 
 * Native MIDI Device, hardware arduino micro + TLP281 4x optocoupler board
 * to be connected to Flokason StuKa Pitch98-13 breakout box
 */ 

#include "MIDIUSB.h"

const int TX_LED = 30;
const int RX_LED = 17;

const int debug_level = 1; //Serial debug output is switched on if >0

//Pitch Connectors
const int START_PIN = 9;
const int STOP_PIN = 8;
const int FAST_PIN = 7;
const int MARK_PIN = 6;

const uint8_t MIDI_IN_CHN = 0x00; //1 - hex listening on this channel btw: 0:15 = 1:16 in midi world

const uint8_t START_NOTE  = 0x11; //F-1
const uint8_t STOP_NOTE   = 0x10; //E-1
const uint8_t FAST_NOTE   = 0x0E; //D-1
const uint8_t MARK_NOTE   = 0x0C; //C-1

void setup() {
  
  pinMode(START_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);
  pinMode(FAST_PIN, OUTPUT);
  pinMode(MARK_PIN, OUTPUT);

  digitalWrite(START_PIN, LOW);
  digitalWrite(STOP_PIN, LOW);
  digitalWrite(FAST_PIN, LOW);
  digitalWrite(MARK_PIN, LOW);
  
  
  Serial.begin(115200);
}

void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      if (debug_level) {
        Serial.print("Received: ");
        Serial.print(rx.header, HEX);
        Serial.print("-");
        Serial.print(rx.byte1, HEX);
        Serial.print("-");
        Serial.print(rx.byte2, HEX);
        Serial.print("-");
        Serial.println(rx.byte3, HEX);
      }
      
      if (rx.header==0x09 && rx.byte1==0x90|MIDI_IN_CHN) {
        //we got a note on event on our midi chn
        switch (rx.byte2) {
          case MARK_NOTE:
            digitalWrite(MARK_PIN, HIGH);
            if (debug_level) {Serial.print("MARK ON");}
          break;
          case START_NOTE:
            digitalWrite(START_PIN, HIGH);
            if (debug_level) {Serial.print("START ON");}
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, HIGH);
            if (debug_level) {Serial.print("FAST ON");}
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, HIGH);
            if (debug_level) {Serial.print("STOP ON");}
          break;
       }
      }
      
      if (rx.header==0x08 && rx.byte1==0x80|MIDI_IN_CHN) {
        //we got a note on event on our midi chn
        switch (rx.byte2) {
          case MARK_NOTE:
            digitalWrite(MARK_PIN, LOW);
            if (debug_level) {Serial.print("MARK OFF");}
          break;
          case START_NOTE:
            digitalWrite(START_PIN, LOW);
            if (debug_level) {Serial.print("START OFF");}
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, LOW);
            if (debug_level) {Serial.print("FAST OFF");}
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, LOW);
            if (debug_level) {Serial.print("STOP OFF");}
          break;
       }
      }
      
    }
  } while (rx.header != 0);
}
