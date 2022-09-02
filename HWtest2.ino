#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI 

#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial midi1(27, 1); // RX, TX
//SoftwareSerial midi2(27, 7); // RX, TX
#include <MIDI.h>

MIDI_CREATE_INSTANCE(SoftwareSerial, midi1, midiA);

////スイッチアサイン

int sw[] = {4, 5, 6, 7, 8, 9} ;       //switch 
int led[] = {18, 19, 20, 21, 22, 23} ;      //LED
byte xr12ch[] = {0, 1, 2, 3, 4, 5} ;  //XR12 ch assign
byte osc1[24] = {
  0x2F, 0x63, 0x68, 0x2F, //  /ch/
  0x30, 0x31, 0x2F, 0x6D, //  01/m
  0x69, 0x78, 0x2F, 0x6F, //  ix/o
  0x6E, 0x00, 0x00, 0x00, //  n___
  0x2C, 0x69, 0x00, 0x00, //  ,i__
  0x01, 0x00, 0x00, 0x00  //  1___
  };
byte osc2[24] = {
  0x2F, 0x63, 0x68, 0x2F, //  /ch/
  0x30, 0x31, 0x2F, 0x6D, //  01/m
  0x69, 0x78, 0x2F, 0x6F, //  ix/o
  0x6E, 0x00, 0x00, 0x00, //  n___
  0x2C, 0x69, 0x00, 0x00, //  ,i__
  0x02, 0x00, 0x00, 0x00  //  2___
  };


int swstate = 4;     //unpushed sw count
int swcount = 6;     //total sw count
bool swread[6];      //pushed switch
bool chstate[6];     //muted ch


void setup() {
  u8g.setRot180();

  u8g.firstPage();  
  
  Serial.begin(9600);
//  midi1.begin(31250);   // Set MIDI baud rate:
  midiA.begin(MIDI_CHANNEL_OMNI);
  pinMode(0, OUTPUT);      
  digitalWrite(0, HIGH);  
  
  for (int i = 0; i < swcount; i++){
  pinMode(sw[i], INPUT_PULLUP);    //switch
  pinMode(led[i], OUTPUT);          //LED
  }
  Serial.println("init");
  
  for (int i = 0; i < swcount; i++){
    digitalWrite(led[i], HIGH);   //tarry LED
  }
}

void loop() {
  keyscan();
  MIDIcom();
}

void keyscan(){
  swstate = 0;
  for (int i = 0; i < swcount ; i++){
  swread[i] = digitalRead(sw[i]); 
  swstate += swread[i];
  }
}

void MIDIcom(){
  if (swstate != swcount){
    Serial.println("sw pushed");
    for (int i = 0; i < swcount; i++){
        delay(10);
      if (swread[i] == 0){
        Serial.print("SW = ");
        Serial.print(i);
        if (chstate[i] == 0){
          Serial.println(" OFF");
          digitalWrite(led[i], LOW);   //tarry LED
        chstate[i] += 1;          //latch mute
        XR12mute(i, 0);
//          midiA.sendNoteOn(16, 127, 1);    // Send a Note (pitch 42, velo 127 on channel 1)



        }else if(chstate[i] == 1){
        chstate[i] -= 1;          //latch mute
        XR12mute(i, 1);  
          Serial.println(" ON");
          digitalWrite(led[i], HIGH);   //tarry LED
        }
      }
    }
    for (int i = 0; i < swcount; i++){
//      digitalWrite(LED[i], chstate[i]);   //tarry LED
    }
      delay(400);
  }
}


void u8g_prepare(void) {
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
}

////MIDI COMMNAD

void XR12(int cmd, int pitch, int velocity) {
  midi1.write(cmd);
  midi1.write(pitch);
  midi1.write(velocity);
}
void XR12mute(int cmd ,int mute) {
  if (mute){
  midi1.write(0xB1);
  midi1.write(cmd);
  midi1.write(byte(0)); //mute off
  Serial.println("mute command");
  }
  else{
  midi1.write(0xB1);    //XR18 Ch mute
  midi1.write(cmd);     //mute Ch#
  midi1.write(0x7F);    //mute on
  Serial.println("unmute command");
  }
}

//SP-404SX
// cmd 
//    0x90 = note on
//    0x80 = note off
// pitch
//    0x2F = A1
//    0x3B = B1
// velocity
//    0x00 = min
//    0x7F = max

void note(int cmd, int pitch, int velocity) {
  midi1.write(cmd);
  midi1.write(pitch);
  midi1.write(velocity);
}
void noteOn(int pitch) {
  midi1.write(0x90);
  midi1.write(pitch);
  midi1.write(0x70);
//    XR12muteOn(0x00);
//    XR12muteOn(0x01);
//    XR12muteOn(0x02);
//    XR12muteOn(0x03);
}
void noteOff(int pitch) {
  midi1.write(0x80);
  midi1.write(pitch);
  midi1.write(0x70);
//    XR12muteOff(0x00);
//    XR12muteOff(0x01);
//    XR12muteOff(0x02);
//    XR12muteOff(0x03);
}
