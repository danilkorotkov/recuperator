#include "HomeSpan.h" 
//homespan-ota
#include "FanClass.h"
RECUP *recuperator;

#define SDA  23
#define SCL  22
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier

#define ROTCNT  35

#define CLK 27
#define DT  26
#define SW  25

/*#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // encoder + button */
//#define EB_FAST 50 
#include "EncButton.h"
EncButton<EB_TICK, DT, CLK, SW> enc1;  // энкодер с кнопкой <A, B, KEY>
TaskHandle_t h_encloop, h_HK_poll;

#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to
const int inoneWireBus = 17; 
const int outoneWireBus = 16;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire inoneWire(inoneWireBus);
OneWire outoneWire(outoneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature insensors(&inoneWire);
DallasTemperature outsensors(&outoneWire);



#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, SDA, SCL);

unsigned long CurrentTime  = 0;   // обновление эрана
unsigned long LCDTimeout   = 20/1000; 

unsigned long RotTimeout   = 5000;   // обновление чтения оборотов
unsigned long RotTime      = 0;   

unsigned long TempTimeout  = 1500;   // чтение термодатчков
unsigned long TempTime     = 0;   // 

static volatile float inc_dec=0; // чтение ожидание окончания вращения энкодера
static volatile unsigned long inc_dec_time=0;
unsigned long inc_dec_timeout=500;
static volatile uint32_t RotCnt=0;

#include "PCF8583.h"
// declare an instance of the library for IC at address 0xA0
// (A0 pin connected to ground)
PCF8583 counter(0xA0);
unsigned long CounterTime     = 0;   // 

#include "THP.h" 
DigooData gy21p;

#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bme; // I2C
Adafruit_Si7021 sensor = Adafruit_Si7021();

String level = "DISCONNECTED"; 

WordStruct LCDoutput;

void IRAM_ATTR isrENC() {
  enc1.tickISR();
  inc_dec_time = millis();
}

void IRAM_ATTR isrROT() {
  RotCnt++;
}

void poll_gy21p(){
  float bmeTemp   = bme.readTemperature();
  float bmePress  = bme.readPressure();//*0.00750062;
  float SiHum     = sensor.readHumidity();
  float SiTemp    = sensor.readTemperature();
  
  Serial.print("Temp bme:   ");Serial.print(bmeTemp);Serial.println("°C\t");
  Serial.print("Press bme:  ");Serial.print(bmePress*0.00750062);Serial.println("mmHg\t");
  Serial.print("Humidity:   ");Serial.print(SiHum, 2);Serial.println("%");
  Serial.print("Temp Si:    ");Serial.print(SiTemp, 2);Serial.println("°C\t");

  if (bmePress != NAN){
    gy21p.pressure = bmePress/100;//gPa for EVE
  }

  if (bmeTemp != NAN && SiTemp != NAN){
    gy21p.temperature = (bmeTemp + SiTemp)/2;
    gy21p.updated     = millis();
    gy21p.isNew[0]    = true;
  }

  if (bmeTemp == NAN && SiTemp != NAN){
    gy21p.temperature = bmeTemp;
    gy21p.updated     = millis();
    gy21p.isNew[0]    = true;
  }  
  
  if (bmeTemp != NAN && SiTemp == NAN){
    gy21p.temperature = SiTemp;
    gy21p.updated     = millis();
    gy21p.isNew[0]    = true;
  }

  if (SiHum != NAN){
    gy21p.humidity    = SiHum;
    gy21p.updated     = millis();
    gy21p.isNew[1]    = true;
  }
  
}

void encloop(void * pvParameters){

  attachInterrupt(CLK, isrENC, CHANGE);    
  attachInterrupt(DT,  isrENC, CHANGE);   
  attachInterrupt(SW,  isrENC, CHANGE);

  for (;;){
    if (enc1.tick()) {
    
      if (enc1.left()) {
        inc_dec--;
      }//left
      
      if (enc1.right()) {
       inc_dec++;
      }//right 

      if (enc1.click()){
         recuperator->OnOff();
      }//click

      if (enc1.held()) {
        recuperator->TargetFanState->setVal(!recuperator->TargetFanState->getVal());
      }//held
      enc1.resetState();
    } //tick  
  }   //loop
}     //task

void HK_poll(void * pvParameters){

  for (;;){
    homeSpan.poll();
  }   //loop
}     //task

void HomeKit(){
  homeSpan.setApSSID("Recup-AP");
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);

  homeSpan.setSketchVersion("2.1.1");
  homeSpan.enableOTA();
  
  homeSpan.begin(Category::Fans,"Recuperator");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Recuperator"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("final"); 
      new Characteristic::FirmwareRevision("2.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    recuperator = new RECUP();

  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Sensor"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("beta"); 
      new Characteristic::FirmwareRevision("0.1"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new THP(&gy21p);

}

void drawStatus() {
    display.clear();

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, LCDoutput.inTemp);
    display.drawString(0, 16, "вн");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 0, LCDoutput.outTemp);
    display.drawString(128, 16, "нар");
   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 16, LCDoutput.Speed);
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 45, LCDoutput.Status);

    
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    if (recuperator->TargetFanState->getVal() == tAUTO){
      display.drawString(128, 45, "@");
    } else{
      display.drawString(128, 45, "®");  
    }


    if (WiFi.status() == WL_CONNECTED) {
      long rssi = WiFi.RSSI();
      if (rssi >= -55) {
        level = "EXCELLENT";
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.fillRect(15,57,4,6);
        display.fillRect(20,55,4,8);
      }else if (rssi < -55 & rssi > -65) {
        level = "GOOD";
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.fillRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }else if (rssi < -65 & rssi > -75) {
        level = "LOW";
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.drawRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }else if (rssi < -75) {
        level = "BAD";
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.drawRect(10,59,4,4);
        display.drawRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }
    }else {
      level = "DISCONNECTED";
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 45, "x");
      display.drawRect(5,61,4,2);
      display.drawRect(10,59,4,4);
      display.drawRect(15,57,4,6);
      display.drawRect(20,55,4,8);
    }

    display.display();
 
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(SDA, SCL, uint32_t (400000));

  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  
  // Start the DS18B20 sensor
  insensors.begin();
  insensors.requestTemperatures();
  outsensors.begin();
  outsensors.requestTemperatures();
  TempTime = millis();

  HomeKit();

  xTaskCreatePinnedToCore(
                    encloop,     /* Task function. */
                    "encloop",   /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &h_encloop, /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */                  

  xTaskCreatePinnedToCore(
                    HK_poll,    /* Task function. */
                    "HK_poll",  /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &h_HK_poll, /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */  
  
  delay(1000);
  
  bme.begin(BMP280_ADDRESS_ALT);
  sensor.begin();
  poll_gy21p();
  
  // configure PCF8586 to event counter mode and reset counts
  pinMode(ROTCNT,INPUT);
  attachInterrupt(ROTCNT, isrROT, CHANGE);
  counter.setMode(MODE_EVENT_COUNTER);
  counter.setCount(0);
  CurrentTime = millis();
  Serial.println("Setup complete");  
}

void loop() {
  
  //homeSpan.poll();
  
  if ( (millis() - CurrentTime) > LCDTimeout ) {

    if ( (millis() - TempTime) > TempTimeout ) {
      float temperatureC = insensors.getTempCByIndex(0);
      insensors.requestTemperatures();
      if(temperatureC != DEVICE_DISCONNECTED_C) {
        LCDoutput.inTemp = String(temperatureC, 1) + "ºC";
        recuperator->IntTemp->setVal(temperatureC);
      }
  
      temperatureC = outsensors.getTempCByIndex(0);
      outsensors.requestTemperatures(); 
      if(temperatureC != DEVICE_DISCONNECTED_C) {        
        LCDoutput.outTemp = String(temperatureC, 1) + "ºC";
        recuperator->OutTemp->setVal(temperatureC);
      }   
  
      TempTime = millis();
    }
    
    if ( (millis() - RotTime) > RotTimeout ) {
      uint32_t rotSpeed = 30000*counter.getCount()/(millis() - RotTime);
      counter.setCount(0);

      uint32_t cntSpeed = 15000*RotCnt/(millis() - RotTime);
      recuperator->RotCnt->setVal(cntSpeed);
      
      RotCnt=0;
      RotTime = millis();
      //LCDoutput.Speed = String(rotSpeed);
      Serial.println("---------------------------");
      Serial.print("Aver speed: ");Serial.println(rotSpeed);
      Serial.print("CNT  speed: ");Serial.println(cntSpeed);
      Serial.print("Targ speed: ");Serial.println(recuperator->RotationSpeed->getVal());   
      Serial.print("IN  temp:   ");Serial.println(LCDoutput.inTemp);
      Serial.print("OUT temp:   ");Serial.println(LCDoutput.outTemp);
      Serial.print("Uptime, s:  ");Serial.println(millis()/1000);
      Serial.print("State:      ");Serial.println(LCDoutput.Status);
      Serial.print("Mode:       ");Serial.println(recuperator->TargetFanState->getVal() == tAUTO? "Auto":"Manual");
            
      if (WiFi.status() != WL_CONNECTED) { 
        Serial.println("Couldn't get a wifi connection");
      }else {
        Serial.print("WIFI RSSI:  ");Serial.print(WiFi.RSSI());Serial.println("dB");
        recuperator->WiFiLevel->setString(level.c_str());//(float(WiFi.RSSI())); 
      }
      poll_gy21p();
      Serial.println("---------------------------");
      
    }
    
    CurrentTime = millis();

    if (inc_dec != 0 & (millis() - inc_dec_time) > inc_dec_timeout){
      float tempSpeed = recuperator->RotationSpeed->getVal() + inc_dec * 100;
      inc_dec = 0;
      
      if (tempSpeed > 5300) {tempSpeed=5300;}
      else if (tempSpeed < 1000)  {tempSpeed=1000;}

      if (tempSpeed != recuperator->RotationSpeed->getVal()){
        
        Serial.println("Save and set new speed");
        recuperator->RotationSpeed->setVal(tempSpeed);
        recuperator->setSpeed();
      }
    }


    drawStatus();
  }
  //encloop();
}
