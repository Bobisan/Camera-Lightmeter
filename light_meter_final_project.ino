#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold9pt7b.h>
int RE1a = 10, RE1b = 21, RE2a = 11, RE2b = 36;
int counter1 = 0, counter2 = 0, counter3 = 0, current_position = 0;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void setup() {
  Serial.begin(9600);
  pinMode(10, INPUT);
  pinMode(21, INPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  delay(500);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,0);
  //display.setFont(&FreeSansBold9pt7b);
  display.printf("Counter 1:%d", counter1);
  //display.setFont(NULL);
  display.printf("Counter 2:%d", counter2);
  display.printf("Counter 3:%d", counter3);
  display.display();
}
// for up RE1a is first
void loop() {
  while(digitalRead(RE2a) == LOW && digitalRead(RE2b) == HIGH){
    current_position ++;
    if(current_position == 3){
      current_position = 0;
    }
  }
  while(digitalRead(RE2a) == HIGH && digitalRead(RE2b) == LOW){
    current_position --;
    if(current_position == -1){
      current_position = 2;
    }
  }
  while(digitalRead(RE1a) == LOW && digitalRead(RE1b) == HIGH){
    switch(current_position){
      case 0: 
      counter1 ++; 
      display.setCursor(0,0);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 1:%d", counter1);
      //display.setFont(NULL);
      display.printf("Counter 2:%d", counter2);
      display.printf("Counter 3:%d", counter3);
      display.display();
      break;
      case 1: 
      counter2 ++; 
      display.setCursor(0,0);
      display.printf("Counter 1:%d", counter1);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 2:%d", counter2);
      //display.setFont(NULL);
      display.printf("Counter 3:%d", counter3);
      display.display();
      break;
      case 2: 
      counter3 ++;
      display.setCursor(0,0);
      display.printf("Counter 1:%d", counter1);
      display.printf("Counter 2:%d", counter2);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 3:%d", counter3);
      //display.setFont(NULL);
      display.display(); 
      break;
    }
    if(digitalRead(RE1a) == HIGH){
      break;
    }
  }
  while(digitalRead(RE1a) == HIGH && digitalRead(RE1b) == LOW){
    switch(current_position){
      case 0: 
      counter1 ++; 
      display.setCursor(0,0);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 1:%d", counter1);
      //display.setFont(NULL);
      display.printf("Counter 2:%d", counter2);
      display.printf("Counter 3:%d", counter3);
      display.display();
      break;
      case 1: 
      counter2 ++; 
      display.setCursor(0,0);
      display.printf("Counter 1:%d", counter1);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 2:%d", counter2);
      //display.setFont(NULL);
      display.printf("Counter 3:%d", counter3);
      display.display();
      break;
      case 2: 
      counter3 ++;
      display.setCursor(0,0);
      display.printf("Counter 1:%d", counter1);
      display.printf("Counter 2:%d", counter2);
      //display.setFont(&FreeSansBold9pt7b);
      display.printf("Counter 3:%d", counter3);
      //display.setFont(NULL);
      display.display(); 
      break;
    }
    if(digitalRead(RE1b) == HIGH){
      break;
    }
  }
  
  
}
