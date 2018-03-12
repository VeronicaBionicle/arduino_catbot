#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial DFPlayer(8, 9); // RX, TX
DFRobotDFPlayerMini Player;
int PIRPin = 2;
int PIRState;
void setup()
{
  DFPlayer.begin(9600);
  if (!Player.begin(DFPlayer)) {  //Use softwareSerial to communicate with mp3.
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  } 
  Player.volume(20);  //Set volume value. From 0 to 30

  pinMode(PIRPin, INPUT);
}

void loop()
{
  PIRState = digitalRead(PIRPin);
  if (PIRState == HIGH){
  Player.play(4);  //проиграть фразу "Вы в порядке? Подойдите, пожалуйста!"
  delay(3000);
  }
}


