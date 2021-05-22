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

const int debug_level = 1;

//Pitch Connectors
const int START_PIN = 17;
const int STOP_PIN = 30;
const int FAST_PIN = 17;
const int MARK_PIN = 30;

const uint8_t MIDI_IN_CHN = 0x00; //1 - hex listening on this channel btw: 0:15 = 1:16 in midi world

const uint8_t START_NOTE  = 0x11; //F1
const uint8_t STOP_NOTE   = 0x10; //E1
const uint8_t FAST_NOTE   = 0x0E; //D1
const uint8_t MARK_NOTE   = 0x0C; //C1

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  
  pinMode(TX_LED, OUTPUT);
  pinMode(RX_LED, OUTPUT);
  pinMode(START_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);
  pinMode(FAST_PIN, OUTPUT);
  pinMode(MARK_PIN, OUTPUT);

  //initatilze pin states
  digitalWrite(TX_LED, LOW);
  digitalWrite(RX_LED, LOW);
  
  Serial.begin(115200);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
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
            digitalWrite(MARK_PIN, ON);
          break;
          case START_NOTE:
            digitalWrite(START_PIN, ON);
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, ON);
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, ON);
          break;
       }
      }
      
      if (rx.header==0x08 && rx.byte1==0x80|MIDI_IN_CHN) {
        //we got a note on event on our midi chn
        switch (rx.byte2) {
          case MARK_NOTE:
            digitalWrite(MARK_PIN, OFF);
          break;
          case START_NOTE:
            digitalWrite(START_PIN, OFF);
          break;
          case FAST_NOTE:
            digitalWrite(FAST_PIN, OFF);
          break;
          case STOP_NOTE:
            digitalWrite(STOP_PIN, OFF);
          break;
       }
      }
      
    }
  } while (rx.header != 0);
}
