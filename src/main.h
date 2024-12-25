#ifndef MAIN_H
#define MAIN_H

#include "Arduino.h"
void intro();
void sendRGB(byte r, byte g, byte b);
void sendByte(byte b);
void send1();
void send0();

void hsi2rgb(float H, float S, float I, byte* rgb);
void isrA_up();
void isrB_up();
void isrA_down();
void isrB_down();
void up();
void down();
void update();   

#endif
