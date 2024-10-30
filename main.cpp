#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>

// Define PWM channels for the single RGB LED
const int red_channel_1[]   = {0, 3, 6,  9, 12};   // Red channel
const int green_channel_1[] = {1, 4, 7, 10, 13}; // Green channel
const int blue_channel_1[]  = {2, 5, 8, 11, 14};  // Blue channel

const int red_channel_2[]   = {0, 3, 6,  9, 12};    // Red channel
const int green_channel_2[] = {1, 4, 7, 10, 13}; // Green channel
const int blue_channel_2[]  = {2, 5, 8, 11, 14};  // Blue channel

//int potPin = A0;  // Potentiometer for color selection             (for testing with hardware)
//int brightPot = A1;   //potentiometer for brightness adjustment    (for testing with hardware)

float currentRed = 0.0;
float currentGreen = 0.0;
float currentBlue = 0.0;
int targetRed = 0;
int targetGreen = 0;
int targetBlue = 0;

int colour = 0; //stores the colour case no. read by the arduino from the bluetooth data stream
float bright = 255.0; //stores the brightness value read by the arduino from the bluetooth data stream
int fx = 0;

float brightness = 0.0;
float fadeAmount = 0.005;
int fadeDirection = 1;

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);

SoftwareSerial BTserial(10,11); //RX TX


// FUNCTIONS
void fadeToColour(float bright);
void chaser(float bright);
void breathe(float &brightness, float &fadeAmnt);
void strobe(float bright);
void rainbow(float bright);
void hueToRGB(uint16_t hue, float &r, float &g, float &b);

void setup() {
  Serial.begin(9600);
 
  // Initialize the PCA9685 module
  pwm1.begin();
  pwm1.setPWMFreq(1000); // Set a reasonable PWM frequency

  pwm2.begin();
  pwm2.setPWMFreq(1000);

  BTserial.begin(9600);



  // Turn off all channels (set all color channels to high for common anode)
  for (int i = 0; i < 16; i++) {
    pwm1.setPWM(i, 4096, 0);
    // 4096 turns the output fully off (high for common anode)
  }
    for (int i = 0; i < 16; i++) {
    pwm2.setPWM(i, 4096, 0);
    // 4096 turns the output fully off (high for common anode)
  }
}

void loop() {
  // Read the potentiometer value (0-1023)                           
 // int potValue = analogRead(potPin);                               (for hardware testing)
 // delay(10);                                                       
                              
 //int brightRead = analogRead(brightPot);                           (for hardware testing)
 //float bright = map(brightRead,0,1023,0,255) / 255.0;              (for hardware testing)
 
// Map the potentiometer value to a range of 0-12 for smooth color transition
//int colourIndex = map(potValue, 0, 1023, 0, 22);                   (for hardware testing)

// Set target colors based on the potentiometer value
if(BTserial.available()){
   String input = BTserial.readStringUntil('\n');

   //check if data is not empty
   if(input.length() > 0){
      int colonIndex1 = input.indexOf(':');                   //split data based on ':'
      int colonIndex2 = input.indexOf(':',colonIndex1 + 1);

        if(colonIndex1 != -1 && colonIndex2 != -1 ){
            String colourValue = input.substring(0, colonIndex1);
            String brightnessValue = input.substring(colonIndex1 + 1, colonIndex2);
            String fxValue = input.substring(colonIndex2 + 1);

            //converting string values to integers(brightness to float)
            colour = colourValue.toInt();
            bright = brightnessValue.toFloat() / 255.0;
            fx = fxValue.toInt();
                 
                  Serial.print("colour: ");
                  Serial.println(colour);
                  Serial.print("bright: ");
                  Serial.println(bright);
                  Serial.print("fx: ");
                  Serial.println(fx);
        }
    }
}

  
  switch (colour) {
    case 0:
      targetRed = 255;
      targetGreen = 0;
      targetBlue = 0;  // Red
      break;
    
    case 1:
      targetRed = 255;
      targetGreen = 64;
      targetBlue = 0; // Reddish-Orange
      break;

    case 2:
      targetRed = 255;
      targetGreen = 128;
      targetBlue = 0; // Orange
      break;

    case 3:
      targetRed = 255;
      targetGreen = 255;
      targetBlue = 0; // Yellow
      break;

    case 4:
      targetRed = 128;
      targetGreen = 255;
      targetBlue = 0; // Yellow-Green
      break;

    case 5:
      targetRed = 0;
      targetGreen = 255;
      targetBlue = 0; // Green
      break;    

    case 6:
      targetRed = 0;
      targetGreen = 255;
      targetBlue = 128; // Light Green-Cyan
      break;

    case 7:
      targetRed = 0;
      targetGreen = 255;
      targetBlue = 255; // Cyan
      break;

    case 8:
      targetRed = 0;
      targetGreen = 128;
      targetBlue = 255; // Light Blue
      break;

    case 9:
      targetRed = 0;
      targetGreen = 0;
      targetBlue = 255; // Blue
      break;

    case 10:
      targetRed = 128;
      targetGreen = 0;
      targetBlue = 255; // Purple
      break;
    
    case 11:
      targetRed = 255;
      targetGreen = 0;
      targetBlue = 255; // Magenta
      break;

    case 12:
      targetRed = 255;
      targetGreen = 255;
      targetBlue = 255; // White
      break;

    default:
      targetRed = 0;
      targetGreen = 0;
      targetBlue = 0; // Off for safety
      break;
  }

  // Fade to the target color smoothly
  //fadeToColour(bright);
 switch(fx){
   case 0: 
   fadeToColour(bright);
   break;

   case 1:
   chaser(bright);
   break;

   case 2:
   breathe(brightness,fadeAmount);
   break;

   case 3: 
   strobe(bright);
   delay(125);

   case 4:
   rainbow(bright);
   break;
 }
}

// FUNCTIONS
void fadeToColour(float bright) {
  
  // Smoothly adjust current color towards the target color
  currentRed += (targetRed - currentRed) / 10.0;
  currentGreen += (targetGreen - currentGreen) / 10.0;
  currentBlue += (targetBlue - currentBlue) / 10.0;

   //Map the color values to the PWM range for common anode logic
  int redValue = map((int)currentRed * bright, 0, 255, 4095, 0);      //(for hardware test)
  int greenValue = map((int)currentGreen * bright, 0, 255, 4095, 0);  //    (for hardware test)
  int blueValue = map((int)currentBlue * bright, 0, 255, 4095, 0);    //  (for hardware test)

//  int redValue = map((int)currentRed, 0, 255, 4095, 0);      
//  int greenValue = map((int)currentGreen, 0, 255, 4095, 0);      //only with colours
//  int blueValue = map((int)currentBlue, 0, 255, 4095, 0); 

  // Apply the PWM values to the LED channels
  for (int i = 0; i < 5; i++) {
    pwm1.setPWM(red_channel_1[i], 0, redValue);
    pwm1.setPWM(green_channel_1[i], 0, greenValue);
    pwm1.setPWM(blue_channel_1[i], 0, blueValue);
  }

  for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 0, redValue);
    pwm2.setPWM(green_channel_2[i], 0, greenValue);
    pwm2.setPWM(blue_channel_2[i], 0, blueValue);
  }


}

void chaser(float bright){
    // Smoothly adjust current color towards the target color
  currentRed += (targetRed - currentRed) / 10.0;
  currentGreen += (targetGreen - currentGreen) / 10.0;
  currentBlue += (targetBlue - currentBlue) / 10.0;

   //Map the color values to the PWM range for common anode logic
  int redValue = map((int)currentRed * bright, 0, 255, 4095, 0);      //(for hardware test)
  int greenValue = map((int)currentGreen * bright, 0, 255, 4095, 0);  //    (for hardware test)
  int blueValue = map((int)currentBlue * bright, 0, 255, 4095, 0);  

    // Apply the PWM values to the LED channels
  for (int i = 0; i < 5; i++) {
    pwm1.setPWM(red_channel_1[i], 0, redValue);
    pwm1.setPWM(green_channel_1[i], 0, greenValue);
    pwm1.setPWM(blue_channel_1[i], 0, blueValue);
    delay(100);
        pwm1.setPWM(red_channel_1[i], 4096, 0);
    pwm1.setPWM(green_channel_1[i], 4096, 0);
    pwm1.setPWM(blue_channel_1[i], 4096, 0);
    
  }

    for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 0, redValue);
    pwm2.setPWM(green_channel_2[i], 0, greenValue);
    pwm2.setPWM(blue_channel_2[i], 0, blueValue);
    delay(100);
    pwm2.setPWM(red_channel_2[i], 4096, 0);
    pwm2.setPWM(green_channel_2[i], 4096, 0);
    pwm2.setPWM(blue_channel_2[i], 4096, 0);
    
  }

}

void breathe(float &brightness,float &fadeAmnt){
  currentRed += (targetRed - currentRed) / 10.0;
  currentGreen += (targetGreen - currentGreen) / 10.0;
  currentBlue += (targetBlue - currentBlue) / 10.0;

  brightness += fadeAmnt;

  if (brightness <= 0) {
    brightness = 0;
    fadeAmnt = -fadeAmnt;
  } 
  else if (brightness >= 1) {
    brightness = 1;
    fadeAmnt = -fadeAmnt;
  }


   //Map the color values to the PWM range for common anode logic
  int redValue = map((int)currentRed * brightness , 0, 255, 4095, 0);      //(for hardware test)
  int greenValue = map((int)currentGreen * brightness , 0, 255, 4095, 0);  //    (for hardware test)
  int blueValue = map((int)currentBlue * brightness, 0, 255, 4095, 0);  

  for (int i = 0; i < 5; i++) {
    pwm1.setPWM(red_channel_1[i], 0, redValue);
    pwm1.setPWM(green_channel_1[i], 0, greenValue);
    pwm1.setPWM(blue_channel_1[i], 0, blueValue);
  }

  for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 0, redValue);
    pwm2.setPWM(green_channel_2[i], 0, greenValue);
    pwm2.setPWM(blue_channel_2[i], 0, blueValue);
  }

}

void strobe(float bright){
    // Smoothly adjust current color towards the target color
  currentRed += (targetRed - currentRed) / 10.0;
  currentGreen += (targetGreen - currentGreen) / 10.0;
  currentBlue += (targetBlue - currentBlue) / 10.0;

   //Map the color values to the PWM range for common anode logic
  int redValue = map((int)currentRed * bright, 0, 255, 4095, 0);      //(for hardware test)
  int greenValue = map((int)currentGreen * bright, 0, 255, 4095, 0);  //    (for hardware test)
  int blueValue = map((int)currentBlue * bright, 0, 255, 4095, 0);  

    // Apply the PWM values to the LED channels
  for (int i = 0; i < 5; i++) {
    pwm1.setPWM(red_channel_1[i], 0, redValue);
    pwm1.setPWM(green_channel_1[i], 0, greenValue);
    pwm1.setPWM(blue_channel_1[i], 0, blueValue);
  }

  for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 0, redValue);
    pwm2.setPWM(green_channel_2[i], 0, greenValue);
    pwm2.setPWM(blue_channel_2[i], 0, blueValue);
  }
  
  delay(500);

  for (int i = 0; i < 5; i++){
    pwm1.setPWM(red_channel_1[i], 4096, 0);
    pwm1.setPWM(green_channel_1[i], 4096, 0);
    pwm1.setPWM(blue_channel_1[i], 4096, 0);
  }

  for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 4096, 0);
    pwm2.setPWM(green_channel_2[i], 4096, 0);
    pwm2.setPWM(blue_channel_2[i], 4096, 0);
    
  }

}

void rainbow(float bright) {
  static uint16_t hue = 0; // Tracks the current hue (0-65535 for smoother transitions)

  // Increment hue to create a continuous transition (wrap around at 65535)
  hue = (hue + 50) % 65536;

  // Convert hue to RGB values
  float r = 0.0, g = 0.0, b = 0.0;
  hueToRGB(hue, r, g, b);

  // Map the RGB values to PWM range for common anode logic
  int redValue = map((int)(r * bright * 255), 0, 255, 4095, 0);
  int greenValue = map((int)(g * bright * 255), 0, 255, 4095, 0);
  int blueValue = map((int)(b * bright * 255), 0, 255, 4095, 0);

  // Apply the PWM values to the LED channels
  for (int i = 0; i < 5; i++) {
    pwm1.setPWM(red_channel_1[i], 0, redValue);
    pwm1.setPWM(green_channel_1[i], 0, greenValue);
    pwm1.setPWM(blue_channel_1[i], 0, blueValue);
  }

  for (int i = 0; i < 5; i++) {
    pwm2.setPWM(red_channel_2[i], 0, redValue);
    pwm2.setPWM(green_channel_2[i], 0, greenValue);
    pwm2.setPWM(blue_channel_2[i], 0, blueValue);
  }

  delay(20); // Adjust for speed of color transition; higher values slow down the transition.
}

void hueToRGB(uint16_t hue, float &r, float &g, float &b) {
  // Convert a hue value (0-65535) to RGB components in the range [0, 1]
  float h = hue / 65536.0 * 360.0; // Scale hue to degrees (0-360)
  float s = 1.0; // Saturation (1.0 for full saturation)
  float v = 1.0; // Value (1.0 for full brightness)

  int i = int(h / 60.0) % 6;
  float f = h / 60.0 - i;
  float p = v * (1.0 - s);
  float q = v * (1.0 - f * s);
  float t = v * (1.0 - (1.0 - f) * s);

  switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}


