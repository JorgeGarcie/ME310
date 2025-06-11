/**
 * @file Hardware.h - Cleaned Up Version
 * @brief Hardware control class definition for the Petri Dish Streaker
 * 
 * Cleaned version removing state machine functions and finger controls.
 * Semantic wrapper functions match NUK command interface.
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Dynamixel2Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include "Config.h"

/**
 * @class HardwareControl
 * @brief Main hardware abstraction class for the Petri Dish Streaker
 */
class HardwareControl {
  private:
    // Dynamixel interface
    Dynamixel2Arduino dxl;
    
    // Servo objects for gripper control
    Servo servo1;
    Servo servo2;
    
    // Position tracking variables
    float current_polar_angle;
    float current_platform_angle;
    bool is_initialized;
    bool first_move;
    
    // Platform geometry parameters
    float platform_center_x;
    float platform_center_y;
    float platform_radius;
    
    // Internal utility functions
    uint16_t degToRaw(float degrees);
    float rawToDeg(uint16_t raw);
    void waitForMotors(uint8_t motorId = 0);
    void waitForMotorsMin(uint8_t motorId = 0);
  
    // ADD THESE for extended position tracking:
    float cumulative_platform_degrees;  // Track total rotation
    float last_platform_degrees;        // Previous target in degrees
    
  public:
    HardwareControl();
    int32_t extendedPlatformPosition(float target_degrees);


    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    void initialize();
    void homeAllAxes();

    // ========================================================================
    // SEMANTIC MOVEMENT FUNCTIONS (Match NUK Commands)
    // ========================================================================
    
    // MOVE command implementations
    bool moveToWorkArea();        // MOVE WORK_AREA - handler to streaking station
    bool moveToStorage();         // MOVE STRG - handler to restacker position  
    bool moveToNormal();          // MOVE NORMAL - handler to cartridge 1 position
    bool moveToBlood();           // MOVE BLOOD - handler to cartridge 2 position
    bool moveToChocolat();        // MOVE CHOCOLAT - handler to cartridge 3 position

    // LIFT command implementations
    bool liftStorageTop();    // LIFT STRG UP
    bool liftStorageUp();    // LIFT STRG UP
    bool liftStorageMid();    // LIFT STRG UP
    bool liftStorageDown();  // LIFT STRG DOWN
    bool liftNormalTop();     // LIFT NORMAL UP
    bool liftNormalUp();     // LIFT NORMAL UP
    bool liftNormalMid();     // LIFT NORMAL UP
    bool liftNormalDown();   // LIFT NORMAL DOWN
    bool liftBloodTop();      // LIFT BLOOD UP
    bool liftBloodUp();      // LIFT BLOOD UP
    bool liftBloodMid();      // LIFT BLOOD UP
    bool liftBloodDown();    // LIFT BLOOD DOWN
    bool liftChocolatTop();   // LIFT CHOCOLAT UP
    bool liftChocolatUp();   // LIFT CHOCOLAT UP
    bool liftChocolatMid();   // LIFT CHOCOLAT UP
    bool liftChocolatDown(); // LIFT CHOCOLAT DOWN
    bool liftAllTop();        // LIFT ALL UP
    bool liftAllUp();        // LIFT ALL UP
    bool liftAllMid();      // LIFT ALL DOWN
    bool liftAllDown();      // LIFT ALL DOWN

    // SUCTION command implementations
    bool suctionRotationOn();     // SUCTION ROT ON
    bool suctionRotationOff();    // SUCTION ROT OFF
    bool suctionLidOn();          // SUCTION LID ON
    bool suctionLidOff();         // SUCTION LID OFF

    // LID command implementations  
    bool lidOpen();               // LID OPEN - complete lid removal sequence
    bool lidClose();              // LID CLOSE - complete lid replacement sequence

    // GRAB/RELEASE command implementations (same as gripper)
    bool openGripper();           // GRAB - open gripper
    bool closeGripper();          // RELEASE - close gripper

    // FETCH and sample command implementations
    bool fetchSample();           // FETCH - move to sample collection position
    bool prepareCut();            // CUT - move to cutting position

    // ========================================================================
    // HARDWARE-SPECIFIC FUNCTIONS (Internal Implementation)
    // ========================================================================
    
    // Handler movement functions
    bool rotateToStreakingStation();
    bool rotateHandlerToInitial();
    bool rotateHandlerToFinished();
    bool rotateHandlerToC1();
    bool rotateHandlerToC2();
    bool rotateHandlerToC3();
    
    // Cartridge and restacker functions
    bool moveRestackerTop();
    bool moveRestackerUp();
    bool moveRestackerMid();
    bool moveRestackerDown();
    bool moveCartridgeTop(uint8_t cartridge_id);
    bool moveCartridgeUp(uint8_t cartridge_id);
    bool moveCartridgeMid(uint8_t cartridge_id);
    bool moveCartridgeDown(uint8_t cartridge_id);
    bool homeAllCartridges();
    
    // Platform control functions
    bool platformGearUp();
    bool platformGearDown();
    
    // Suction control functions
    bool platformSuctionOn();
    bool platformSuctionOff();
    bool LidSuctionOn();
    bool LidSuctionOff();
    
    // Lid lifter functions
    bool lowerLidLifter();
    bool raiseLidLifter();
    
    // Polar arm functions
    bool movePolarArmToVial();
    bool movePolarArmToPlatform();
    bool movePolarArmToCutting();
    bool extrude();
    
    // ========================================================================
    // PATTERN DRAWING FUNCTIONS
    // ========================================================================
    
    bool executeStreakPattern(uint8_t pattern_id);
    bool drawPlatformPoint(float x, float y);
    bool moveToCoordinate(float x, float y);
    bool drawLine(float x1, float y1, float x2, float y2, int num_points = 20);
    bool drawCircle(float radius, int num_points = 36);
    bool drawSpiral(float max_radius, float revolutions, int num_points = 50);
    bool drawFlower(float radius, float amplitude, int petals, int num_points = 50);
    
    // ========================================================================
    // UTILITY FUNCTIONS
    // ========================================================================
    
    bool homePosition();
    bool setHandlerGoalPosition(float position);
    bool shakeHandler();
    bool resetEncoder(uint8_t motorId);
    
    // Motor status functions
    uint16_t getMotorPosition(uint8_t motorId);
    bool isMotorMoving(uint8_t motorId);
    
    // Sensor functions (placeholders)
    bool isDishPresent();
    bool areMoreDishesAvailable();
    bool isSampleCollected();
    bool liftAllTopNB();
    bool liftAllUpNB();
    bool liftAllMidNB();
};

#endif // HARDWARE_H