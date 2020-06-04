// Jog Controller
// AJC
// For Arduino Nano

#include <SendOnlySoftwareSerial.h>
#include <AnalogMultiButton.h>

#define Z_AXIS_BUT A3      // Z axis voltage divider buttons
#define XY_AXIS_BUT A2     // X/Y axis voltage divider buttons
#define SET_AXIS_BUT A1    // X/Y/Z Set Zero voltage divider buttons
#define MISC_BUT A0        // Misc Gcode voltage divider buttons

#define EN_A 2            // Encoder A input pin
#define EN_B 3            // Encoder B input pin
#define EN_SWITCH 4       // Encoder Press Button Pin

#define LED_1 5           // LED 1 Pin (Smallest Value)
#define LED_2 6           // LED 2 Pin
#define LED_3 7           // LED 3 Pin
#define LED_4 8           // LED 4 Pin
#define LED_5 9           // LED 5 Pin (Largest Value)
#define LED_MODE 10       // LED Mode Pin (Fine/Coarse)

SendOnlySoftwareSerial MarlinSerial(1);

int debug = true;

unsigned long CurrentMillis;                      // Timer elements for delayed button press
unsigned long ButtonMillis;
unsigned int ButtonDelay=300;                             // Button delay in milliseconds. Adjust to suit or 0 for off.
unsigned long MoveTypeDelay = 3000;  //How long to wait until issuing command to switch back to absolute G90.
unsigned long MoveTypeLastTime = MoveTypeDelay;

int CurrZState[3];
int LastZState[3];
int LastZtime[3];
int ButtonPressedZ[3];
int dbToggleZ[3];
int debounceZ = false;

double jogVal = 0.050;

int read_Z;
int read_XY;
int read_SET;
int read_MISC;

#define XY_But_Total 6
#define Z_But_Total 4
#define Set_But_Total 4
#define Misc_But_Total 4

int setpt3[Z_But_Total] = {0,250,500,800};
int setptxy[XY_But_Total] = {0,167,338,510,680,850};

int Abs_Posn = false;
int Paused = false;

AnalogMultiButton xyButton = AnalogMultiButton(XY_AXIS_BUT,XY_But_Total,setptxy,80);
AnalogMultiButton zButton = AnalogMultiButton(Z_AXIS_BUT,Z_But_Total,setpt3,80);
AnalogMultiButton setButton = AnalogMultiButton(SET_AXIS_BUT,Set_But_Total,setpt3,80);
AnalogMultiButton miscButton = AnalogMultiButton(MISC_BUT,Misc_But_Total,setpt3,80);

int setTol = 35;

int counter = 0;
int currentStateA;
int previousStateA;

bool FINE_JOG = true;

void setup() {

  MarlinSerial.begin(250000);
  Serial.begin(9600);
  Serial.println("initialize");
  pinMode(Z_AXIS_BUT, INPUT);
  pinMode(XY_AXIS_BUT, INPUT);
  pinMode(SET_AXIS_BUT, INPUT);
  pinMode(MISC_BUT, INPUT);
  pinMode(EN_A, INPUT);
  pinMode(EN_B, INPUT);
  pinMode(EN_SWITCH, INPUT);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(LED_MODE, OUTPUT);

//   for (int i=0; i < 3; i++) {
//     LastZState[i]=LOW;
//     LastZtime[i]=0;
//     ButtonPressedZ[i]=false;
//     dbToggleZ[i]=0;
//   }
 }


void loop() {
  CurrentMillis = millis();

  // read_Z = analogRead(Z_AXIS_BUT);
  // read_XY = analogRead(XY_AXIS_BUT);
  // read_SET = analogRead(SET_AXIS_BUT);
  // read_MISC = analogRead(MISC_BUT);

  //Serial.println(read_Z);
  // ReadZ(read_Z);

  //EncoderInput();

  //SetJog();
  xyButton.update();
  zButton.update();
  setButton.update();
  miscButton.update();
  SetMoveType();

  //-----------------------------------------------------------------------------
  //----------------------------Check X and Y Buttons----------------------------
  //-----------------------------------------------------------------------------

  if (xyButton.onPress(1)){
     // X Postive Move
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 X" + String(jogVal);
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (xyButton.onPress(2)){
     // X Negative Move
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 X-" + String(jogVal);
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (xyButton.onPress(3)){
     // Go to XY Home
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 X0.0 Y0.0";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (xyButton.onPress(4)){
     // Y Positive Move
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 Y" + String(jogVal);
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (xyButton.onPress(5)){
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

  if (zButton.onPress(1)){
     // Z Postive Move
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 Z" + String(jogVal);
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (zButton.onPress(2)){
     // Home Z - Go to Z0
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G0 Z0.0";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (zButton.onPress(3)){
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

  if (setButton.onPress(1)){
     // Set X-Axis Zero
     String msg = "G92 X0.0";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (setButton.onPress(2)){
     // Set Y-Axis Zero
     String msg = "G92 Y0.0";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (setButton.onPress(3)){
     // Set Z-Axis Zero
     String msg = "G92 Z0.0";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }

  //-----------------------------------------------------------------------------
  //---------------------------Check Misc Buttons--------------------------------
  //-----------------------------------------------------------------------------

  if (miscButton.onPress(1)){
     // Home XY
     MoveTypeLastTime = millis();
     SetMoveType();
     String msg = "G28 X Y";
     MarlinSerial.println(msg);
     if (debug){Serial.println(msg);}
  }
  else if (miscButton.onPress(2)){
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
  else if (miscButton.onPress(3)){
     // Go To Spoilboard Origin
     String msg1 = "G90";
     String msg2 = "G0 X2.15625 Y2.15625";
     MarlinSerial.println(msg1);
     MarlinSerial.println(msg2);
     if (debug){Serial.println(msg1);}
     if (debug){Serial.println(msg2);}
  }
}

// void ReadZ(int inVal) {
// // Check analog value and execute Z axis buttons

//
//   if (analogRead(Z_AXIS_BUT) < 1025){
//     for (int i=0; i < 3; i++) {
//       CurrZState[i] = CheckState(setpt[i],"Z");
//       if (CurrZState[i] != LastZState[i]){
//         LastZtime[i] = millis();
//         debounceZ = true;
//       }
//         else if (debounceZ){
//           if ((millis()-LastZtime[i])>ButtonDelay){
//             dbToggleZ[i] = !dbToggleZ[i];
//             if (dbToggleZ[i]==true){
//               ButtonPressedZ[i] = true;
//             }
//             else{
//               ButtonPressedZ[i] = false;
//             }
//             if (ButtonPressedZ[1]){
//               // Button 1 (Z Positive Move)
//               MarlinSerial.println("G0 Z" + String(jogVal));
//               Serial.println("G0 Z" + String(jogVal));
//             }
//             else if (ButtonPressedZ[2]){
//               MarlinSerial.println("G0 Z-" + String(jogVal));
//               Serial.println("G0 Z-" + String(jogVal));
//             }
//             else if (ButtonPressedZ[3]){
//               MarlinSerial.println("G28 Z");
//               Serial.println("G28 Z");
//             }
//             debounceZ = false;
//           }
//         }
//         LastZState[i] = CurrZState[i];
//       }
//     }
// }

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
  // Read the current state of inputCLK
  currentStateA = digitalRead(EN_A);

  // If the previous and the current state of the inputCLK are different then a pulse has occured
  if (currentStateA != previousStateA){

    // If the inputDT state is different than the inputCLK state then
    // the encoder is rotating counterclockwise
    if (digitalRead(EN_B) != currentStateA) {
      counter --;
      if (counter < 1) {counter=1;}
      }
    else {
      // Encoder is rotating clockwise
      counter ++;
      if (counter > 5) {counter=5;}
    }
    // If state changes, update LED status
    SelectLED(counter);
  }
}

void SelectLED(int cnt) {
  // Turn on LED based on encoder position
  // First clear any that are on
  digitalWrite(LED_1,LOW);
  digitalWrite(LED_2,LOW);
  digitalWrite(LED_3,LOW);
  digitalWrite(LED_4,LOW);
  digitalWrite(LED_5,LOW);

  if (cnt == 1){
    digitalWrite(LED_1, HIGH);
  }
  else if (cnt == 2){
    digitalWrite(LED_2, HIGH);
  }
  else if (cnt == 3){
    digitalWrite(LED_3,HIGH);
  }
  else if (cnt == 4){
    digitalWrite(LED_4,HIGH);
  }
  else{
    digitalWrite(LED_5,HIGH);
  }
}

void SetJog(){
  if (!FINE_JOG){
    switch(counter){
      case 1:
        jogVal = .100;
      case 2:
        jogVal = .250;
      case 3:
        jogVal = .500;
      case 4:
        jogVal = 1.0;
      case 5:
        jogVal = 2.0;
    }
  }
  else{
    switch(counter){
      case 1:
        jogVal = .001;
      case 2:
        jogVal = .005;
      case 3:
        jogVal = .010;
      case 4:
        jogVal = .020;
      case 5:
        jogVal = .050;
    }
  }
}

// int CheckState(int setpt, char button)
//   {
//     // Serial.println("In check");
//     if (button = "Z"){
//       int inVal = analogRead(Z_AXIS_BUT);
//       //Serial.println(inVal,DEC);
//       bool check1 = (inVal > (setpt - setTol)) && (inVal < (setpt + setTol));
//       return(check1);
//     }
//   }
//
// int chk_db()
//   {
//     return(millis()-CurrentMillis)>ButtonDelay;
  // }
