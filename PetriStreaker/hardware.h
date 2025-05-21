#ifndef HARDWARE_H
#define HARDWARE_H

#include <Dynamixel2Arduino.h>
#include <Wire.h>
#include "Config.h"

class HardwareControl {
  private:
    // Dynamixel interface
    Dynamixel2Arduino dxl;
    
    // Position tracking variables
    float current_polar_angle;
    float current_platform_angle;
    bool is_initialized;
    bool first_move;
    
    // Platform geometry
    float platform_center_x;
    float platform_center_y;
    float platform_radius;

    static const uint8_t EXTRUDER_I2C_ADDR = 0x08;  // I2C address of extruder controller
    void sendExtruderCommand(char cmd);  // Helper to send I2C commands
    
    // Internal functions
    uint16_t degToRaw(float degrees);
    float rawToDeg(uint16_t raw);
    void waitForMotors(uint8_t motorId = 0);
    
  public:
    HardwareControl();
    
    // Initialization
    void initialize();
    
    // Movement functions
    void homeAllAxes();
    bool homePosition();
    bool moveToCoordinate(float x, float y);
    bool drawPlatformPoint(float x, float y);
    
    // Pattern drawing functions
    bool drawLine(float x1, float y1, float x2, float y2, int num_points = 20);
    bool drawCircle(float radius, int num_points = 36);
    bool drawSpiral(float max_radius, float revolutions, int num_points = 50);
    bool drawFlower(float radius, float amplitude, int petals, int num_points = 50);
    
    // Streaking specific operations
    bool executeStreakPattern(uint8_t pattern_id);
    
    // State-specific operations
    bool doPurgeOperation();
    bool openFirstFingers();
    bool clampFirstFingers();
    bool openSecondFingers();
    bool closeSecondFingers();
    bool movePolarArmToVial();
    bool movePolarArmToPlatform();
    bool extrudeSample();
    bool retractSample();
    bool rotateToStreakingStation();
    bool rotateHandlerToInitial();
    bool rotateHandlerToFinished();
    bool resetEncoder(uint8_t motorId);
    bool platformGearUp();
    bool shakeHandler();
    bool platformGearDown();
    bool platformSuctionOn();
    bool platformSuctionOff();
    bool LidSuctionOn();
    bool LidSuctionOff();
    bool lowerLidLifter();
    bool lowerLidLifterNoContact();
    bool raiseLidLifter();
    bool cutFilament();
    bool extrudeFilament(float amount);
    bool solenoidLift();
    bool solenoidDown();
    
    // Motor control helpers
    void setMotorSpeed(uint8_t motorId, uint32_t speed);
    uint16_t getMotorPosition(uint8_t motorId);
    bool isMotorMoving(uint8_t motorId);
    
    // Sensor functions
    bool isDishPresent();
    bool areMoreDishesAvailable();
    bool isSampleCollected();
};

// Global hardware instance
extern HardwareControl hardware;

#endif // HARDWARE_H