#include "Arduino.h"
#include "main.h"
#include "math.h"
#define DEG_TO_RAD(X) (M_PI*(X)/180)
#define LED_DATA 5
#define BUTTON 4
#define DEBUG false
#define NUM_LEDS 25

byte rgb[3];

volatile int mode=0;
volatile float hue=0;
volatile float saturation=1;
volatile float intensity=0.1;

volatile bool A = false;
volatile bool B = false;
volatile unsigned int count = 0;
volatile bool button_down=false;

void setup() {
  mode=2;
  hue=0;
  saturation=1;
  intensity=0.05;
  pinMode(LED_DATA, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(2, INPUT);
  pinMode(3, INPUT);

  // Pin 2 interrupt
  A=((PIND & B00000100) == B00000100);
  if (A) {
    attachInterrupt(0, isrA_down, FALLING);
  } else {
    attachInterrupt(0, isrA_up, RISING);  
  }

  // Pin 3 interrupt
  B=((PIND & B00001000) == B00001000);
  if (B) {
    attachInterrupt(1, isrB_down, FALLING); 
  } else {
    attachInterrupt(1, isrB_up, RISING);  
  }
  
  update();
  if (DEBUG) {
    Serial.begin(115200);
  }
}


 
void isrA_up() {
  A=true;
  if (B) {
    down();
  } else {
    up();
  }
  attachInterrupt(0, isrA_down, FALLING);
}

void isrB_up() {
  B=true;
  if (A) {
    up();
  } else {
    down();
  }
  attachInterrupt(1, isrB_down, FALLING);
}

void isrA_down() {
  A=false;
  if (B) {
    up();
  } else {
    down();
  }
  attachInterrupt(0, isrA_up, RISING);
}

void isrB_down() {
  B=false;
  if (A) {
    down();
  } else {
    up();
  }
  attachInterrupt(1, isrB_up, RISING);
}

void up() {
  switch(mode) {
    case 0: {
      hue+=2;
      if (hue>360) {
        hue-=360;
      }
      break;
    }
    case 1: {
      saturation+=0.01;
      if (saturation>1) {
        saturation=1;
      }
      break;
    }
    case 2: {
      intensity+=0.005;
      if (intensity>0.8) {
        intensity=0.8;
      }
      break;
    }
  }
  
  update();
}

void down()  {
  switch(mode) {
    case 0: {
      hue-=2;
      if (hue<0) {
        hue+=360;
      }
      break;
    }
    case 1: {
      saturation-=0.01;
      if (saturation<0) {
        saturation=0;
      }
      break;
    }
    case 2: {
      intensity-=0.005;
      if (intensity<0) {
        intensity=0;
      }
      break;

      break;
    }
  }
  
  update();
}




void loop() {
  if (digitalRead(BUTTON)==LOW) {
    mode+=1;
    mode%=4;
    while(digitalRead(BUTTON)==LOW) {
    }  
    update();
    delay(5);
  }
}

void update() {
  if (mode==3) {
    rgb[0]=0;
    rgb[1]=0;
    rgb[2]=0;
  } else {
    hsi2rgb(hue,saturation,intensity,&rgb[0]);
  }
  noInterrupts();
  intro();
  for (int i=0;i<NUM_LEDS;i++) {
      sendRGB(rgb[0],rgb[1],rgb[2]);
  }
  interrupts();
  if (DEBUG) {
    Serial.print("Hue:");
    Serial.print(hue);
    Serial.print(", Sat:");
    Serial.print(saturation);
    Serial.print(", Int:");
    Serial.print(intensity);
    Serial.println();
    Serial.print("Mode: ");
    Serial.println(mode);
  }
}

void intro() {
  delayMicroseconds(6);
}

void sendRGB(byte r, byte g, byte b) {
  sendByte(g);
  sendByte(r);
  sendByte(b);
}

void sendByte(byte b) {
  for (int i = 0; i < 8; i++) {
    if ((b & B10000000) != 0) {
      send1();
    } else {
      send0();
    }
    b <<= 1;
  }
}

void send1() {
  PORTD |= B00100000; //set PIN HIGH, 1 cycle, now wait 700ns
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);

  PORTD &= B11011111;   //set PIN LOW, 1 cycle, now wait 600ns


}
void send0() {
  PORTD |= B00100000; //set PIN HIGH, 1 cycle, now wait 350ns
  asm volatile ("nop"::); //uses 1 cycle, thus takes about 62.5 ns
  asm volatile ("nop"::);
  asm volatile ("nop"::);



  PORTD &= B11011111; //set PIN LOW, 1 cycle, now wait 800ns
  asm volatile ("nop"::);
  asm volatile ("nop"::);
  asm volatile ("nop"::);
}


void hsi2rgb(float H, float S, float I, byte* rgb) {
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;
    
  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439) {
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    b = 255*I/3*(1-S);
  } else if(H < 4.188787) {
    H = H - 2.09439;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    g = 255*I/3*(1-S);
  }
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
}




void hsi2rgbw(float H, float S, float I, int* rgbw) {
  int r, g, b, w;
  float cos_h, cos_1047_h;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;
  
  if(H < 2.09439) {
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    r = S*255*I/3*(1+cos_h/cos_1047_h);
    g = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    b = 0;
    w = 255*(1-S)*I;
  } else if(H < 4.188787) {
    H = H - 2.09439;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    g = S*255*I/3*(1+cos_h/cos_1047_h);
    b = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    r = 0;
    w = 255*(1-S)*I;
  } else {
    H = H - 4.188787;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    b = S*255*I/3*(1+cos_h/cos_1047_h);
    r = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    g = 0;
    w = 255*(1-S)*I;
  }
  
  rgbw[0]=r;
  rgbw[1]=g;
  rgbw[2]=b;
  rgbw[3]=w;
}