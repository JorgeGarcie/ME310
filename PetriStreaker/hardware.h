/**
 * @file Hardware.h
 * @brief Hardware control class definition for the Petri Dish Streaker
 * 
 * This file defines the HardwareControl class that manages all physical
 * components including Dynamixel motors, servos, pneumatic systems,
 * and I2C communication with the extruder.
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
 * 
 * This class provides high-level control over all hardware components:
 * - Dynamixel servo motors for motion control
 * - Servo motors for gripper control
 * - Pneumatic suction and solenoid systems
 * - I2C communication with extruder
 * - Coordinate transformation and motion planning
 */
class HardwareControl {
  private:
    // Dynamixel interface
    Dynamixel2Arduino dxl;  ///< Dynamixel motor controller interface
    
    // Servo objects for gripper control
    Servo servo1;  ///< First gripper servo
    Servo servo2;  ///< Second gripper servo
    
    // Position tracking variables
    float current_polar_angle;    ///< Current polar arm angle (radians)
    float current_platform_angle; ///< Current platform angle (radians)
    bool is_initialized;          ///< Hardware initialization status
    bool first_move;              ///< Flag for first movement optimization
    
    // Platform geometry parameters
    float platform_center_x;  ///< Platform center X coordinate (mm)
    float platform_center_y;  ///< Platform center Y coordinate (mm)
    float platform_radius;    ///< Platform radius (mm)

    static const uint8_t EXTRUDER_I2C_ADDR = 0x08;  ///< I2C address of extruder controller
    
    /**
     * @brief Send command to extruder via I2C
     * @param cmd Command character to send
     */
    void sendExtruderCommand(char cmd);
    
    // Internal utility functions
    /**
     * @brief Convert degrees to raw motor units
     * @param degrees Angle in degrees
     * @return Raw motor position (0-4095)
     */
    uint16_t degToRaw(float degrees);
    
    /**
     * @brief Convert raw motor units to degrees
     * @param raw Raw motor position (0-4095)
     * @return Angle in degrees
     */
    float rawToDeg(uint16_t raw);
    
    /**
     * @brief Wait for motors to complete movement
     * @param motorId Specific motor ID to wait for (0 = wait for all motors)
     */
    void waitForMotors(uint8_t motorId = 0);
    
  public:
    /**
     * @brief Constructor for HardwareControl class
     */
    HardwareControl();
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    /**
     * @brief Initialize all hardware components
     * 
     * Sets up Dynamixel motors, servos, pneumatic systems, and I2C communication.
     * Must be called before using any other hardware functions.
     */
    void initialize();
    
    // ========================================================================
    // MOVEMENT FUNCTIONS
    // ========================================================================
    
    /**
     * @brief Home all axes to their starting positions
     */
    void homeAllAxes();
    
    /**
     * @brief Move all axes to home position
     * @return true if successful, false otherwise
     */
    bool homePosition();
    
    /**
     * @brief Move to absolute coordinate
     * @param x X coordinate (mm)
     * @param y Y coordinate (mm)
     * @return true if position is reachable and movement successful
     */
    bool moveToCoordinate(float x, float y);
    
    /**
     * @brief Draw a point on the platform at specified coordinates
     * @param x X coordinate relative to platform center (mm)
     * @param y Y coordinate relative to platform center (mm)
     * @return true if position is reachable and movement successful
     */
    bool drawPlatformPoint(float x, float y);
    
    // ========================================================================
    // NEW MOTOR MOVEMENT FUNCTIONS
    // ========================================================================
    
    /**
     * @brief Move restacker to up position
     * @return true if successful
     */
    bool moveRestackerUp();
    
    /**
     * @brief Move restacker to down position  
     * @return true if successful
     */
    bool moveRestackerDown();
    
    /**
     * @brief Move specified cartridge to up position
     * @param cartridge_id Cartridge number (1, 2, or 3)
     * @return true if successful
     */
    bool moveCartridgeUp(uint8_t cartridge_id);
    
    /**
     * @brief Move specified cartridge to down position
     * @param cartridge_id Cartridge number (1, 2, or 3)
     * @return true if successful
     */
    bool moveCartridgeDown(uint8_t cartridge_id);
    
    /**
     * @brief Home all cartridges to down position
     * @return true if successful
     */
    bool homeAllCartridges();
    
    // ========================================================================
    // GRIPPER CONTROL FUNCTIONS
    // ========================================================================
    
    /**
     * @brief Open gripper using servo sequence
     * @return true if successful
     */
    bool openGripper();
    
    /**
     * @brief Close gripper to default position
     * @return true if successful
     */
    bool closeGripper();
    
    // ========================================================================
    // PATTERN DRAWING FUNCTIONS
    // ========================================================================
    
    /**
     * @brief Draw a straight line between two points
     * @param x1 Start X coordinate (mm)
     * @param y1 Start Y coordinate (mm)
     * @param x2 End X coordinate (mm)
     * @param y2 End Y coordinate (mm)
     * @param num_points Number of interpolation points (default: 20)
     * @return true if successful
     */
    bool drawLine(float x1, float y1, float x2, float y2, int num_points = 20);
    
    /**
     * @brief Draw a circle centered at origin
     * @param radius Circle radius (mm)
     * @param num_points Number of points to define circle (default: 36)
     * @return true if successful
     */
    bool drawCircle(float radius, int num_points = 36);
    
    /**
     * @brief Draw a spiral pattern
     * @param max_radius Maximum spiral radius (mm)
     * @param revolutions Number of complete revolutions
     * @param num_points Number of points to define spiral (default: 50)
     * @return true if successful
     */
    bool drawSpiral(float max_radius, float revolutions, int num_points = 50);
    
    /**
     * @brief Draw a flower-like pattern
     * @param radius Base radius (mm)
     * @param amplitude Petal amplitude (mm)
     * @param petals Number of petals
     * @param num_points Number of points to define pattern (default: 50)
     * @return true if successful
     */
    bool drawFlower(float radius, float amplitude, int petals, int num_points = 50);
    
    // ========================================================================
    // STREAKING OPERATIONS
    // ========================================================================
    
    /**
     * @brief Execute predefined streaking pattern
     * @param pattern_id Pattern identifier (0-3)
     * @return true if successful
     */
    bool executeStreakPattern(uint8_t pattern_id);
    
    // ========================================================================
    // STATE-SPECIFIC OPERATIONS
    // ========================================================================
    
    bool doPurgeOperation();      ///< Perform system purge operation
    bool openFirstFingers();      ///< Open first set of gripper fingers
    bool clampFirstFingers();     ///< Clamp first set of gripper fingers
    bool openSecondFingers();     ///< Open second set of gripper fingers
    bool closeSecondFingers();    ///< Close second set of gripper fingers
    bool movePolarArmToVial();    ///< Move polar arm to sample vial position
    bool movePolarArmToPlatform(); ///< Move polar arm to platform position
    bool extrudeSample();         ///< Extrude sample material
    bool retractSample();         ///< Retract sample material
    bool rotateToStreakingStation(); ///< Rotate handler to streaking position
    bool rotateHandlerToInitial(); ///< Rotate handler to initial position
    bool rotateHandlerToFinished(); ///< Rotate handler to finished position
    /**
    * @brief Safely set handler goal position with collision checking
    * @param position Target position for handler
    * @return true if safe to move, false if collision risk detected
    */
    bool setHandlerGoalPosition(float position);
    /**
     * @brief Reset encoder for specified motor
     * @param motorId Motor ID to reset
     * @return true if successful
     */
    bool resetEncoder(uint8_t motorId);
    
    bool platformGearUp();        ///< Raise platform to engage with dish
    
    /**
     * @brief Shake handler to dislodge dishes
     * @return true if successful
     */
    bool shakeHandler();
    
    bool platformGearDown();      ///< Lower platform to disengage from dish
    bool platformSuctionOn();     ///< Turn on platform suction
    bool platformSuctionOff();    ///< Turn off platform suction
    bool LidSuctionOn();          ///< Turn on lid suction
    bool LidSuctionOff();         ///< Turn off lid suction
    bool lowerLidLifter();        ///< Lower lid lifter to contact position
    bool lowerLidLifterNoContact(); ///< Lower lid lifter without contact
    bool raiseLidLifter();        ///< Raise lid lifter to home position
    bool cutFilament();           ///< Cut extruder filament
    
    /**
     * @brief Extrude specified amount of filament
     * @param amount Amount to extrude (units depend on extruder calibration)
     * @return true if successful
     */
    bool extrudeFilament(float amount);
  
    
    // ========================================================================
    // MOTOR CONTROL HELPERS
    // ========================================================================
    
    /**
     * @brief Set motor movement speed
     * @param motorId Motor ID
     * @param speed Speed value
     */
    void setMotorSpeed(uint8_t motorId, uint32_t speed);
    
    /**
     * @brief Get current motor position
     * @param motorId Motor ID
     * @return Current position in raw units
     */
    uint16_t getMotorPosition(uint8_t motorId);
    
    /**
     * @brief Check if motor is currently moving
     * @param motorId Motor ID
     * @return true if motor is moving
     */
    bool isMotorMoving(uint8_t motorId);
    
    // ========================================================================
    // SENSOR FUNCTIONS
    // ========================================================================
    
    bool isDishPresent();         ///< Check if dish is present in handler
    bool areMoreDishesAvailable(); ///< Check if more dishes are available
    bool isSampleCollected();     ///< Check if sample has been collected
};

// Global hardware instance
extern HardwareControl hardware;

#endif // HARDWARE_H