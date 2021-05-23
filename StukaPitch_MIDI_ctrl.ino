/*
 * StukaPitch_MIDI_ctrl.ino
 *
 * Created: 22.05.2021 
 * Author: ameise (Martin Sukale)
 * based on MIDIUSB examples from MIDIUSB arduino lib by Gary Grewal
 * 
 * Native MIDI Device, hardware arduino micro + TLP281 4x optocoupler board
 * to be connected to Flokason StuKa Pitch98-13 breakout box
 * 
 * TODO: rewrite and use MIDI_NOTE : PIN function table
 * GOAL: bidirectional communication with DAW plugin
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
const uint8_t LOOP_NOTE   = 0x13; //G-1 starts the loop mode: start, wait one revolution, stop

//adjust to your actual hardware: MS_PER_REVOLUTION - TIME_TO_LOWER_HEAD + TIME_TO_LIFT_HEAD 
const int REVOLUTION_TIME_33 = 1800; //1800 ms = 33,33 RPM
const int LOOP_MARK_TIME = 1000; //move the cutterhead forward after cutting locked groove
const int DEBOUNCE_TIME = 100; //how long to pull up pins for proper signal transmission to pitch

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

//print debug messages on Serial output
void dprintln(const char str[]){
  if (debug_level) {
    Serial.print(str);
    Serial.print("\n");
  }
}


void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      if (debug_level>1) {
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
            dprintln("MARK ON");
          break;
          case START_NOTE:
            digitalWrite(START_PIN, HIGH);
            dprintln("START ON");
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, HIGH);
            dprintln("FAST ON");
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, HIGH);
            dprintln("STOP ON");
          break;
          case LOOP_NOTE:
            dprintln("LOOP START");
            digitalWrite(START_PIN,HIGH); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(START_PIN,LOW);
            
            delay(REVOLUTION_TIME_33); //wait for one revolution 33RPM
            
            dprintln("LOOP STOP");
            digitalWrite(STOP_PIN,HIGH); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(STOP_PIN,LOW);
            
            delay(10*DEBOUNCE_TIME); //waiting for pitch to stop cutting, and before next command
            
            dprintln("MARK BETWEEN LOOP ON");
            digitalWrite(MARK_PIN, HIGH);
            delay(LOOP_MARK_TIME); //move cutterhead forward certain time TODO: pitch optimize locked grooves for space effiency
            digitalWrite(MARK_PIN, LOW); //ready for next locked groove
            dprintln("MARK BETWEEN LOOP OFF");
       }
      }
      
      if (rx.header==0x08 && rx.byte1==0x80|MIDI_IN_CHN) {
        //we got a note on event on our midi chn
        switch (rx.byte2) {
          case MARK_NOTE:
            digitalWrite(MARK_PIN, LOW);
            dprintln("MARK OFF");
          break;
          case START_NOTE:
            digitalWrite(START_PIN, LOW);
            dprintln("START OFF");
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, LOW);
            dprintln("FAST OFF");
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, LOW);
            dprintln("STOP OFF");
          break;
       }
      }
      
    }
  } while (rx.header != 0);
}
