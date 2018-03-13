#include "Arduino.h"
#include "SoftwareSerial.h" //библиотека для работы с последовательным портом
#include "DFRobotDFPlayerMini.h"  //библиотека для работы с MP3-плеером
#include "TimerOne.h" //библиотека для работы с прерываниями по таймеру

SoftwareSerial DFPlayer(8, 9); //порты RX, TX на плате, к которым подключается MP3-плеер
DFRobotDFPlayerMini Player; //именуем MP3-плеер

int PIRPin = 2; //порт подключения датчика движения
int PIRState = LOW; //переменная состояния датчика движения

volatile unsigned int NoMoveFlag = 0; //флаг отсутствия движения
volatile unsigned int NoMoveTime = 0; //время отсутствия движения
unsigned int NoMoveFlagCopy;  //копия флага отсутствия движения для loop
unsigned int WaitTime = 5; //время бездействия пользователя WaitTime*10 = c

void setup()
{
  DFPlayer.begin(9600);
  if (!Player.begin(DFPlayer)) {  //Включение плеера
    while(true){delay(0);}
                               } 
  Player.volume(20);  //Громкость звука

  pinMode(PIRPin, INPUT); //пин, к которому подключен датчик движение, работает в режиме входа

  Timer1.initialize(10000000);  // опрос датчика каждые 10 секунд
  Timer1.attachInterrupt(Moving); //при прерывании по Таймеру 1 начинает работать функция Moving
}

void loop()
{
  noInterrupts(); 
  NoMoveFlagCopy = NoMoveFlag;  //передача данных из обработчика прерываний
  interrupts();

  if (NoMoveFlagCopy){  //если движения не было в течение указанного времени WaitTime*8 с
    Player.play(4);  //проиграть фразу "Вы в порядке? Подойдите, пожалуйста!"
    delay(3000);
                   }
}

void Moving(void) //обработчик прерываний
{
  if (digitalRead(PIRPin) == HIGH){ //сохранение состояния датчика движения
    if (PIRState == LOW) PIRState = HIGH;}
  else {if (PIRState == HIGH)PIRState = LOW;}
  
  if (PIRState == HIGH){  //подсчет времени без движения
    NoMoveTime = 0;}
  else NoMoveTime++;
  
  if (NoMoveTime > WaitTime){  //если движения не было, флаг отсутствия движения равен 1 
    NoMoveFlag = 1;}
  else NoMoveFlag = 0;
}
