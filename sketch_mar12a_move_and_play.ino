#include "Arduino.h"
#include "SoftwareSerial.h" //библиотека для работы с последовательным портом
#include "DFRobotDFPlayerMini.h"  //библиотека для работы с MP3-плеером
#include "TimerOne.h" //библиотека для работы с прерываниями по таймеру
#include "iarduino_RTC.h" //библиотека для работы с RTC модулем

SoftwareSerial DFPlayer(8, 9); //порты RX, TX на плате, к которым подключается MP3-плеер
DFRobotDFPlayerMini Player; //именуем MP3-плеер

int PIRPin = 2; //порт подключения датчика движения
int PIRState = LOW; //переменная состояния датчика движения

volatile unsigned int NoMoveFlag = 0; //флаг отсутствия движения
volatile unsigned int NoMoveTime = 0; //время отсутствия движения
unsigned int NoMoveFlagCopy;  //копия флага отсутствия движения для loop
unsigned int WaitTime = 5; //время бездействия пользователя WaitTime*10 = c

iarduino_RTC time(RTC_DS1302, 10, 12, 11); //подключение RTC модуля (выводы RST, CLK, DAT)     
volatile unsigned int FlagSaid = 0;  //флаг для отслеживания сказанных фраз

void setup()
{
 Serial.begin(9600);
  
  DFPlayer.begin(9600);
  if (!Player.begin(DFPlayer)) {  //включение плеера
    while(true){delay(0);}
                               } 
  Player.volume(20);  //громкость звука

  pinMode(PIRPin, INPUT); //пин, к которому подключен датчик движение, работает в режиме входа

  Timer1.initialize(10000000);  // опрос датчика каждые 10 секунд

  time.begin(); //запуск RTC модуля
}

void loop()
{
  if (time.Hours >= 9 && time.Hours <= 21) { //прерывание по датчику движения работает только в часы с 9 до 21, чтобы не мешать сну

    Timer1.attachInterrupt(Moving); 
  
    noInterrupts(); 
    NoMoveFlagCopy = NoMoveFlag;  //передача данных из обработчика прерываний
    interrupts();

    if (NoMoveFlagCopy){  //если движения не было в течение указанного времени WaitTime*8 с
      Player.play(2);  //проиграть фразу "Вы в порядке? Подойдите, пожалуйста!"
      delay(3000);
                       }
 
    if (time.Hours == 10 && time.minutes == 0 && FlagSaid == 0){
      Player.play(4); //проиграть фразу "Пора вставать!"
      delay(3000);
      FlagSaid = 1;}

 
    if (time.Hours == 15 && time.minutes == 0 && FlagSaid == 1){
      Player.play(1); //проиграть фразу "Вам пора выпить лекарство."
      delay(3000);
      FlagSaid = 2;}

    if (time.Hours == 21 && time.minutes == 59 && FlagSaid == 2){
      Player.play(5); //проиграть фразу "Спокойной ночи"
      delay(3000);
      FlagSaid = 0;}
          } else Timer1.detachInterrupt();
}

void Moving(void) //обработчик прерываний
{
  if (digitalRead(PIRPin) == HIGH){ //сохранение состояния датчика движения
    if (PIRState == LOW) PIRState = HIGH;}
  else {if (PIRState == HIGH)PIRState = LOW;}
  
  if (PIRState == HIGH){  //подсчет времени без движения
    NoMoveTime = 0;
    if(time.Hours == 9 && time.minutes <= 59 && FlagSaid == 0){  //если проснуться раньше будильника, робот пожелает доброго утра
       Player.play(6); //проиграть фразу "Доброе утро"
       delay(3000);
       FlagSaid = 1;
    }
  }
  else NoMoveTime++;
  
  if (NoMoveTime > WaitTime){  //если движения не было, флаг отсутствия движения равен 1 
    NoMoveFlag = 1;}
  else NoMoveFlag = 0;
}
