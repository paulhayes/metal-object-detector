#include <Arduino.h>
#include <FreqCount.h>
#include <Debouncer.h>
#include <EEPROM.h>

const int ledGndPin = 9;
const int ledPin = 10;
const int btnPin = 12;

uint32_t minValue;
uint32_t maxValue;
bool btnPressed = false;
uint32_t currentValue;

Debouncer buttonDebounce(100);

enum States {
  READING,
  RECORD_START,
  RECORDING,
  RECORD_END
};

States state;

void setup(){
  FreqCount.begin(100);
  Serial.begin(9600);
  pinMode(btnPin,INPUT_PULLUP);
  pinMode(ledPin,OUTPUT);
  pinMode(ledGndPin,OUTPUT);
  digitalWrite(ledGndPin,LOW);
  EEPROM.begin();
  EEPROM.put<uint32_t>(0,minValue);
  EEPROM.put<uint32_t>(sizeof(minValue),maxValue);
}

void loop(){
  if(FreqCount.available()){
    currentValue = FreqCount.read();
    Serial.print( currentValue );
    Serial.print(" ");
    Serial.print( minValue );
    Serial.print(" ");
    Serial.print( maxValue );
    Serial.print(" ");
    Serial.println("");
  }

  state = btnPressed ? RECORDING : READING;
  int btnVal = digitalRead(btnPin);
  if(buttonDebounce.update(btnVal)){  
    btnPressed = buttonDebounce.get() == LOW;
    state = btnPressed ? RECORD_START : RECORD_END;
  } 

  analogWrite(ledPin,0);
  switch(state){
    case READING:
    analogWrite(ledPin,((minValue<=currentValue) && (currentValue<=maxValue))?64:0);
    break;
    case RECORD_START:
    minValue = 1UL<<31;
    maxValue = 0UL;
    break;
    case RECORDING:
    minValue = min(minValue,currentValue);
    maxValue = max(maxValue,currentValue); 
    break;
    case RECORD_END:
    EEPROM.put(0,minValue);
    EEPROM.put(sizeof(minValue),maxValue);
    state = READING;
    break;
  }
  
}