#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"

// defining the set up variables for the led ring
#define LEDS_COUNT  8  // The number of led
#define LEDS_PIN	  2  // define the pin connected to the Freenove 8 led strip
#define CHANNEL		  0  // RMT module channel

//defining the set up variables for the ir remote
const uint16_t recvPin = 14; // Infrared receiving pin
IRrecv irrecv(recvPin); // Create a class object used to receive class
decode_results results; // Create a decoding results class object

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB); // initalize the string with the number of led, the information pin, channerl number, and color type

int m_color[4][3] = { {255, 146, 0}, {60, 14, 247}, {246, 14, 21}, {14, 248, 21}}; // list of all posible colors
int num_rows = sizeof(m_color) / sizeof(m_color[0]);
int LEDstatus = 1;// var for the led on.pff
int BrightnessLevel = 10; //var for brightness
int stableColorValue = 0; // if the leds are stable what color from array to show
int lastInputValue; // last input from ir remote

void setup() {//set up
  strip.begin();
  Serial.begin(115200); // Initialize the serial port and set the baud rate to 115200
  irrecv.enableIRIn(); // Start the receiver
  while (! Serial) // Wait for the serial connection to be established.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(recvPin); //print the infrared receiving pin
  strip.setBrightness(55);
  //strip.setAllLedsColorData(m_color[1][0],m_color[1][1],m_color[1][2]);
}


void brightness(int j){ // cahnges the brightness of the strip based on a given value
  strip.setBrightness(j);
  strip.show();
}
void rotate();
void stableColor(int color);
void flash();
void circle();

void loop() {
  if (irrecv.decode(&results)) { // Waiting for decoding
    delay(100);// this delay is important because it makes sure theres a delay between inputs, 100 seems good but maybe increase it to more than 250, just incase some one has quick fingers
    if (results.value == 0xFFA25D && LEDstatus == 1){// this wukk trun of led 
      strip.setBrightness(0);
      LEDstatus = 0;
      Serial.print("led is off");
      serialPrintUint64(LEDstatus);
      Serial.println("");
    }
    else if (results.value == 0xFFA25D && LEDstatus == 0){ //this will turn on led
      strip.setBrightness(BrightnessLevel);
      LEDstatus = 1;
      Serial.print("led is on");
      serialPrintUint64(LEDstatus);
      Serial.println("");
    }
    else if (results.value == 0xFF02FD ){ // will increase the brightness by 20 points
      BrightnessLevel = BrightnessLevel+20;
      Serial.print("brightness is at");
      brightness(BrightnessLevel);
      serialPrintUint64(BrightnessLevel);
      Serial.println("");
    }
    else if (results.value == 0xFF9867 && BrightnessLevel > 0){  // will decrease the brightness by 20 points
      BrightnessLevel = BrightnessLevel - 20;
      brightness(BrightnessLevel);
      Serial.print("brightness is at");
      serialPrintUint64(BrightnessLevel);
      Serial.println("");
    }
    else if (results.value == 0xFF9867 && BrightnessLevel == 0){  // this makes sure that the brightness doesnt go below the 0
      Serial.println("brightness at lowest setting.");
        Serial.println("");
    }
    else if (results.value == 0xffa857){ // this will change the color value for stable color
      if (stableColorValue < num_rows){
        stableColorValue++;
        Serial.print("stableColorValue= ");
        Serial.println(stableColorValue);
        Serial.println("");
      }
      else if ((stableColorValue == num_rows)){ // this will make sure that the stable colors will go back to the first item when it reaches the end of the array
        stableColorValue=0;
        Serial.print("stableColorValue= ");
        Serial.println(stableColorValue);
        Serial.println("");
      }
    }
    else if(results.value == 0xff906f){// this will set to max brightness (255)
      brightness(255);
      Serial.print("brightness is at max");
      Serial.println("");
    }
    else if(results.value == 0xffe01f){// this will set to min brightness (10)
      brightness(10);
      Serial.print("brightness is at min");
      Serial.println("");
    }
    else {
      lastInputValue = 0; // this will insure that inputs will not stack on top of each other by deleting the previouse input
      lastInputValue = results.value;
      Serial.println("");
      Serial.printf("lastInputValue = 0x%x\n", lastInputValue);
      Serial.println("");
    }
    
    irrecv.resume(); // Release the IRremote. Receive the next value
    
  }//if (irrecv.decode(&results))

  if (lastInputValue == 0){// if the user hasn't provided the lighting case, the default case will play, in this case its rotate
    rotate();
  }
  else {
    switch (lastInputValue) {
      // this case is for input validation, this will reset the leds incase the user was to hold down any button for too long
      case 0xffffffff:
        lastInputValue = 0;
        break;
      case 0xff30cf:
        rotate();
        break;
      case 0xff6897:
        stableColor(stableColorValue);
        break;
      case 0xff18e7:
        flash();
        break;
      case 0xff7a85:
        circle();
        break;
    }
  }
     
}


void rotate(){ // will create a rotating effect on the led showing different colors
  for (int x = 0; x < 4 ; x++) {
    for ( int j = 0; j < 8; j++) {
      strip.setLedColorData(j, m_color[x][0],m_color[x][1],m_color[x][2]);
      strip.show(); 
      delay(50);
    }
  }
}

void stableColor(int color){// this will set all the leds to a color that is passed through the input
  strip.setAllLedsColorData(m_color[color][0],m_color[color][1],m_color[color][2]);
  strip.show();
}
void flash(){ // flash leds in sequnce of color array
  for ( int j = 0; j < 4; j++) {
      strip.setLedColorData(0, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(1, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(2, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(3, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(4, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(5, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(6, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.setLedColorData(7, m_color[j][0],m_color[j][1],m_color[j][2]);
      strip.show(); 
      delay(130);
    }
}
void circle(){ // this function should create two leds that will rotate in the circle
  int x = 0;
  int j = 4;
  for(int y = 0; y < 4; y++){
    strip.setLedColorData(x, m_color[stableColorValue][0],m_color[stableColorValue][1],m_color[stableColorValue][2]);
    strip.setLedColorData(j, m_color[stableColorValue][0],m_color[stableColorValue][1],m_color[stableColorValue][2]);
    strip.show();
    delay(110);
    strip.setLedColorData(x, 0,0,0);
    strip.setLedColorData(j, 0,0,0);
    strip.show();
    delay(110);
    x++;
    j++;
  }
}

