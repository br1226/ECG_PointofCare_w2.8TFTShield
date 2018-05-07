/*Point of Care ECG Mat Arduino Code

This code collects an ECG signal using a Bitalino sensor for an analog input. When the button is pressed the signal is collected and displayed on the 
2.8" TFT LCD with touchscreen. A potentiometer is used to set a threshold value to calculate the heartrate of the patient.  When the button is pressed
a second time the collection pauses and will pick right back up if the button is pressed again. A green LED lights up to indicate signal collection.


The circuit:
 The LED input is connected to the Arduino UNO digital 3 pin and is a green LED grounded via a 220ohm resistor
 The button input is a switch connected to the Arduino Uno digital 2 is a common switch given 3.3V and grounded via a 220ohm resistor 
 The potentiometer is connected to the Arduino Uno analog pin 5 and is adjusting the voltage between ground and 3.3V
 The ECG input is a BITalino ECG sensor connected to the 3.3V, ground, and Arduino Uno analog pin 4

 The screen output Adafruit 2.8" TFT LCD shield with touch screen is connected directly to the Arduino on the corresponding pins for the following pins
  Reset
  3.3V
  5V
  Ground (3x)
  Vin
  Digital Pins 13-8


Date Created: May 6, 2018
By Lily Gaudreau and Brianne Roper
With contributions from Dr. Emily Farrar 

Tutorial: 
 */



//ECG Variables 
const int ECG = A4;
int sensorValue = 0;

//Other Variables 
const int buttonPin = 2;
const int LEDgreen = 3;

//Heart Rate Variables 
long prevBeatTime = 0; // time of the last recorded heart beat
long beatTime = 0;//time of the most recently recorded beat
float beatInterval = 0;//time between the current beat and previous beat
int beatCount = 0;//keeps track of how many beats have occured
float beatIntervalSum = 0;//sum of the intervals between beats
float beatIntervalAverage =0;//average of the intervals between beats
const int interval = 3;//after 3 beats are recorded the heart rate is calculated
int HeartRate = 0;//the calculated patient heart rate


int reading; //current reading from the input
int state = LOW; //the current state of the output
int previous = LOW; //the previous reading from the input
long timeStamp = 0; //time of data collection

//potentiometer variables
const int potPin = A5;//the analog pin input of the potentiometer
int potVal = 0;//sets the initial potentiometer value
int threshold = 0;//threshold used to calculate heartrate based on potentiometer

//2.8" Adafruit TFT sheild screen values
#include "SPI.h" //Serial peripheral interphase library
#include "Adafruit_ILI9341.h"//screen library
#include "Adafruit_GFX.h"//Graphics library

const int TFT_DC =9; //default pins
const int TFT_CS=10; //default pins
int screenx=240; //screen has 240 pixels in the x direction
int screeny = 320; //screen has 320 pixels in the y direction
int countScreen=40; //integer used to start the signals printed on the screen
int previouspoint=20; //keeps track of the previous ECG signal value 
int previousThreshold=20; //keeps track of the previous threshold signal value

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {

 Serial.begin(115200);//begin the serial montior

 //Set pin modes for the LED and button
 pinMode(LEDgreen, OUTPUT);
 pinMode(buttonPin, INPUT);

 //Begin the screen and set the background to black
 tft.begin();
 tft.fillScreen(ILI9341_BLACK);

}

void loop() {
//We begin by setting the LED off and reading the signal from the button and the potentimeter for the threshold
digitalWrite(LEDgreen, LOW);
reading= digitalRead(buttonPin);
potVal = analogRead(potPin);
threshold = potVal;

//Next we set the default screen with ECG title and gridlines
//The following displays the ECG title above the grid
  tft.setRotation(1);
  tft.setCursor(screeny/2,0);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.print("ECG");
  tft.setRotation(0);
  
 //the following lines draw the vertical gridlines
  tft.drawLine(0, 40, screenx-20, 40, ILI9341_RED);
  tft.drawLine(0, 96, screenx-20, 96, ILI9341_RED);
  tft.drawLine(0, 152, screenx-20, 152, ILI9341_RED);
  tft.drawLine(0, 208, screenx-20, 208, ILI9341_RED);
  tft.drawLine(0, 264, screenx-20, 264, ILI9341_RED);
  tft.drawLine(0, 319, screenx-20, 319, ILI9341_RED);

//the following lines draw the horizontal gridlines
  tft.drawLine(1, 40, 1, screeny, ILI9341_RED);
  tft.drawLine(56, 40, 56, screeny, ILI9341_RED);
  tft.drawLine(112, 40, 112, screeny, ILI9341_RED);
  tft.drawLine(168, 40, 168, screeny, ILI9341_RED);
  tft.drawLine(220, 40, 220, screeny, ILI9341_RED);
  //end of screen

//We keep track of whether or not the button was pushed use that information to determine if the ECG signal should be collected, every other time the button is pressed data is collected
if (reading == HIGH && previous == LOW){ 

  if(state == HIGH){
    state = LOW;
    digitalWrite(LEDgreen, LOW);
  }

    else{
      state = HIGH;
    }
}

/*The following will occur when the button is pressed in order to collect a reading
The LED turns on, the ECG is collected and displayed on the TFT, 
 the threshold is displayed on the TFT, Heartrate is calculated based on the threshold and displayed on the TFT,
 the screen is reset once the display is filled with signal,the time, heartrate, signal, and threshold are also printed on the serial monitor to be saved 
 when plugged into the computer. 
*/
if (state == HIGH){
digitalWrite(LEDgreen, HIGH);
countScreen= countScreen+1;
sensorValue = analogRead(ECG);

//This block is specifically for calculating heartrate
//First, we determine if the sensor value crosses the threshold and measure the beat and the time between this beat and the last
  if (sensorValue> threshold) 
  {
    beatTime = millis();
    delay(5);
    beatInterval = beatTime - prevBeatTime;
    beatIntervalSum = beatIntervalSum + beatInterval;
    prevBeatTime= beatTime;
    beatCount = beatCount +1;
  }
  
 //Next, after 3 beats have happened, the average interval between beats is used to calculate the heart rate   
    if(beatCount == interval){

      beatIntervalAverage= beatIntervalSum/beatCount;
      HeartRate = 60000/beatIntervalAverage;
      HeartRate = constrain( HeartRate, 40, 150);
      beatCount = 0;
      beatIntervalSum = 0;
    }
    

 
//We time stamp each data point
  timeStamp = millis();
  float seconds = timeStamp*0.001;
  
//All the data is sent to the serial monitor so when the arduino is plugged into the computer data can be stored.
 Serial.print(seconds);
 Serial.print(", ");
 Serial.print(sensorValue);
 Serial.print(", ");
 Serial.print(threshold);
 Serial.print(", ");
 Serial.println(HeartRate);
//The next block has to do with printing values to the TFT screen 


//When the signal reaches the edge of the TFT screen the cursor is reset and the screen clears 
  if(countScreen > 320){
  countScreen = 40;//start screen at beginning
  previouspoint=20;
  tft.fillScreen(ILI9341_BLACK);}
  
//We translate the ECG and Threshold Values to heights on the screen
  int height  = map (sensorValue, 0, 1023, 0, 300); 
  int thresholdheight=map(threshold, 0, 1023, 0, 300);

//We draw lines for both signal between the previous point and the current point
  tft.drawLine(previouspoint, countScreen-1, height, countScreen, ILI9341_WHITE);
  tft.drawLine(previousThreshold, countScreen-1, thresholdheight, countScreen, ILI9341_YELLOW);

  previouspoint=height; //set previous signal value
  previousThreshold=thresholdheight;//set previous threshold value
  
 //The following block displays the heart rate on the side of the screen
 tft.setCursor(0,0);
 tft.setTextColor(ILI9341_WHITE); tft.setTextSize(1);
 tft.print("Heart Rate: ");
 tft.println(HeartRate);

  delay(5);//delays to reset everything
}


else{
  
}

previous = reading;//Finally, we make sure the button pressing status is updated. 
}

  
  
  
    

