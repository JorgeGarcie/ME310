#include <AccelStepper.h>


// ##### THIS HAS TO DDO WITH THE STEPPER ####


#define DIR_PIN 5
#define STEP_PIN 2
#define ENABLE_PIN 8

#define DIR_PIN_CUT 6
#define STEP_PIN_CUT 3

#define EN_PIN_CUT 13

#define LOADCELL_DOUT_PIN 28
#define LOADCELL_SCK_PIN 29

#define LIFT_PIN 12



AccelStepper extruder(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
AccelStepper cutter(AccelStepper::DRIVER, STEP_PIN_CUT, DIR_PIN_CUT);

// ###########################################

// #### THIS HAS TO DO WITH LOAD CELL ####
#include "HX711.h"
HX711 scale;
unsigned long timer = 0;
unsigned long lastScaleCheck = 0;
const unsigned long scaleInterval = 100;

float calibration_factor = -3050.545898;  //31450.00; //orignally this was negative
long currentload;





// ###########################################

#include "StateMachine.h"
enum State MEGA = EXTRUDE;


String command;


#include "Communication.h"

void setup() {
  Serial.begin(115200);

  // lin actuator
  pinMode(LIFT_PIN, OUTPUT);
  analogWrite(LIFT_PIN, 0);

  // Enable driver
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);  // Disable motor
  pinMode(DIR_PIN, OUTPUT);

  pinMode(EN_PIN_CUT, OUTPUT);
  digitalWrite(EN_PIN_CUT, HIGH);  // LOW = enabled
  pinMode(DIR_PIN_CUT, OUTPUT);



  // Start init comunication
  initArd(Serial, "Bob", "Hi Bob");

  // Set speed and acceleration
  extruder.setAcceleration(1000);
  extruder.setMaxSpeed(2000);

  cutter.setAcceleration(40000);
  cutter.setMaxSpeed(100000);


  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  currentload = scale.get_units();
  MEGA = WAIT;

  extruder.setSpeed(800);
}

void loop() {

  switch (MEGA) {


    case EXTRUDE:


      // implement extrude for sample collection (this involves to back extrude x amount of steps)
      digitalWrite(ENABLE_PIN, LOW);
      if (millis() - lastScaleCheck > scaleInterval) {
        if (abs(scale.get_units() - currentload) >= 1) {
          extruder.move(-550);
          while (extruder.distanceToGo() != 0) {
            extruder.run();
          }
          Serial.println("EXTRUDE COMPLETED");
          MEGA = WAIT;
          command = "";
        }
        lastScaleCheck = millis();
      }
      if (millis() - timer > 10000) {
        Serial.println("EXTRUDE FAILED");
        MEGA = SBY;
        command = "";
      }


      extruder.runSpeed();
      break;


    case COLLECTION:
      digitalWrite(ENABLE_PIN, LOW);

      //Dip
      extruder.move(8000);
      while (extruder.distanceToGo() != 0) {
        extruder.run();
      }

      // Extract
      extruder.move(-4000);
      while (extruder.distanceToGo() != 0) {
        extruder.run();
      }


      extruder.stop();                 // Stop motion
      extruder.setCurrentPosition(0);  // Reset position logic
      extruder.setSpeed(2000);

      Serial.println("FETCH COMPLETED");
      MEGA = WAIT;
      command = "";
      break;


    case PREPCUT:
      digitalWrite(ENABLE_PIN, LOW);
      extruder.moveTo(0);
      while (extruder.distanceToGo() != 0) {
        extruder.run();
      }
      // move other extruder. when finished update Extruder to SBY and respond

      Serial.println("FILAMENT RDY");
      MEGA = SBY;
      command = "";
      break;

    case CUT:
      digitalWrite(EN_PIN_CUT, LOW);
      cutter.move(9100);
      while (cutter.distanceToGo() != 0) {
        cutter.run();
      }
      Serial.println("CUT COMPLETED");
      MEGA = SBY;
      command = "";
      break;

    case CUTOPEN:
      digitalWrite(EN_PIN_CUT, LOW);
      cutter.move(-9000);
      while (cutter.distanceToGo() != 0) {
        cutter.run();
      }
      Serial.println("CUTOPEN COMPLETED");
      MEGA = SBY;
      command = "";
      break;

    case NAIUP:
      analogWrite(LIFT_PIN, 100);
      Serial.println("PLATFORM LIFT UP");
      MEGA = SBY;
      break;

    case NAIDOWN:
      analogWrite(LIFT_PIN, 0);
      Serial.println("PLATFORM LIFT DOWN");
      MEGA = SBY;
      break;


    case WAIT:
      command = "";
      digitalWrite(ENABLE_PIN, HIGH);  // Disable motor
      while (Serial.available() > 0) {
        command = Serial.readStringUntil('\n');
      }

      if (command == "FETCH") {
        Serial.println("FETCH START");
        MEGA = COLLECTION;
        command = "";
      }


      if (command == "EXTRUDE") {
        Serial.println("EXTRUDE START");
        //Reset scale and loadthrehold
        scale.tare();
        currentload = scale.get_units();
        MEGA = EXTRUDE;
        timer = millis();
      }


      else if (command == "PREP CUT") {
        Serial.println("PREP START");
        MEGA = PREPCUT;
        command = "";

      }

      else if (command == "CUT") {
        Serial.println("CUT START");
        MEGA = CUT;
        command = "";
      }

      else if (command == "CUT OPEN") {
        Serial.println("CUT START");
        MEGA = CUTOPEN;
        command = "";
      }

      else if (command == "PLATFORM LIFT UP") {
        MEGA = NAIUP;
        command = "";
      }

      else if (command == "PLATFORM LIFT DOWN") {
        MEGA = NAIDOWN;
        command = "";
      }


      break;

    case SBY:
      digitalWrite(ENABLE_PIN, HIGH);  // Disable motor
      digitalWrite(EN_PIN_CUT, HIGH);
      MEGA = WAIT;
      command = "";

      // do we want dignostics or just that something is wrong? error light?
      break;

    default:
      Serial.println("UNKNOWN STATE");
      MEGA = SBY;
      command = "";
      break;
  }
}
