#include "HomeSpan.h" 
#include "FanClass.h"


#define CLK 25
#define DT  26
#define SW  27

#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // encoder + button

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 21, 22);

unsigned long CurrentTime     = 0;   // опрос
unsigned long LCDTimeout   = 500;   // опрос
WordStruct LCDoutput;

void IRAM_ATTR isrENC() {
  enc1.tick();  // encoder int
}

void encloop(){
  enc1.tick();
  
  if (enc1.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
    // ваш код
  }
  
  if (enc1.isRight()) Serial.println("Right");         // если был поворот
  if (enc1.isLeft()) Serial.println("Left");
  
  if (enc1.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (enc1.isLeftH()) Serial.println("Left holded");
  
  //if (enc1.isPress()) Serial.println("Press");         // нажатие на кнопку (+ дебаунс)
  //if (enc1.isRelease()) Serial.println("Release");     // то же самое, что isClick
  
  if (enc1.isClick()) Serial.println("Click");         // одиночный клик
  if (enc1.isSingle()) Serial.println("Single");       // одиночный клик (с таймаутом для двойного)
  if (enc1.isDouble()) Serial.println("Double");       // двойной клик
  
  
  if (enc1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
  //if (enc1.isHold()) Serial.println("Hold");         // возвращает состояние кнопки
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
      new Characteristic::Model("alpha"); 
      new Characteristic::FirmwareRevision("0.1"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new RECUP();
}

void drawStatus() {

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, LCDoutput.inTemp);

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(128, 0, LCDoutput.outTemp);
   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 16, LCDoutput.Speed);
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 45, LCDoutput.Status);
 
}

void setup() {
  Serial.begin(115200);

  Serial.println("LCD test");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  Serial.println("Enc test");
  attachInterrupt(CLK, isrENC, CHANGE);    
  attachInterrupt(DT,  isrENC, CHANGE);   
  attachInterrupt(SW,  isrENC, CHANGE);
  enc1.setType(TYPE2);

  CurrentTime = millis();
  HomeKit();
  Serial.println("Setup complete");  
}

void loop() {
  homeSpan.poll();
  if ( (millis() - CurrentTime) > LCDTimeout ) {
    CurrentTime = millis();
    display.clear();
    drawStatus();
    display.display();
    //Serial.println("LCD updated");
  }
  encloop();
}
