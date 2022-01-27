#include "HomeSpan.h" 

#include "FanClass.h"
RECUP *recuperator;

#include "PCF8583.h"
// declare an instance of the library for IC at address 0xA0
// (A0 pin connected to ground)
#define sda  23
#define scl  22
PCF8583 counter(0xA0, sda, scl);
unsigned long CounterTime     = 0;   // 

#define CLK 25
#define DT  26
#define SW  27

/*#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // encoder + button */
#define EB_FAST 50 
#include "EncButton.h"
EncButton<EB_TICK, DT, CLK, 27> enc1;  // энкодер с кнопкой <A, B, KEY>
TaskHandle_t h_encloop;

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
SSD1306  display(0x3c, sda, scl);

unsigned long CurrentTime  = 0;   // обновление эрана
unsigned long LCDTimeout   = 20/1000; 

unsigned long RotTimeout   = 5000;   // обновление чтения оборотов
unsigned long RotTime      = 0;   

unsigned long TempTimeout  = 1500;   // чтение термодатчков
unsigned long TempTime     = 0;   // 

static volatile float inc_dec=0; // чтение ожидание окончания вращения энкодера
static volatile unsigned long inc_dec_time=0;
unsigned long inc_dec_timeout=500;

//bool isISRset=0;

WordStruct LCDoutput;

void IRAM_ATTR isrENC() {
  enc1.tickISR();
  inc_dec_time = millis();
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
         on_off_enc(false);
         recuperator->OnOff();
         on_off_enc(true);
      }//click

      if (enc1.held()) {
        on_off_enc(false);
        recuperator->TargetFanState->setVal(!recuperator->TargetFanState->getVal());
        on_off_enc(true);
      }//held
      enc1.resetState();
    } //tick  
  }   //loop
}     //task

void on_off_enc(bool on){
  if (on){
    gpio_intr_enable(gpio_num_t(CLK));
    gpio_intr_enable(gpio_num_t(DT));
    gpio_intr_enable(gpio_num_t(SW));
  }else{
    gpio_intr_disable(gpio_num_t(CLK));
    gpio_intr_disable(gpio_num_t(DT));
    gpio_intr_disable(gpio_num_t(SW));
  }
}

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
      new Characteristic::Model("beta"); 
      new Characteristic::FirmwareRevision("1.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    recuperator = new RECUP();
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
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.fillRect(15,57,4,6);
        display.fillRect(20,55,4,8);
      }else if (rssi < -55 & rssi > -65) {
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.fillRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }else if (rssi < -65 & rssi > -75) {
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.fillRect(10,59,4,4);
        display.drawRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }else if (rssi < -75) {
        display.drawRect(0,62,4,1);
        display.drawRect(5,61,4,2);
        display.drawRect(10,59,4,4);
        display.drawRect(15,57,4,6);
        display.drawRect(20,55,4,8);
      }
    }else {
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

  // Start the DS18B20 sensor
  insensors.begin();
  insensors.requestTemperatures();
  outsensors.begin();
  outsensors.requestTemperatures();
  TempTime = millis();

  HomeKit();

  Serial.println("LCD test");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  Serial.println("Enc test");

  xTaskCreatePinnedToCore(
                    encloop,     /* Task function. */
                    "encloop",   /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &h_encloop, /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  
  
  // configure PCF8586 to event counter mode and reset counts
  counter.setMode(MODE_EVENT_COUNTER);
  counter.setCount(0);
  CurrentTime = millis();
  Serial.println("Setup complete");  
}

void loop() {
  
  homeSpan.poll();
  
  if ( (millis() - CurrentTime) > LCDTimeout ) {

    if ( (millis() - TempTime) > TempTimeout ) {
      float temperatureC = insensors.getTempCByIndex(0);
      insensors.requestTemperatures();
      if(temperatureC != DEVICE_DISCONNECTED_C) {
        LCDoutput.inTemp = String(temperatureC, 1) + "ºC";
      }
  
      temperatureC = outsensors.getTempCByIndex(0);
      outsensors.requestTemperatures(); 
      if(temperatureC != DEVICE_DISCONNECTED_C) {
        LCDoutput.outTemp = String(temperatureC, 1) + "ºC";
      }   
  
      TempTime = millis();
      Serial.print("IN  temp:  ");Serial.println(LCDoutput.inTemp);
      Serial.print("OUT temp:  ");Serial.println(LCDoutput.outTemp);
    }
    
    if ( (millis() - RotTime) > RotTimeout ) {
      uint32_t rotSpeed = 30000*counter.getCount()/(millis() - RotTime);
      counter.setCount(0);
      //LCDoutput.Speed = String(rotSpeed);
      Serial.print("Aver speed ");Serial.println(rotSpeed);
      RotTime = millis();
      
      if (WiFi.status() != WL_CONNECTED) { 
        Serial.println("Couldn't get a wifi connection");
      }else {
        Serial.print("WIFI RSSI: ");Serial.println(WiFi.RSSI());
      }
    }
    
    CurrentTime = millis();

    if (inc_dec != 0 & (millis() - inc_dec_time) > inc_dec_timeout){
      float tempSpeed = recuperator->RotationSpeed->getVal() + inc_dec * 100;
      inc_dec = 0;
      
      if (tempSpeed > 5300) {tempSpeed=5300;}
      else if (tempSpeed < 1000)  {tempSpeed=1000;}

      if (tempSpeed != recuperator->RotationSpeed->getVal()){
        
        on_off_enc(false);
        
        recuperator->RotationSpeed->setVal(tempSpeed);
        recuperator->setSpeed();

        on_off_enc(true);
      }
    }


    drawStatus();
  }
  //encloop();
}
