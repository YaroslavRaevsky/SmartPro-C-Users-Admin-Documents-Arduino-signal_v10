
#include <GyverHub.h>  // Подключаем библиотеку для билдера
#include <WiFi.h> // Подключаем библиотеку для WiFi модуля
#include <WebServer.h> // Подключаем библиотеку для отиображения html web страницы
#include <Arduino.h>  // Подключаем библиотеку для платформы
#include <AsyncTCP.h>  // Подключаем библиотеку для обмена данными по сети через протоколы TCP
#include <EEPROM.h>  // Подключаем библиотеку для эмулированной EEPROM памяти во Flash
#include <ESPAsyncWebServer.h>  // Подключаем библиотеку для отиображения html web страницы
#include <RCSwitch.h>  // Подключаем библиотеку для радиомодуля 433 МГц

AsyncWebServer server(80); //номер порта сервера на ESP32
TaskHandle_t Task2; //объявление задачи для FreeRTOS
RCSwitch mySwitch = RCSwitch(); //объявление имени структуры для работы с радиомодулем 433 МГц 

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "ALARM_WiFi";  //имя сети в режиме точки доступа
const char* password = "123456789"; //пароль от сети в режиме точки доступа
//const char* password = "super_strong_password";

const char* PARAM_STRING_1 = "inputString1";  //обявление и инициализация строки для сохранения имени сети в EEPROM
const char* PARAM_STRING_2 = "inputString2";  //обявление и инициализация строки для сохранения пароля в EEPROM

char data1[200];  //обявление массива для сохранения имени сети в EEPROM
char data2[200];  //обявление массива для сохранения пароля в EEPROM
char ssid_STA[200];   //обявление массива для сохранения имени сети в EEPROM
char password_STA[200];  //обявление массива для сохранения пароля в EEPROM
char str[20];  //обявление массива для сохранения имени сети и пароля из формы web страницы
int i = 0;  //обявление и инициализация перменной для счётчика

GyverHub hub("MyDevices", "Сигнализация 2", "");  // префикс, имя, иконка
//Привязка имён пинам
#define SWITCH 22 //переключатель режимов STA-AP
#define LED_AP 2 //идикатор режима AP
#define LED_STA 15  //индикатор режима STA
#define S0_2 26 //пин управлния мультиплексором 
#define S1_2 27 //пин управлния мультиплексором 
#define S2_2 33 //пин управлния мультиплексором 
#define S3_2 25 //пин управлния мультиплексором 
#define EN_2 34 //пин управлния мультиплексором 
#define SIG_2 19 //пин управлния мультиплексором 
#define batPin 32 //вход АЦП для контроля уровня заряда аккумулятора
#define chrPin 14 //вход для контроля факта работы внешнего блока питания
#define SPK 23  //выход для включения сирены
#define spkLvlPin 35  //вход АЦП для контроля уровня громкости сирены
const int SER = 16; //выход для последовательного вывода на сдвиговые регистры
const int LATCH = 5; //защёлка для управления сдвиговыми регистрами
const int CLK = 18; //выход тактирования сдвиговых регистров
//Объявление и иницилизация переменных, массивов и строк
int LedOut1 = 0, LedOut2 = 0; //буфер для свиговых регистров
int Pins[16]; //массив для заполнения значениями входов мультиплексора
int batValue, batValue100 = 0;  //уровень заряда аккуммулятора в % 
int spkLevel, spkLevel100 = 0;  //уровень громкости сирены в %
char level[] = "100 %"; //уровни по умолчанию
bool chrFlag = 0; //факт работы зарядного устройства
bool AlarmFlag = 0; //единица только в случае срабатывания сирены
bool leds[16];  //индикаторы номеров подключенных датчиков
bool s0, s1, s2, s3;  //переменные для управления мультиплексором
bool flag1 = LOW, flag2 = LOW, flag3 = LOW, flag4 = LOW, flag5 = LOW, flag6 = LOW, flag7 = LOW, flag8 = LOW, flag9 = LOW, flag10 = LOW, flag11 = LOW, flag12 = LOW, flag13 = LOW, flag14 = LOW, flag15 = LOW, flag16 = LOW; //флаги для фиксирования состояния датчика в случае срабатывания
bool alarm1 = LOW, alarm2 = LOW, alarm3 = LOW, alarm4 = LOW, alarm5 = LOW, alarm6 = LOW, alarm7 = LOW, alarm8 = LOW, alarm9 = LOW, alarm10 = LOW, alarm11 = LOW, alarm12 = LOW, alarm13 = LOW, alarm14 = LOW, alarm15 = LOW, alarm16 = LOW; //флаги срабатывания тревоги
//bool LED1status = LOW;
//bool LED2status = LOW;
//bool SENSOR3status = LOW;
bool sens1, sens2, sens3, sens4, sens5, sens6, sens7, sens8, sens9, sens10, sens11, sens12, sens13, sens14, sens15, sens16; // переменные состояния датчиков
GHbutton rst; // обновление виджетов кнопок
// Страница html отображаемая в режиме STA(сервер) 
// HTML web page to handle 2 input fields (inputString++, inputInt-, inputFloat-)

// Далее комментарии для строк соответственно:
//объявление и инициализация строки с html страницей
//тип документа для браузера
//заголовок страницы
//размер и масштаб страницы
//форма для заполнения имени сети
//запись имени сети в переменную
//кнопка для записи имени сети в переменную и перезагрузки страницы
//пробел
//вывод текущего имени сети на страницу
//пробел
//пробел
//пробел
//конец формы
//форма для заполнения пароля сети
//запись пароля сети в переменную
//кнопка для записи пароля сети в переменную и перезагрузки страницы
//пробел
//вывод текущего пароля сети на страницу
//конец формы
//оформление странцы
//конец строки с html страницей
const char index_html[] PROGMEM = R"rawliteral( 
<!DOCTYPE HTML><html><head> 
  <title>ALARM WiFi input</title> 
  <meta name="viewport" content="width=device-width, initial-scale=20"> 

  <form action="/get" target="hidden-form"> 
    WiFi name <input type="text" name="inputString1"> 
    <input type="submit" value="Submit WiFi name" onclick="document.location.reload(true)"> 
    <p class="margin-bottom-20"></p>  
    saved WiFi name: %inputString1%   
    <p class="margin-bottom-20"></p>  
    <p class="margin-bottom-20"></p>  
    <p class="margin-bottom-20"></p>  
  </form><br> 
  <form action="/get" target="hidden-form"> 
    Password <input type="text" name="inputString2"> 
    <input type="submit" value="Submit Password" onclick="document.location.reload(true)"> 
    <p class="margin-bottom-20"></p>  
    saved Password: %inputString2%   
  </form><br> 

  <iframe style="display:none" name="hidden-form"></iframe> 
</body></html>)rawliteral";

// билдер
void build() {  //запуск билдера
hub.BeginWidgets(); //запуск виджетов
hub.WidgetSize(70); //выбор размера виджетов
hub.Label_(F("Advertisement"), "В случае тревоги нажать кнопку СБРОС", F("."), GH_DEFAULT, 45); //вывод текста с высотой 45
hub.WidgetSize(30); //выбор размера виджетов
if (hub.Button_(F("Reset"), &rst, F("Сброс"), GH_ORANGE, 30)) handle_reset(); //кнопка Сброс
hub.WidgetSize(20); //выбор размера виджетов
hub.SwitchText_(F("sw1"), &sens1, F("Датчик №1 подключен"), F("№1"), GH_DEFAULT); //индикатор подключения датчика 
hub.LED_(F("led1"), 1, "ТРЕВОГА №1"); //индикатор срабатывания тревоги от датчика
hub.Space(); //пробел
hub.SwitchText_(F("sw9"), &sens9, F("Датчик №9 подключен"), F("№9"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led9"), 1, "ТРЕВОГА №9"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw2"), &sens2, F("Датчик №2 подключен"), F("№2"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led2"), 1, "ТРЕВОГА №2"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw10"), &sens10, F("Датчик №10 подключен"), F("№10"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led10"), 1, "ТРЕВОГА №10"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw3"), &sens3, F("Датчик №3 подключен"), F("№3"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led3"), 1, "ТРЕВОГА №3"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw11"), &sens11, F("Датчик №11 подключен"), F("№11"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led11"), 1, "ТРЕВОГА №11"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw4"), &sens4, F("Датчик №4 подключен"), F("№4"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led4"), 1, "ТРЕВОГА №4"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw12"), &sens12, F("Датчик №12 подключен"), F("№12"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led12"), 1, "ТРЕВОГА №12"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw5"), &sens5, F("Датчик №5 подключен"), F("№5"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led5"), 1, "ТРЕВОГА №5"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw13"), &sens13, F("Датчик №13 подключен"), F("№13"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led13"), 1, "ТРЕВОГА №13"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw6"), &sens6, F("Датчик №6 подключен"), F("№6"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led6"), 1, "ТРЕВОГА №6"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw14"), &sens14, F("Датчик №14 подключен"), F("№14"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led14"), 1, "ТРЕВОГА №14"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw7"), &sens7, F("Датчик №7 подключен"), F("№7"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led7"), 1, "ТРЕВОГА №7"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw15"), &sens15, F("Датчик №15 подключен"), F("№15"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led15"), 1, "ТРЕВОГА №15"); //индикатор срабатывания тревоги от датчика
hub.SwitchText_(F("sw8"), &sens8, F("Датчик №8 подключен"), F("№8"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led8"), 1, "ТРЕВОГА №8"); //индикатор срабатывания тревоги от датчика
hub.Space();  //пробел
hub.SwitchText_(F("sw16"), &sens16, F("Датчик №16 подключен"), F("№16"), GH_DEFAULT); //индикатор подключения датчика
hub.LED_(F("led16"), 1, "ТРЕВОГА №16"); //индикатор срабатывания тревоги от датчика
hub.WidgetSize(70); //выбор размера виджетов
hub.Label_(F("Sound"), "Уровень громкости", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
hub.WidgetSize(30); //выбор размера виджетов
hub.Label_(F("Sound_lvl"), "100 %", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
hub.WidgetSize(70); //выбор размера виджетов
hub.Label_(F("Bat"), "Уровень заряда аккумулятора", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
hub.WidgetSize(30); //выбор размера виджетов
hub.Label_(F("Bat_lvl"), "100 %", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
hub.WidgetSize(70); //выбор размера виджетов
hub.Label_(F("Charge"), "Наличие питания от сети", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
hub.WidgetSize(30); //выбор размера виджетов
hub.Label_(F("Charge_flag"), "Да", F("."), GH_DEFAULT, 30); //вывод текста с высотой 30
} //конец билдера

void setup() {   //действия, выплняемые однокрано при запуске микроконроллера
  mySwitch.enableReceive(digitalPinToInterrupt(0)); //выбор входа для считывания пакетов с приёмника 433 МГц
  pinMode(SPK, OUTPUT); // назначение вывода сирены на выход
  pinMode(SWITCH, INPUT_PULLUP);  // назначение вывода переключателя на вход с поддтяжкой к питанию
  pinMode(LED_AP, OUTPUT);  // назначение вывода идикатора режима AP на выход
  pinMode(LED_STA, OUTPUT);  // назначение вывода идикатора режима STA на выход

  pinMode(SER, OUTPUT);  // назначение последовательного вывода данных для сдвиговых регистров на выход
  pinMode(LATCH, OUTPUT);  // назначение вывода управления сдвиговыми регистрами на выход
  pinMode(CLK, OUTPUT);  // назначение вывода тактирования сдвиговых регистров на выход

  pinMode(EN_2, OUTPUT);  // назначение вывода управления мультиплексором на выход
  pinMode(S0_2, OUTPUT);  // назначение вывода управления мультиплексором на выход
  pinMode(S1_2, OUTPUT);  // назначение вывода управления мультиплексором на выход
  pinMode(S2_2, OUTPUT);  // назначение вывода управления мультиплексором на выход
  pinMode(S3_2, OUTPUT);  // назначение вывода управления мультиплексором на выход
  pinMode(SIG_2, INPUT_PULLUP);  // назначение вывода, подключённого к выходу мультиплексора на вход с поддтяжкой к питанию
  //Serial.begin(115200); //запуск UART
  EEPROM.begin(512);  // Инициализация EEPROM с размером 512 байт
  delay(100); // Задержка 100 мС

  if (digitalRead(SWITCH)){ //если переключатель режимов замкнут
  digitalWrite(LED_AP, HIGH); //зажечь индикатор режима AP
  digitalWrite(LED_STA, LOW); //погасить индикатор режима STA

  //Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP); //включить WiFi в режиме AP(точка доступа)
  WiFi.softAP(ssid, password);  //ипользует прописанные в коде программы имясети и пароль
 // Serial.print("[+] AP Created with IP Gateway ");
 // Serial.println(WiFi.softAPIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){  //запуск сервера с html страницей и запросами
    request->send_P(200, "text/html", index_html, processor); //отправление html страницы
  }); 

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {  //запуск сервера с html страницей и запросами
    String inputMessage;  //объявление переменной для записи имени сети и пароля со страницы
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING_1)) {  //если отправлено значение со страницы
      inputMessage = request->getParam(PARAM_STRING_1)->value();  // записать в переменную считанное имя сети
      EEPROM.put(0, inputMessage);   //Запись имени сети в памяти, начиная с адреса 0
      EEPROM.commit();   // Сохранение изменений в памяти
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_STRING_2)) {  //если отправлено значение со страницы
      inputMessage = request->getParam(PARAM_STRING_2)->value();  // записать в переменную считанный пароль
      //writeFile(SPIFFS, "/inputString2.txt", inputMessage.c_str());
      EEPROM.put(255, inputMessage);   //Запись пароля в памяти, начиная с адреса 255
      EEPROM.commit();   // Сохранение изменений
    }

    else {  //если ничего не отправлено
      inputMessage = "No message sent"; //записать строку в переменную
    }
    Serial.println(inputMessage); //вывести сообщение для отладки
    request->send(200, "text/text", inputMessage); //отправить на страницу сообщение  No message sent
  });
  server.onNotFound(notFound);  //отправить на страницу сообщение  notFound
  server.begin();  //запуск сервера
  }
  else{ //если переключатель режимов разомкнут
  digitalWrite(LED_STA, HIGH);  //зажечь индикатор режима STA
  digitalWrite(LED_AP, LOW);  //погасить индикатор режима AP
  Serial.println("Connecting to "); //Вывести на ПК сообщение для отладки

  EEPROM.get(0, data1); //считать из EEPROM в переменную
  ssid_STA[0] = '"';  //записать кавычки первым элементом массива имени сети
  for(i = 0; i<= strlen(data1); i++){ //счётчик
  ssid_STA[i+1] = data1[i]; //запись каждого символа имени сети со сдвигом на 1
  }
  ssid_STA[i] = '"';  //записать кавычки последним элементом массива имени сети
//Serial.print(ssid_STA);

  EEPROM.get(255, data2); //считать из EEPROM в переменную
  password_STA[0] = '"';  //записать кавычки первым элементом массива пароля сети
  for(i = 0; i<= strlen(data2); i++){ //счётчик
  password_STA[i+1] = data2[i]; //запись каждого символа пароля сети со сдвигом на 1
  }
  password_STA[i] = '"';  //записать кавычки последним элементом массива пароля сети
//Serial.print(password_STA);

  WiFi.mode(WIFI_STA);  //запуск WiFi модуля в режиме STA (в качестве клиента)
  WiFi.begin(data1, data2); //указываем имя сети и пароль из памяти EEPROM
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) { //если WiFi соединение не установлено, то  
  delay(1000); //задержка
  //Serial.print(".");
  //Serial.println(WiFi.status());
  //Serial.println(WiFi.begin(ssid, keyIndex, key));
  }
  //Serial.println("WiFi connected..!");
  //Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  //Serial.println("HTTP server started");
  //hub.setupMQTT("test.mosquitto.org", 1883);
 // hub.setupMQTT("m6.wqtt.ru", 14426, "u_OD9CWG", "BjkdB1Py");  //подключиться к MQTT брокеру по адресу с логном и паролем
 // hub.setupMQTT("m4.wqtt.ru", 9022, "u_J2GRP4", "h7A64L6U");
  //hub.setupMQTT("m4.wqtt.ru", 9010, "u_158PW3", "woSIw3vz");
  hub.onBuild(build);     // подключаем билдер
  hub.begin();            // запускаем систему
  }

  xTaskCreatePinnedToCore(  //создаём задачу для ядра на FreeRTOS
                    Task2code,   // Функция задачи. 
                    "Task2",     // Имя задачи. 
                    10000,       // Размер стека 
                    NULL,        // Параметры задачи 
                    1,           // Приоритет 
                    &Task2,      // Дескриптор задачи для отслеживания 
                    1);          // Указываем ядро для этой задачи 

}

void notFound(AsyncWebServerRequest *request) { //функция на случай ненахода страницы
  request->send(404, "text/plain", "Not found");  //ответ на запрос в случае ненахода страницы
}

// Replaces placeholder with stored values
String processor(const String& var){  //функция для записи данных из памяти в переменную
  //Serial.println(var);
  if(var == "inputString1"){  //если в качестве аргумента указана первая строка 
    EEPROM.get(0, data1); // считываем из памяти, начиная с адреса 0 и записываем в переменную
    return data1; //выводим значение переменной
  }
  else if(var == "inputString2"){  //если в качестве аргумента указана вторая строка 
    EEPROM.get(255, data2); // считываем из памяти, начиная с адреса 255 и записываем в переменную
    return data2; //выводим значение переменной
  }
  return String();  //выводим значение одной из 2 переменных
}

void Shift16Leds()  //создаём функцию для управления сдвиговами регистрами
{
int reg[8] = {128,64,32,16,8,4,2,1};  //объявление и инициализация массива-маски
for (int i = 0; i<=7;i++) //счётчик колличества светодиодов первой микросхемы
{
LedOut1 = LedOut1 + (leds[i]*reg[i]); //перемножение состояния светодиода на маску
}
for (int i = 8; i<=15;i++) //счётчик колличества светодиодов второй микросхемы
{
LedOut2 = LedOut2 + (leds[i]*reg[i-8]); //перемножение состояния светодиода на маску
}
    digitalWrite(LATCH, LOW); //отключение защёлки на время записи в регистры
    shiftOut(SER, CLK, MSBFIRST, LedOut1);  //выводим состояние светодиодов для первого сдвигового регистра
    shiftOut(SER, CLK, MSBFIRST, LedOut2); //выводим состояние светодиодов для второго сдвигового регистра
    digitalWrite(LATCH, HIGH); //включение защёлки после записи в регистры и активация светодиодов
  //  delay(1000);
    LedOut1 = 0;  //обнуление переменной
    LedOut2 = 0;  //обнуление переменной
}
void MUXin()  //функция управления мультиплексором
{
  for (int i=0; i<=15; i++) //счётчик входов
  {
  s0 = i%2; //разложение номера датчика на степени двойки
  s1 = i/2%2; //разложение номера датчика на степени двойки
  s2 = i/4%2; //разложение номера датчика на степени двойки
  s3 = i/8%2; //разложение номера датчика на степени двойки
  digitalWrite(EN_2, HIGH); //активировать разрешающий вход
  digitalWrite(S0_2, s0); //вывод номера датчика на входы мультиплексора
  digitalWrite(S1_2, s1); //вывод номера датчика на входы мультиплексора
  digitalWrite(S2_2, s2); //вывод номера датчика на входы мультиплексора
  digitalWrite(S3_2, s3); //вывод номера датчика на входы мультиплексора
  digitalWrite(EN_2, LOW); //отключить разрешающий вход
  delay(5); //задержка
  Pins[i] = (1-digitalRead(SIG_2))*(i+1); //заполнение массива значениями со входа мультилексора
  leds[i] = 1-digitalRead(SIG_2); //заполнение массива значениями со входа мультилексора
  }
}

void handle_reset() { //функция сброса тревоги
  flag1 = 0;  //обнулить значение флага
  alarm1 = 0; //отмена тревоги датчика №1
  flag2 = 0;  //обнулить значение флага
  alarm2 = 0; //отмена тревоги датчика №2
  flag3 = 0;  //обнулить значение флага
  alarm3 = 0; //отмена тревоги датчика №3
  flag4 = 0;  //обнулить значение флага
  alarm4 = 0; //отмена тревоги датчика №4
  flag5 = 0;  //обнулить значение флага
  alarm5 = 0; //отмена тревоги датчика №5
  flag6 = 0;  //обнулить значение флага
  alarm6 = 0; //отмена тревоги датчика №6
  flag7 = 0;  //обнулить значение флага
  alarm7 = 0; //отмена тревоги датчика №7
  flag8 = 0;  //обнулить значение флага
  alarm8 = 0; //отмена тревоги датчика №8
  flag9 = 0;  //обнулить значение флага
  alarm9 = 0; //отмена тревоги датчика №9
  flag10 = 0;  //обнулить значение флага
  alarm10 = 0; //отмена тревоги датчика №10
  flag11 = 0;  //обнулить значение флага
  alarm11 = 0; //отмена тревоги датчика №11
  flag12 = 0;  //обнулить значение флага
  alarm12 = 0; //отмена тревоги датчика №12
  flag13 = 0;  //обнулить значение флага
  alarm13 = 0; //отмена тревоги датчика №13
  flag14 = 0;  //обнулить значение флага
  alarm14 = 0; //отмена тревоги датчика №14
  flag15 = 0;  //обнулить значение флага
  alarm15 = 0; //отмена тревоги датчика №15
  flag16 = 0;  //обнулить значение флага
  alarm16 = 0; //отмена тревоги датчика №16
}

void Task2code( void * pvParameters ){  //код для задачи FreeRTOS

  for(;;){  //бесконечный цикл
    Serial.println(batValue); //отправляем в последовательный порт значение уровня заряда аккумулятора
    Serial.println(spkLevel); //отправляем в последовательный порт значение уровня громкости сирены

if (alarm1 == 1||alarm2 == 1||alarm3 == 1||alarm4 == 1||alarm5 == 1||alarm6 == 1||alarm7 == 1||alarm8 == 1||alarm9 == 1||alarm10 == 1||alarm11 == 1||alarm12 == 1||alarm13 == 1||alarm14 == 1||alarm15 == 1||alarm16 == 1){ //если сработает любая из тревог
      AlarmFlag = 1;  //обединение всех сигналов тревоги в один (общая тревога)
}
if (alarm1 == 0&&alarm2 == 0&&alarm3 == 0&&alarm4 == 0&&alarm5 == 0&&alarm6 == 0&&alarm7 == 0&&alarm8 == 0&&alarm9 == 0&&alarm10 == 0&&alarm11 == 0&&alarm12 == 0&&alarm13 == 0&&alarm14 == 0&&alarm15 == 0&&alarm16 == 0){ //если нет ни одной активной тревоги
      AlarmFlag = 0;  //обединение всех сигналов тревоги в один (общая тревога = 0)
}
    if (mySwitch.available()) { //если пришёл пакет с радиоприёмника 433МГц
      if ((mySwitch.getReceivedValue()== 10437380) || (mySwitch.getReceivedValue()== 5592368)) //если пришедший сигнал равен коду брелка
      {
      AlarmFlag = 0; //отмена общей тревоги
      flag1 = 0;  //обнулить значение флага
      alarm1 = 0; //отмена тревоги датчика №1
      flag2 = 0;  //обнулить значение флага
      alarm2 = 0; //отмена тревоги датчика №2
      flag3 = 0;  //обнулить значение флага
      alarm3 = 0; //отмена тревоги датчика №3
      flag4 = 0;  //обнулить значение флага
      alarm4 = 0; //отмена тревоги датчика №4
      flag5 = 0;  //обнулить значение флага
      alarm5 = 0; //отмена тревоги датчика №5
      flag6 = 0;  //обнулить значение флага
      alarm6 = 0; //отмена тревоги датчика №6
      flag7 = 0;  //обнулить значение флага
      alarm7 = 0; //отмена тревоги датчика №7
      flag8 = 0;  //обнулить значение флага
      alarm8 = 0; //отмена тревоги датчика №8
      flag9 = 0;  //обнулить значение флага
      alarm9 = 0; //отмена тревоги датчика №9
      flag10 = 0;  //обнулить значение флага
      alarm10 = 0; //отмена тревоги датчика №10
      flag11 = 0;  //обнулить значение флага
      alarm11 = 0; //отмена тревоги датчика №11
      flag12 = 0;  //обнулить значение флага
      alarm12 = 0; //отмена тревоги датчика №12
      flag13 = 0;  //обнулить значение флага
      alarm13 = 0; //отмена тревоги датчика №13
      flag14 = 0;  //обнулить значение флага
      alarm14 = 0; //отмена тревоги датчика №14
      flag15 = 0;  //обнулить значение флага
      alarm15 = 0; //отмена тревоги датчика №15
      flag16 = 0;  //обнулить значение флага
      alarm16 = 0; //отмена тревоги датчика №16
      mySwitch.resetAvailable();  //возобновить возможность приёма пакетов радиомодулм 433МГц
      }
    }
    digitalWrite(SPK, AlarmFlag); //включить сирену в случае тревоги
    batValue100 = map(batValue, 1450, 2250, 1, 100);  //преобразование абсолютного значения с АЦП в проценты заряда
    if(batValue100 > 100) //ограничение максимального значения процентов
    {
      batValue100 = 100; //ограничение максимального значения процентов
    }
    if(batValue100 < 1) //ограничение минимального значения процентов
    {
      batValue100 = 1; //ограничение минимального значения процентов
    }
    spkLevel100 = map(spkLevel, 0, 4095, 5, 100);  //преобразование абсолютного значения с АЦП в проценты уровня громкости
    Serial.println(batValue100); //отправляем в последовательный порт значение уровня заряда аккумулятора %
    Serial.println(spkLevel100); //отправляем в последовательный порт значение уровня громкости сирены %
    Serial.println(batValue); //отправляем в последовательный порт значение уровня заряда аккумулятора
    Serial.println(spkLevel); //отправляем в последовательный порт значение уровня громкости сирены
    Serial.println(chrFlag); //отправляем в последовательный порт факт работы внешнего блока питания(1 или 0)
  }
}

void loop() { //начало бесконечного цикла
  hub.tick();  // обязательно тикаем тут
  static GHtimer tmr(1000); //таймер отправки обновлений интерфейса 
  // for (int count = 0; count < 10000; count++)
  MUXin();  //считыываем состояние датчиков мультиплексором
  Shift16Leds();  //отправляем состояние светодиодов на сдвиговые регистры
  if((1-leds[0])==0 && flag1 == 0)  //если светодиод горит
  {flag1 = 1;}  //поднять флаг
  if((1-leds[1])==0 && flag2 == 0)  //если светодиод горит
  {flag2 = 1;}  //поднять флаг
  if((1-leds[2])==0 && flag3 == 0)  //если светодиод горит
  {flag3 = 1;}  //поднять флаг
  if((1-leds[3])==0 && flag4 == 0)  //если светодиод горит
  {flag4 = 1;}  //поднять флаг
  if((1-leds[4])==0 && flag5 == 0)  //если светодиод горит
  {flag5 = 1;}  //поднять флаг
  if((1-leds[5])==0 && flag6 == 0)  //если светодиод горит
  {flag6 = 1;}  //поднять флаг
  if((1-leds[6])==0 && flag7 == 0)  //если светодиод горит
  {flag7 = 1;}  //поднять флаг
  if((1-leds[7])==0 && flag8 == 0)  //если светодиод горит
  {flag8 = 1;}  //поднять флаг
  if((1-leds[8])==0 && flag9 == 0)  //если светодиод горит
  {flag9 = 1;}  //поднять флаг
  if((1-leds[9])==0 && flag10 == 0)  //если светодиод горит
  {flag10 = 1;}  //поднять флаг
  if((1-leds[10])==0 && flag11 == 0)  //если светодиод горит
  {flag11 = 1;}  //поднять флаг
  if((1-leds[11])==0 && flag12 == 0)  //если светодиод горит
  {flag12 = 1;}  //поднять флаг
  if((1-leds[12])==0 && flag13 == 0)  //если светодиод горит
  {flag13 = 1;}  //поднять флаг
  if((1-leds[13])==0 && flag14 == 0)  //если светодиод горит
  {flag14 = 1;}  //поднять флаг
  if((1-leds[14])==0 && flag15 == 0)  //если светодиод горит
  {flag15 = 1;}  //поднять флаг
  if((1-leds[15])==0 && flag16 == 0)  //если светодиод горит
  {flag16 = 1;}  //поднять флаг
  if (tmr) {    //когда срабатывает таймер
  hub.sendUpdate("sw1", String(flag1)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw2", String(flag2)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw3", String(flag3)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw4", String(flag4)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw5", String(flag5)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw6", String(flag6)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw7", String(flag7)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw8", String(flag8)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw9", String(flag9)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw10", String(flag10)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw11", String(flag11)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw12", String(flag12)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw13", String(flag13)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw14", String(flag14)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw15", String(flag15)); //обновить состояние кнопки нтерфейса
  hub.sendUpdate("sw16", String(flag16)); //обновить состояние кнопки нтерфейса
  // Преобразуем значение переменной spkLevel100 в строку
  sprintf(str, "%d %%", spkLevel100); // %d используется для форматирования целочисленного значения, %% используется для вывода символа %
  hub.sendUpdate("Sound_lvl", str); //обновляем значение строки в интерфейсе
  // hub.sendUpdate("Sound_lvl", String(22));
  sprintf(str, "%d %%", batValue100); // %d используется для форматирования целочисленного значения, %% используется для вывода символа %
  hub.sendUpdate("Bat_lvl", str); //обновляем значение строки в интерфейсе
  //hub.sendUpdate("Bat_lvl", String(22));
  if (chrFlag == 0) //если внешний блок питания не подключен
  {
    sprintf(str, "%Нет"); //записать в строку Нет
  }
  else //иначе
  {
  sprintf(str, "%Да"); //записать в строку Да
  }
  }
  hub.sendUpdate("Charge_flag", str); //обновляем значение строки в интерфейсе

    batValue = analogRead(batPin);  // записать в переменную значение с АЦП (от 0 до 1023)
    spkLevel = analogRead(spkLvlPin);  // записать в переменную значение с АЦП (от 0 до 1023)
    chrFlag = digitalRead(chrPin);  // записать в переменную значение с цифрового входа(0 или 1)

  if(((1-leds[0]) && flag1) || alarm1 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led1", String(0)); //гасим светодиод в интерфейсе
  alarm1 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led1", String(1));}  //зажигаем светодиод

  if(((1-leds[1]) && flag2) || alarm2 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led2", String(0)); //гасим светодиод в интерфейсе
  alarm2 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led2", String(1));}  //зажигаем светодиод

  if(((1-leds[2]) && flag3) || alarm3 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led3", String(0)); //гасим светодиод в интерфейсе
  alarm3 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led3", String(1));}  //зажигаем светодиод

  if(((1-leds[3]) && flag4) || alarm4 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led4", String(0)); //гасим светодиод в интерфейсе
  alarm4 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led4", String(1));}  //зажигаем светодиод

  if(((1-leds[4]) && flag5) || alarm5 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led5", String(0)); //гасим светодиод в интерфейсе
  alarm5 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led5", String(1));}  //зажигаем светодиод

  if(((1-leds[5]) && flag6) || alarm6 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led6", String(0)); //гасим светодиод в интерфейсе
  alarm6 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led6", String(1));}  //зажигаем светодиод

  if(((1-leds[6]) && flag7) || alarm7 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led7", String(0)); //гасим светодиод в интерфейсе
  alarm7 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led7", String(1));}  //зажигаем светодиод

  if(((1-leds[7]) && flag8) || alarm8 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led8", String(0)); //гасим светодиод в интерфейсе
  alarm8 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led8", String(1));}  //зажигаем светодиод

  if(((1-leds[8]) && flag9) || alarm9 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led9", String(0)); //гасим светодиод в интерфейсе
  alarm9 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led9", String(1));}  //зажигаем светодиод

  if(((1-leds[9]) && flag10) || alarm10 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led10", String(0)); //гасим светодиод в интерфейсе
  alarm10 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led10", String(1));}  //зажигаем светодиод

  if(((1-leds[10]) && flag11) || alarm11 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led11", String(0)); //гасим светодиод в интерфейсе
  alarm11 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led11", String(1));}  //зажигаем светодиод

  if(((1-leds[11]) && flag12) || alarm12 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led12", String(0)); //гасим светодиод в интерфейсе
  alarm12 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led12", String(1));}  //зажигаем светодиод

  if(((1-leds[12]) && flag13) || alarm13 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led13", String(0)); //гасим светодиод в интерфейсе
  alarm13 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led13", String(1));}  //зажигаем светодиод

  if(((1-leds[13]) && flag14) || alarm14 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led14", String(0)); //гасим светодиод в интерфейсе
  alarm14 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led14", String(1));}  //зажигаем светодиод

  if(((1-leds[14]) && flag15) || alarm15 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led15", String(0)); //гасим светодиод в интерфейсе
  alarm15 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led15", String(1));}  //зажигаем светодиод

  if(((1-leds[15]) && flag16) || alarm16 == 1) //если срабатывает тревога с датчика
  {hub.sendUpdate("led16", String(0)); //гасим светодиод в интерфейсе
  alarm16 = 1;}  //поднять флаг тревоги
  else  // иначе
  {hub.sendUpdate("led16", String(1));}  //зажигаем светодиод
}
