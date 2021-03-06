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
 * TODO: remove debug code...
 * GOAL: bidirectional communication with DAW plugin
 */ 

#include "MIDIUSB.h"

#define ON LOW
#define OFF HIGH

const int TX_LED = 30;
const int RX_LED = 17;

const int debug_level = 0; //Serial debug output is switched on if >0

//Pitch Connectors
const int MARK_PIN = 2;
const int FAST_PIN = 3;
const int STOP_PIN = 4;
const int START_PIN = 5;
const int RESET_PIN = 6; //not available with 4x optocoupler board...

const uint8_t MIDI_IN_CHN = 0x00; //1 - hex listening on this channel btw: 0:15 = 1:16 in midi world

const uint8_t START_NOTE  = 0x11; //F-1
const uint8_t STOP_NOTE   = 0x10; //E-1
const uint8_t FAST_NOTE   = 0x0E; //D-1
const uint8_t MARK_NOTE   = 0x0C; //C-1
const uint8_t RESET_NOTE  = 0x13; //G-1 (see also: https://audeonic.com/cgi-bin/midi_table.pl)
const uint8_t LOOP33_NOTE   = 0x15; //A-1 starts the loop mode at 33 RPM: start, wait one revolution, stop
const uint8_t LOOP45_NOTE   = 0x17; //B-1 starts the loop mode at 45 RPM: start, wait one revolution, stop

//adjust to your actual hardware: MS_PER_REVOLUTION - TIME_TO_OFFER_HEAD + TIME_TO_LIFT_HEAD 
const int REVOLUTION_TIME_33 = 1800; //1800 ms = 33,33 RPM
const int REVOLUTION_TIME_45 = 1333; //1333 ms = 45 RPM TODO: is this precise enough?

const int LOOP_MARK_TIME = 1000; //move the cutterhead forward after cutting locked groove
const int DEBOUNCE_TIME = 100; //how long to pull up pins for proper signal transmission to pitch

void setup() {
  
  pinMode(START_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);
  pinMode(FAST_PIN, OUTPUT);
  pinMode(MARK_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(START_PIN, OFF);
  digitalWrite(STOP_PIN, OFF);
  digitalWrite(FAST_PIN, OFF);
  digitalWrite(MARK_PIN, OFF);
  digitalWrite(RESET_PIN, OFF);
  
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
            digitalWrite(MARK_PIN, ON);
            dprintln("MARK ON");
          break;
          case START_NOTE:
            digitalWrite(START_PIN, ON);
            dprintln("START ON");
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, ON);
            dprintln("FAST ON");
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, ON);
            dprintln("STOP ON");
          break;
          case RESET_NOTE:
            digitalWrite(RESET_PIN, ON);
            dprintln("RESET ON");
          break;
          case LOOP33_NOTE:
            dprintln("33 RPM LOOP START");
            digitalWrite(START_PIN,ON); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(START_PIN,OFF);
            
            delay(REVOLUTION_TIME_33); //wait for one revolution 33RPM
            
            dprintln("LOOP STOP");
            digitalWrite(STOP_PIN,ON); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(STOP_PIN,OFF);
            
            delay(10*DEBOUNCE_TIME); //waiting for pitch to stop cutting, and before next command
            
            dprintln("MARK BETWEEN LOOP ON");
            digitalWrite(MARK_PIN, ON);
            delay(LOOP_MARK_TIME); //move cutterhead forward certain time TODO: pitch optimize locked grooves for space effiency
            digitalWrite(MARK_PIN, OFF); //ready for next locked groove
            dprintln("MARK BETWEEN LOOP OFF");
          break;
          case LOOP45_NOTE:
            dprintln("45 RPM LOOP START");
            digitalWrite(START_PIN,ON); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(START_PIN,OFF);
            
            delay(REVOLUTION_TIME_45); //wait for one revolution 33RPM
            
            dprintln("LOOP STOP");
            digitalWrite(STOP_PIN,ON); //start cutting
            delay(DEBOUNCE_TIME);
            digitalWrite(STOP_PIN,OFF);
            
            delay(10*DEBOUNCE_TIME); //waiting for pitch to stop cutting, and before next command
            
            dprintln("MARK BETWEEN LOOP ON");
            digitalWrite(MARK_PIN, ON);
            delay(LOOP_MARK_TIME); //move cutterhead forward certain time TODO: pitch optimize locked grooves for space effiency
            digitalWrite(MARK_PIN, OFF); //ready for next locked groove
            dprintln("MARK BETWEEN LOOP OFF");
          break;
       }
      }
      
      if (rx.header==0x08 && rx.byte1==0x80|MIDI_IN_CHN) {
        //we got a note on event on our midi chn
        switch (rx.byte2) {
          case MARK_NOTE:
            digitalWrite(MARK_PIN, OFF);
            dprintln("MARK OFF");
          break;
          case START_NOTE:
            digitalWrite(START_PIN, OFF);
            dprintln("START OFF");
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, OFF);
            dprintln("FAST OFF");
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, OFF);
            dprintln("STOP OFF");
          break;
          case RESET_NOTE:
            digitalWrite(RESET_PIN, OFF);
            dprintln("RESET OFF");
          break;
       }
      }
      
    }
  } while (rx.header != 0);
}
