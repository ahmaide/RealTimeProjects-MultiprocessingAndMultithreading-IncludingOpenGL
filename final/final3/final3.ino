#include <LiquidCrystal.h>

#include <Arduino_FreeRTOS.h>

const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;


string password = "1234";
string entered;

void checkButton(void *para);

void setup() {
  pinMode(2, INPUT);
  xTaskCreate(checkButton,     
             "Checker",       
             200,              
             NULL,             
             1,                
             NULL);  

 vTaskStartScheduler();

}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0, 1);
  // entering code here
}

void checkButton(void *para){
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  lcd.begin(16, 2);
  lcd.print("Enter the code");
  while(1){
    if(digitalRead(2)==HIGH){
      lcd.clear();
      lcd.setCursor(0, 0);
      if(password==entered){
        lcd.print("Safe locked");
      }
      else{
        lcd.print("wrong password");
      }
    }
  }
  
}
