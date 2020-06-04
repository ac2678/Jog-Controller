
// Jog Controller for garage CNC
// AJC
// For Arduino Nano

#include <SendOnlySoftwareSerial.h>
#include <AnalogButtons.h>
#include <Rotary.h>

#define Z_AXIS_BUT A3      // Z axis voltage divider buttons
#define XY_AXIS_BUT A2     // X/Y axis voltage divider buttons
#define SET_AXIS_BUT A1    // X/Y/Z Set Zero voltage divider buttons
#define MISC_BUT A0        // Misc Gcode voltage divider buttons

#define EN_A 3             // Encoder A input pin
#define EN_B 4             // Encoder B input pin

#define LED_1 5            // LED 1 Pin (Smallest Value)
#define LED_2 6            // LED 2 Pin
#define LED_3 7            // LED 3 Pin
#define LED_4 8            // LED 4 Pin
#define LED_5 9            // LED 5 Pin (Largest Value)
#define LED_6 10           // LED 6 Pin
#define LED_7 11           // LED 7 Pin
#define LED_8 12           // LED 8 Pin

SendOnlySoftwareSerial MarlinSerial(1);

int debug = false;

unsigned int ButtonDelay = 5;                     // Button delay in milliseconds. Adjust to suit or 0 for off.
unsigned long MoveTypeDelay = 3000;               //How long to wait until issuing command to switch back to absolute G90.
unsigned long MoveTypeLastTime = MoveTypeDelay;

double jogVal = 0.050;

#define XY_But_Total 6
#define Z_But_Total 4
#define Set_But_Total 4
#define Misc_But_Total 4

int setpt3[Z_But_Total] = {255,511,766};
int setptxy[XY_But_Total] = {173,342,511,681,850};

int Abs_Posn = false;
int Paused = false;

//-----------------------------------------------------------------------------
//------------------------Button Function Prototypes---------------------------
//-----------------------------------------------------------------------------

void xPosClick();
void xNegClick();
void xyHomeClick();
void yPosClick();
void yNegClick();
void zPosClick();
void zHomeClick();
void zNegClick();
void xZeroClick();
void yZeroClick();
void zZeroClick();
void EndstopHomeClick();
void PauseResumeClick();
void sbOriginClick();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


#define ANALOGBUTTONS_SAMPLING_INTERVAL 25

AnalogButtons xyButtons(XY_AXIS_BUT, INPUT, ButtonDelay,50);
AnalogButtons zButtons(Z_AXIS_BUT, INPUT, ButtonDelay,50);
AnalogButtons setButtons(SET_AXIS_BUT, INPUT, ButtonDelay,50);
AnalogButtons miscButtons(MISC_BUT, INPUT, ButtonDelay,50);

Button zPos = Button(setpt3[0], &zPosClick);
Button zHome = Button(setpt3[1], &zHomeClick);
Button zNeg = Button(setpt3[2], &zNegClick);

Button xPos = Button(setptxy[1], &xPosClick, 0, 50,3000);
Button xNeg = Button(setptxy[3], &xNegClick, 0, 50,3000);
Button xyHome = Button(setptxy[2], &xyHomeClick, 0, 50,3000);
Button yPos = Button(setptxy[4], &yPosClick, 0, 50,3000);
Button yNeg = Button(setptxy[0], &yNegClick, 0, 50,3000);

Button xZero = Button(setpt3[0], &xZeroClick);
Button yZero = Button(setpt3[1], &yZeroClick);
Button zZero = Button(setpt3[2], &zZeroClick);

Button EndstopHome = Button(setpt3[0], &EndstopHomeClick);
Button PauseResume = Button(setpt3[1], &PauseResumeClick);
Button sbOrigin = Button(setpt3[2], &sbOriginClick);

int counter = 0;

 Rotary rotary = Rotary(EN_A,EN_B);


void setup() {

  xyButtons.add(xPos);
  xyButtons.add(xNeg);
  xyButtons.add(xyHome);
  xyButtons.add(yPos);
  xyButtons.add(yNeg);

  zButtons.add(zPos);
  zButtons.add(zHome);
  zButtons.add(zNeg);

  setButtons.add(xZero);
  setButtons.add(yZero);
  setButtons.add(zZero);

  miscButtons.add(EndstopHome);
  miscButtons.add(PauseResume);
  miscButtons.add(sbOrigin);


  MarlinSerial.begin(250000);
  Serial.begin(9600);
  Serial.println("initialize");
  pinMode(Z_AXIS_BUT, INPUT);
  pinMode(XY_AXIS_BUT, INPUT);
  pinMode(SET_AXIS_BUT, INPUT);
  pinMode(MISC_BUT, INPUT);
  pinMode(EN_A, INPUT_PULLUP);
  pinMode(EN_B, INPUT_PULLUP);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(LED_6, OUTPUT);
  pinMode(LED_7, OUTPUT);
  pinMode(LED_8, OUTPUT);


}

void loop() {

  // EncoderInput();
  unsigned char result = rotary.process();
    if (result == DIR_CW) {
      counter++;
      if (counter > 8){counter = 8;}
      Serial.println(counter);
    } else if (result == DIR_CCW) {
      counter--;
      if (counter < 1){counter = 1;}
      Serial.println(counter);
    }

  // SetJog();

//  xyButtons.check();
//  zButtons.check();
//  setButtons.check();
//  miscButtons.check();
//  SetMoveType();

}

void SetMoveType(){
// Check if any commands have been sent recently.  If so, sent a command for relative positioning moves (i.e. G91).
//   If not (i.e. enough time has passed in the delay interval), then set Marlin back to absolute positioning (i.e. G90).
  if ((millis() - MoveTypeLastTime) < MoveTypeDelay){
    if (Abs_Posn){
      // Set Relative Positioning
      Abs_Posn = false;
      String msg = "G91";
      MarlinSerial.println(msg);
      if (debug){Serial.println(msg);}
    }
  }
  else if (!Abs_Posn){
    // Switch back to Absolute Positioning
    Abs_Posn = true;
    String msg = "G90";
    MarlinSerial.println(msg);
    if (debug){Serial.println(msg);}
  }
}

void EncoderInput() {

  unsigned char result = rotary.process();
    if (result == DIR_CW) {
      counter++;
      if (counter > 8){counter = 8;}
      Serial.println(counter);
    } else if (result == DIR_CCW) {
      counter--;
      if (counter < 1){counter = 1;}
      Serial.println(counter);
    }

  // // Read the current state of inputCLK
  // currentStateA = digitalRead(EN_A);
  //
  // // If the previous and the current state of the inputCLK are different then a pulse has occured
  // if ((previousStateA == LOW) && (currentStateA == HIGH)){
  //   // If the inputDT state is different than the inputCLK state then
  //   // the encoder is rotating counterclockwise
  //   if (digitalRead(EN_B) == LOW) {
  //     counter --;
  //   } else {
  //     counter ++;
  //   }
  //   if (counter < 1) {counter=1;}
  //   if (counter > 8) {counter=8;}
  //   if (debug){Serial.println(counter);}
  // }
  // // If the previous and the current state of the inputCLK are different then a pulse has occured
  // if ((previousStateA == HIGH) && (currentStateA == LOW)){
  //   // If the inputDT state is different than the inputCLK state then
  //   // the encoder is rotating counterclockwise
  //   if (digitalRead(EN_B) == HIGH) {
  //     counter --;
  //   } else {
  //     counter ++;
  //   }
  //   if (counter < 1) {counter=1;}
  //   if (counter > 8) {counter=8;}
  //   if (debug){Serial.println(counter);}
  // }
  // previousStateA = currentStateA;
}

void SelectLED(int cnt) {
 // Turn on LED based on encoder position

int LED_arr[] = {LED_1,
                 LED_2,
                 LED_3,
                 LED_4,
                 LED_5,
                 LED_6,
                 LED_7,
                 LED_8};

  for (int i = 1; i <= sizeof(LED_arr); i++){
    // If LED is on but not the current selection, turn it off
    if ((LED_arr[i] == HIGH) && (LED_arr[i] != cnt)){
      LED_arr[i] = LOW;
    }
    // If LED is off and IS current selection, turn it on
    if ((LED_arr[i] == LOW) && (LED_arr[i] == cnt)){
      LED_arr[i] = HIGH;
    }
  }
}

void SetJog(){
  switch(counter){
    case 1:
      jogVal = .001;
    case 2:
      jogVal = .010;
    case 3:
      jogVal = .020;
    case 4:
      jogVal = .050;
    case 5:
      jogVal = .100;
    case 6:
      jogVal = .500;
    case 7:
      jogVal = 1.0;
    case 8:
      jogVal = 3.0;
  }
}


//-----------------------------------------------------------------------------
//----------------------------Check X and Y Buttons----------------------------
//-----------------------------------------------------------------------------

void xPosClick(){
   // X Postive Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 X" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void xNegClick(){
   // X Negative Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 X-" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void xyHomeClick(){
   // Go to XY Home
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 X0.0 Y0.0";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void yPosClick(){
   // Y Positive Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 Y" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void yNegClick(){
   // Y Negative Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 Y-" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}

//-----------------------------------------------------------------------------
//-----------------------------Check Z Buttons---------------------------------
//-----------------------------------------------------------------------------

void zPosClick(){
   // Z Postive Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 Z" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void zHomeClick(){
   // Home Z - Go to Z0
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 Z0.0";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void zNegClick(){
   // Z Negative Move
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G0 Z-" + String(jogVal);
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}

//-----------------------------------------------------------------------------
//---------------------------Check Set Zero Buttons----------------------------
//-----------------------------------------------------------------------------

void xZeroClick(){
   // Set X-Axis Zero
   String msg = "G92 X0.0";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void yZeroClick(){
   // Set Y-Axis Zero
   String msg = "G92 Y0.0";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void zZeroClick(){
   // Set Z-Axis Zero
   String msg = "G92 Z0.0";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}

//-----------------------------------------------------------------------------
//---------------------------Check Misc Buttons--------------------------------
//-----------------------------------------------------------------------------

void EndstopHomeClick(){
   // Home XY
   MoveTypeLastTime = millis();
   SetMoveType();
   String msg = "G28 X Y";
   MarlinSerial.println(msg);
   if (debug){Serial.println(msg);}
}
void PauseResumeClick(){
   // Pause/Resume
   if (!Paused){
       String msg = "M0";
       MarlinSerial.println(msg);
       if (debug){Serial.println(msg);}
       Paused = true;
   }
   else {
       String msg = "M108";
       MarlinSerial.println(msg);
       if (debug){Serial.println(msg);}
       Paused = false;
   }
}
void sbOriginClick(){
   // Go To Spoilboard Origin
   String msg1 = "G90";
   String msg2 = "G0 X2.15625 Y2.15625";
   MarlinSerial.println(msg1);
   MarlinSerial.println(msg2);
   if (debug){Serial.println(msg1);}
   if (debug){Serial.println(msg2);}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
