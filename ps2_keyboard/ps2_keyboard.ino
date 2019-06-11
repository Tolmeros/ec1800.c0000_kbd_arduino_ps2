#include <ps2dev.h>
#include <PS2KeyCode.h>
//#include <PS2KeyTable.h>

#define PS2_LED_SCROLL  B00000001
#define PS2_LED_NUM     B00000010
#define PS2_LED_CAPS    B00000100

#define DD3_A0  2
#define DD3_A1  3
#define DD3_A2  4
#define DD3_A3  5
#define DD3_nE1 13

#define DD5_S0  6
#define DD5_S1  7
#define DD5_S2  8
#define DD5_nQ  12

#define VD93    9
#define VD95    10
#define VD94    11

#define NumLockLed  VD94
#define LatLed  VD95
#define RusLed  VD93

#define myCodeSHIFT -1
#define myCodeENTER 13
#define myCodeBACKSPACE -2

#define ps2device_data    A0
#define ps2device_clock   A1

PS2dev ps2kbd_dev(ps2device_clock, ps2device_data);

const char layout[6][16] = {
  {0,0,'1','2','3','4','5','6','7','8','9','0','-','=','\\',myCodeBACKSPACE},
  {0,'q','w','e','r','t','y','u','i','o','p','{','}','/',0},
  {'a','s','d','f','g','h','j','k','l','[',']',';',':',myCodeENTER, 0, myCodeSHIFT},
  {'z','x','c','v','b','n','m','`','~','ё',',', '.',myCodeSHIFT},
  {0,' ',},
  {}
};

const byte scancodes2[6][16] = {
  {0, PS2_KC_ESC, PS2_KC_1, PS2_KC_2, PS2_KC_3, PS2_KC_4, PS2_KC_5, PS2_KC_6, 
    PS2_KC_7, PS2_KC_8, PS2_KC_9, PS2_KC_0, PS2_KC_MINUS, PS2_KC_EQUAL, PS2_KC_BACK , PS2_KC_BS},
    
  {PS2_KC_TAB, PS2_KC_Q, PS2_KC_W, PS2_KC_E, PS2_KC_R, PS2_KC_T, PS2_KC_Y, PS2_KC_U, PS2_KC_I,
   PS2_KC_O, PS2_KC_P, PS2_KC_OPEN_SQ, PS2_KC_CLOSE_SQ, PS2_KC_DIV, 0, PS2_KC_CTRL},
   
  {PS2_KC_A, PS2_KC_S, PS2_KC_D, PS2_KC_F, PS2_KC_G, PS2_KC_H, PS2_KC_J, PS2_KC_K, PS2_KC_L, PS2_KC_SEMI ,
   PS2_KC_APOS,0,0, PS2_KC_ENTER, PS2_KC_ALT, PS2_KC_L_SHIFT },
   
  {PS2_KC_Z, PS2_KC_X, PS2_KC_C, PS2_KC_V, PS2_KC_B, PS2_KC_N, PS2_KC_M, PS2_KC_COMMA, PS2_KC_DOT, PS2_KC_SINGLE, 0, 0,
   PS2_KC_R_SHIFT, PS2_KC_CAPS, PS2_KC_CAPS, 0},
   
  {0, PS2_KC_SPACE, 0, 0, PS2_KC_F1, PS2_KC_F2, PS2_KC_F3, PS2_KC_F4, PS2_KC_F5, PS2_KC_F6, PS2_KC_F7, PS2_KC_F8, PS2_KC_F9,
   PS2_KC_F10, PS2_KC_NUM, 0},
   
  {PS2_KC_KP_MINUS, PS2_KC_KP7, PS2_KC_KP8, PS2_KC_KP9, PS2_KC_KP4, PS2_KC_KP5, PS2_KC_KP6, PS2_KC_KP1, PS2_KC_KP2, PS2_KC_KP3,
   PS2_KC_KP0, PS2_KC_KP_DOT, PS2_KC_KP_PLUS}
};

byte kstate[6][16];

boolean ps2kbd_send_enabled = false;
//byte ScanCodeSet = 0xFF;
byte ScanCodeSet = 0x02;
byte ps2Leds = 0x00;
byte ps2Typematic =0x00;

void ack()
{
  //acknowledge commands
  while(ps2kbd_dev.write(PS2_KC_ACK));
}

int keyboardcommand(int command)
{
  byte val;
  int s;
  byte n;
  switch (command)
  {
  case PS2_KC_RESET:
    ack();
    //the while loop lets us wait for the host to be ready
    while(ps2kbd_dev.write(0xAA)!=0);
    Serial.println(F("PS/2 GET RESET"));
    break;
  case PS2_KC_RESEND:
    ack();
    break;
  case PS2_KC_DEFAULTS: //set defaults
    //enter stream mode
    ack();
    Serial.println(F("PS/2 SET DEFAULTS"));
    break;
  case PS2_KC_DISABLE:
    //FM
    ps2kbd_send_enabled = false;
    ack();
    break;
  case PS2_KC_ENABLE: //enable data reporting
    //FM
    ps2kbd_send_enabled = true;
    ack();
    break;
  case PS2_KC_RATE: //set typematic rate
    ack();
    ps2kbd_dev.read(&val); //do nothing with the rate
    ack();
    ps2Typematic = val;
    Serial.println(F("PS/2 SET RATE"));
    break;
  case PS2_KC_READID:
    ack();
    // MF2 keyboard
    ps2kbd_dev.write(0xAB);
    ps2kbd_dev.write(0x83);
    Serial.println(F("PS/2 READID"));
    break;
  case PS2_KC_SCANCODE: //set/get scan code set
    ack();
    ps2kbd_dev.read(&val); //do nothing with the rate
    ack();
    if (val == 0) {
      while(ps2kbd_dev.write(ScanCodeSet)!=0);
      Serial.println(F("PS/2 GET SCANCODE"));
    } else {
      ScanCodeSet = val;
      Serial.println(F("PS/2 SET SCANCODE"));
    }
    break;
  case PS2_KC_ECHO: //echo
    //ack();
    ps2kbd_dev.write(PS2_KC_ECHO);
    Serial.println(F("PS/2 ECHO"));
    break;
  case PS2_KC_LOCK: //set/reset LEDs
    ack();
    s = -1; //do nothing with the rate
    n = 20;
    while ((s!=0) && (n>0)) {
      s = ps2kbd_dev.read(&val);
      Serial.println(s);
      n--;
    }
    
    if (s==0) {
      ack();
      ps2Leds = val;
    };
    Serial.println(F("PS/2 LOCK"));
    break;
   default:
    ack();
    //ps2kbd_dev.write(PS2_KC_RESEND);
    break;
  }
}

void updateLeds() {
  digitalWrite(LatLed, ps2Leds & PS2_LED_CAPS);
  digitalWrite(RusLed, ps2Leds & PS2_LED_SCROLL);
  digitalWrite(NumLockLed, ps2Leds & PS2_LED_NUM);
}



void setup() {
  // put your setup code here, to run once:
  pinMode(DD3_nE1,OUTPUT);
  digitalWrite(DD3_nE1, HIGH);
  
  pinMode(DD3_A0,OUTPUT);
  pinMode(DD3_A1,OUTPUT);
  pinMode(DD3_A2,OUTPUT);
  pinMode(DD3_A3,OUTPUT);

  pinMode(DD5_S0,OUTPUT);
  pinMode(DD5_S1,OUTPUT);
  pinMode(DD5_S2,OUTPUT);

  pinMode(VD93,OUTPUT);
  pinMode(VD95,OUTPUT);
  pinMode(VD94,OUTPUT);
  
  Serial.begin(115200);
  delay(1500);
  Serial.println(F("EC1840.C000"));


  while(ps2kbd_dev.write(0xAA)!=0);
  delay(10);
  ps2kbd_send_enabled = true;
}

void set_dd5(byte n) {
  digitalWrite(DD5_S0, n & B001);
  digitalWrite(DD5_S1, n & B010);
  digitalWrite(DD5_S2, n & B100);
}

void set_dd3(byte n) {
  digitalWrite(DD3_A0, n & B0001);
  digitalWrite(DD3_A1, n & B0010);
  digitalWrite(DD3_A2, n & B0100);
  digitalWrite(DD3_A3, n & B1000);
}

void print_keyboard_state() {
  Serial.println("Scan codes set " + String(ScanCodeSet) + ".");
  Serial.println("Leds "+ String(ps2Leds, BIN) + ".");
  Serial.println("Typematic "+ String(ps2Typematic, BIN) + ".");
  Serial.println("ps2kbd_send_enabled "+ String(ps2kbd_send_enabled) + ".");
}

void loop() {
  byte state;
  char tmp;
  unsigned char c;

  if( (digitalRead(ps2device_clock)==LOW) || (digitalRead(ps2device_data) == LOW))
  {
    while(ps2kbd_dev.read(&c)) ;
    keyboardcommand(c);
    print_keyboard_state();
  }
  else if (ps2kbd_send_enabled) {
    for (byte dd5 = 0; dd5 < 6; dd5++) {
      set_dd5(dd5);
      for (byte dd3 = 0; dd3 < 16; dd3++) {
        set_dd3(dd3);
        digitalWrite(DD3_nE1, LOW);
        // delay(2);
        // delay(1);
        state = digitalRead(DD5_nQ);
        digitalWrite(DD3_nE1, HIGH);
  
        if ((not state) and (kstate[dd5][dd3] == 0)) {
          
          kstate[dd5][dd3] = 1;

          tmp = scancodes2[dd5][dd3];
          if (tmp > 0) {
            ps2kbd_dev.write(tmp);
          }
          
          tmp = layout[dd5][dd3];

          if (tmp == myCodeENTER) {
            Serial.println("[ENTER]");
          }
          
        };
        
        if (state and (kstate[dd5][dd3] > 0)) {
          kstate[dd5][dd3] = 0;

          tmp = scancodes2[dd5][dd3];
          if (tmp > 0) {
            ps2kbd_dev.write(PS2_KC_KEYBREAK);
            ps2kbd_dev.write(tmp);
          }
          

          tmp = layout[dd5][dd3]; 

          
          if ((tmp > 20) and (tmp < 127)) { 
            Serial.print("[");
            Serial.print(tmp);
            Serial.println("]");
          } else if (tmp == myCodeBACKSPACE) {
            // Serial.print(char(127));
            Serial.println("[BS]");
          }
        }
  
        updateLeds();
      }
    }
    
  }

  
  
  

}
