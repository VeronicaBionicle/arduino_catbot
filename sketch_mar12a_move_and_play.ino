#include "Arduino.h"
#include "SoftwareSerial.h" //библиотека для работы с последовательным портом
#include "DFRobotDFPlayerMini.h"  //библиотека для работы с MP3-плеером
#include "TimerOne.h" //библиотека для работы с прерываниями по таймеру
#include "iarduino_RTC.h" //библиотека для работы с RTC модулем

SoftwareSerial GSMModule(6, 7); // RX, TX порты RX, TX на плате, к которым подключается GSM-модуль
SoftwareSerial DFPlayer(8, 9); //порты RX, TX на плате, к которым подключается MP3-плеер
DFRobotDFPlayerMini Player; //именуем MP3-плеер

int PIRPin = 2; //порт подключения датчика движения
int PIRState = LOW; //переменная состояния датчика движения

volatile unsigned int NoMoveFlag = 0; //флаг отсутствия движения
volatile unsigned int NoMoveTime = 0; //время отсутствия движения
unsigned int NoMoveFlagCopy = 0;  //копия флага отсутствия движения для loop
unsigned int WaitTime = 450; //время бездействия пользователя WaitTime*2 c = 15 минут

iarduino_RTC time(RTC_DS1302, 10, 12, 11); //подключение RTC модуля (выводы RST, CLK, DAT)     
volatile unsigned int FlagSaid = 0;  //флаг для отслеживания сказанных фраз

unsigned int SMSSentFlag = 0; //флаг отправки SMS
String _response = ""; //переменная для хранения ответа GSM-модуля
String Name = "Larisa Ivanovna";  //имя
String Phone_number = "+79638252860"; //номер мобильного телефона

volatile unsigned int FlagSOS = 0;  //флаг срабатывания тревожной кнопки
unsigned int FlagSOSCopy = 0;  //копия флага срабатывания тревожной кнопки для loop

void setup()
{ 
  DFPlayer.begin(9600);
  if (!Player.begin(DFPlayer)) {  //включение плеера
    while(true){delay(0);}
                               } 
  Player.volume(20);  //громкость звука

  pinMode(PIRPin, INPUT); //пин, к которому подключен датчик движение, работает в режиме входа

  Timer1.initialize(1000000);  //опрос датчика каждую секунду

  time.begin(); //запуск RTC модуля

  GSMModule.begin(9600);  //запуск GSM-модуля
  //настройки GSM-модуля
  sendATCommand("AT", true);                //автонастройка скорости
  sendATCommand("ATE0", true);              //выключение режима Echo
  sendATCommand("AT+CMGF=1", true);         //включить TextMode для SMS
  sendATCommand("AT+DDET=1,0,0", true);     //включить DTMF

  attachInterrupt(1, SOS, RISING);  //прерывание по нажатию тревожной кнопки
}

void loop()
{
  noInterrupts(); 
  FlagSOSCopy = FlagSOS;  //передача данных из обработчика прерываний
  interrupts();
    
  if (FlagSOSCopy) {
    Player.play(2);  //проиграть фразу "Вызываю помощь!"
    delay(2500);
    sendSMS(Phone_number, Name+" v opasnosti!");  //отправка SMS
    FlagSOS = 0;
    FlagSOSCopy = 0;};
  
  if (time.Hours >= 9 && time.Hours <= 20) { //прерывание по датчику движения работает только в часы с 9 до 21, чтобы не мешать сну

    Timer1.attachInterrupt(Moving); 
  
    noInterrupts(); 
    NoMoveFlagCopy = NoMoveFlag;  //передача данных из обработчика прерываний
    interrupts();
    
    if (!SMSSentFlag){
      if (NoMoveFlagCopy){  //если движения не было в течение указанного времени WaitTime*2 с
        Player.play(1);  //проиграть фразу "Вы в порядке? Подойдите, пожалуйста!"
        delay(122500); //ожидание движения в течение 2 минут
        noInterrupts(); 
        NoMoveFlagCopy = NoMoveFlag;  //передача данных из обработчика прерываний
        interrupts();
        if (NoMoveFlagCopy){  //если движения не было
          Player.play(2);  //проиграть фразу "Вызываю помощь!"
          delay(2500);
          sendSMS(Phone_number, Name+" v opasnosti!"); //отправка SMS
          SMSSentFlag = 1;
        }
      }
    }    

    if(millis()%1000==0){ //если прошла секунда
      if (time.seconds == 0){
        if (time.Hours == 10 && time.minutes == 0 && FlagSaid == 0){  //утренний будильник
          Player.play(3); //проиграть фразу "Пора вставать!"
          delay(1000);
          Player.play(5); //проиграть фразу "Доброе утро"
          delay(1000);
          SMSSentFlag = 0;}

 
        if (time.Hours == 13 && time.minutes == 0){ //будильник-напоминание о лекарстве
         Player.play(4); //проиграть фразу "Вам пора выпить лекарство."
         delay(2000);
         FlagSaid = 0;
         SMSSentFlag = 0;
        }

        if (time.Hours == 20 && time.minutes == 59){  //пожелание спокойной ночи
         Player.play(6); //проиграть фразу "Спокойной ночи"
         delay(1000);
         FlagSaid = 0;
         SMSSentFlag=0;
         }
          }
        }     
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
       Player.play(5); //проиграть фразу "Доброе утро"
       delay(1000);
       FlagSaid = 1;
    }
  }
  else NoMoveTime++;
  
  if (NoMoveTime > WaitTime){  //если движения не было, флаг отсутствия движения равен 1 
    NoMoveFlag = 1;}
  else NoMoveFlag = 0;
}

String sendATCommand(String cmd, bool waiting) {  //функция для отправки команд GSM-модулю
  String _resp = "";                            //переменная для хранения результата
  GSMModule.println(cmd);                       //отправка команды модулю
  if (waiting) {                                //ожидание ответа от модуля
    _resp = waitResponse();                     
  }
  return _resp;                                 //возвращаем результат. пусто, если проблема
}

String waitResponse() {                         //функция ожидания ответа и возврата полученного результата
  String _resp = "";                            //переменная для хранения результата
  long _timeout = millis() + 10000;             //переменная для отслеживания таймаута (10 секунд)
  while (!GSMModule.available() && millis() < _timeout)  {}; //ожидание ответа 10 секунд
  if (GSMModule.available()) {                     //получение ответа от модуля
    _resp = GSMModule.readString();                
  };
  return _resp;                                 //возвращаем результат. пусто, если проблема
}

void sendSMS(String phone, String message)  //функция отправки SMS
{
  sendATCommand("AT+CMGS=\"" + phone + "\"", true);             //переход в режим ввода текстового сообщения
  sendATCommand(message + "\r\n" + (String)((char)26), true);   //ввод текста сообщения
}

void SOS()  //обработчик прерываний с тревожной кнопки
{
  FlagSOS = 1;
}
