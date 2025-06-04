/**
 * @file Hardware.cpp - Complete Cleaned Version with Extended Position Logic
 * @brief Implementation of hardware control functions for the Petri Dish Streaker
 * 
 * Updated version with extended position tracking for platform motor to avoid discontinuities.
 */

#include "Hardware.h"
#include <math.h>
#include <Servo.h>

/**
 * @brief Constructor - Initialize hardware control object
 */
HardwareControl::HardwareControl()
  : dxl(DXL_SERIAL, DXL_DIR_PIN) {
  current_polar_angle = 0.0f;
  current_platform_angle = 0.0f;
  is_initialized = false;
  first_move = true;

  // Initialize extended position tracking for platform motor
  cumulative_platform_degrees = 0.0f;
  last_platform_degrees = 0.0f;

  // Platform geometry settings
  platform_center_x = 70.0f;  // Cx
  platform_center_y = 70.0f;  // Cy
  platform_radius = 45.0f;    // Rplat
}

/**
 * @brief Extended position tracking for platform motor to avoid discontinuities
 */
int32_t HardwareControl::extendedPlatformPosition(float target_degrees) {
  // Calculate the angular difference
  float diff = target_degrees - last_platform_degrees;

  // Normalize to [-180, 180] to find shortest path
  while (diff > 180.0f) diff -= 360.0f;
  while (diff < -180.0f) diff += 360.0f;

  // Update cumulative angle
  cumulative_platform_degrees += diff;
  last_platform_degrees = target_degrees;

  // Convert to raw position (can exceed 4095)
  int32_t raw_position = (int32_t)((cumulative_platform_degrees / 360.0f) * 4096.0f);

  DEBUG_SERIAL.print("Platform: target=");
  DEBUG_SERIAL.print(target_degrees);
  DEBUG_SERIAL.print("°, cumulative=");
  DEBUG_SERIAL.print(cumulative_platform_degrees);
  DEBUG_SERIAL.print("°, raw=");
  DEBUG_SERIAL.println(raw_position);

  return raw_position;
}

/**
 * @brief Initialize all hardware components
 */
void HardwareControl::initialize() {
  DEBUG_SERIAL.println("Initializing hardware...");

  // Initialize I2C for extruder control
  //Wire.begin();

  // Initialize Dynamixel
  dxl.begin(DXL_BAUD_RATE);
  dxl.setPortProtocolVersion(2.0);

  // Initialize lid lifter motor
  dxl.torqueOff(DXL_LID_LIFTER);
  dxl.setOperatingMode(DXL_LID_LIFTER, OP_POSITION);
  dxl.torqueOn(DXL_LID_LIFTER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_LID_LIFTER, LID_LIFTER_SPEED);

  // Initialize polar arm motor
  dxl.torqueOff(DXL_POLAR_ARM);
  dxl.setOperatingMode(DXL_POLAR_ARM, OP_POSITION);
  dxl.torqueOn(DXL_POLAR_ARM);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_POLAR_ARM, POLAR_ARM_SPEED);

  // Initialize platform motor with EXTENDED POSITION MODE to avoid discontinuities
  dxl.torqueOff(DXL_PLATFORM);
  dxl.setOperatingMode(DXL_PLATFORM, OP_EXTENDED_POSITION);  // Changed to extended position
  dxl.torqueOn(DXL_PLATFORM);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_PLATFORM, PLATFORM_SPEED);

  // Initialize handler motor
  dxl.torqueOff(DXL_HANDLER);
  dxl.setOperatingMode(DXL_HANDLER, OP_EXTENDED_POSITION);
  dxl.torqueOn(DXL_HANDLER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_HANDLER, HANDLER_SPEED);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_ACCELERATION, DXL_HANDLER, HANDLER_ACCEL);

  // Initialize restacker motor
  dxl.torqueOff(DXL_RESTACKER);
  dxl.setOperatingMode(DXL_RESTACKER, OP_EXTENDED_POSITION);
  dxl.torqueOn(DXL_RESTACKER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_RESTACKER, RESTACKER_SPEED);

  // Initialize Cartridge motors
  dxl.torqueOff(DXL_CARTRIDGE1);
  dxl.setOperatingMode(DXL_CARTRIDGE1, OP_EXTENDED_POSITION);
  dxl.torqueOn(DXL_CARTRIDGE1);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_CARTRIDGE1, CARTRIDGE1_SPEED);

  dxl.torqueOff(DXL_CARTRIDGE2);
  dxl.setOperatingMode(DXL_CARTRIDGE2, OP_EXTENDED_POSITION);
  dxl.torqueOn(DXL_CARTRIDGE2);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_CARTRIDGE2, CARTRIDGE2_SPEED);

  dxl.torqueOff(DXL_CARTRIDGE3);
  dxl.setOperatingMode(DXL_CARTRIDGE3, OP_EXTENDED_POSITION);
  dxl.torqueOn(DXL_CARTRIDGE3);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_CARTRIDGE3, CARTRIDGE3_SPEED);

  // Initialize servo pins (check if these are defined in config.h)
  // servo1.attach(SERVO1_PIN);  // Uncomment if servo pins are defined
  // servo2.attach(SERVO2_PIN);  // Uncomment if servo pins are defined

  // Initialize pneumatic relays
  pinMode(LID_SUCTION, OUTPUT);
  pinMode(LID_SOLENOID, OUTPUT);
  pinMode(PLATFORM_SUCTION, OUTPUT);
  pinMode(PLATFORM_SOLENOID, OUTPUT);
  digitalWrite(LID_SUCTION, LOW);
  digitalWrite(LID_SOLENOID, LOW);
  digitalWrite(PLATFORM_SUCTION, LOW);
  digitalWrite(PLATFORM_SOLENOID, LOW);

  // Set initialized flag
  is_initialized = true;

  // Home all axes
  homeAllAxes();

  DEBUG_SERIAL.println("Hardware initialization complete");
}

/**
 * @brief Home all motors to their starting positions
 */
void HardwareControl::homeAllAxes() {
  DEBUG_SERIAL.println("Homing all axes...");

  // First: Home restacker, cartridges and platform
  DEBUG_SERIAL.println("Homing restacker and cartridges...");
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_HOME);
  dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_HOME);
  dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_HOME);
  dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_HOME);
  dxl.setGoalPosition(DXL_PLATFORM, (uint32_t)PLATFORM_HOME);

  // Initialize extended position tracking variables to home position
  cumulative_platform_degrees = (PLATFORM_HOME / 4096.0f) * 360.0f;
  last_platform_degrees = cumulative_platform_degrees;

  // Wait for completion
  waitForMotors();

  // Then: Home main motion system
  DEBUG_SERIAL.println("Homing main motion system...");
  dxl.setGoalPosition(DXL_LID_LIFTER, (uint32_t)LID_LIFTER_HOME);
  waitForMotors(DXL_LID_LIFTER);
  dxl.setGoalPosition(DXL_POLAR_ARM, (uint32_t)POLAR_ARM_TO_VIAL);
  dxl.setGoalPosition(DXL_HANDLER, (uint32_t)HANDLER_HOME);

  // Wait for all motors to reach position
  waitForMotors();

  // Reset current positions
  current_polar_angle = 0.0f;
  current_platform_angle = 0.0f;
  first_move = true;

  DEBUG_SERIAL.println("All axes homed");
}

/**
 * @brief Wait for motors to complete their movement
 */
void HardwareControl::waitForMotors(uint8_t motorId) {
  uint32_t m1, m2, m3, m4, m5, m6, m7, m8;

  // Initial delay to allow motor controller to update status registers
  delay(50);

  // Initial check to see if motors are moving
  if (motorId > 0) {
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    m2 = m3 = m4 = m5 = m6 = m7 = m8 = 0;
  } else {
    // Check all 8 motors
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
    m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
    m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
    m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
    m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
    m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
    m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
    m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
  }

  // If no motors appear to be moving initially, wait a bit more and check again
  if (m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0 && m5 == 0 && m6 == 0 && m7 == 0 && m8 == 0) {
    delay(50);

    // Double-check
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
      m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
      m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
      m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
    }

    // If still no movement detected, assume completed quickly
    if (m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0 && m5 == 0 && m6 == 0 && m7 == 0 && m8 == 0) {
      return;
    }
  }

  // Main waiting loop
  do {
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
      m2 = m3 = m4 = m5 = m6 = m7 = m8 = 0;
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
      m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
      m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
      m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
    }
    delay(5);
  } while (m1 == 1 || m2 == 1 || m3 == 1 || m4 == 1 || m5 == 1 || m6 == 1 || m7 == 1 || m8 == 1);
}


/**
 * @brief Wait for motors to complete their movement
 */
void HardwareControl::waitForMotorsMin(uint8_t motorId) {
  uint32_t m1, m2, m3, m4, m5, m6, m7, m8;

  // Initial delay to allow motor controller to update status registers
  delay(10);

  // Initial check to see if motors are moving
  if (motorId > 0) {
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    m2 = m3 = m4 = m5 = m6 = m7 = m8 = 0;
  } else {
    // Check all 8 motors
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
    m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
    m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
    m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
    m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
    m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
    m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
    m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
  }

  // If no motors appear to be moving initially, wait a bit more and check again
  if (m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0 && m5 == 0 && m6 == 0 && m7 == 0 && m8 == 0) {
    delay(10);

    // Double-check
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
      m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
      m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
      m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
    }

    // If still no movement detected, assume completed quickly
    if (m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0 && m5 == 0 && m6 == 0 && m7 == 0 && m8 == 0) {
      return;
    }
  }

  // Main waiting loop
  do {
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
      m2 = m3 = m4 = m5 = m6 = m7 = m8 = 0;
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m5 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_RESTACKER);
      m6 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE1);
      m7 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE2);
      m8 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_CARTRIDGE3);
    }
    delay(1);
  } while (m1 == 1 || m2 == 1 || m3 == 1 || m4 == 1 || m5 == 1 || m6 == 1 || m7 == 1 || m8 == 1);
}
// ============================================================================
// SEMANTIC WRAPPER FUNCTIONS (MATCH NUK COMMANDS)
// ============================================================================

/**
 * @brief Move handler to work area (streaking station)
 * MOVE WORK_AREA command implementation
 */
bool HardwareControl::moveToWorkArea() {
  DEBUG_SERIAL.println("Moving to work area (streaking station)");
  return rotateToStreakingStation();
}

/**
 * @brief Move handler to storage position (restacker)
 * MOVE STRG command implementation
 */
bool HardwareControl::moveToStorage() {
  DEBUG_SERIAL.println("Moving to storage position");
  return rotateHandlerToFinished();
}

/**
 * @brief Move handler to normal cartridge position
 * MOVE NORMAL command implementation
 */
bool HardwareControl::moveToNormal() {
  DEBUG_SERIAL.println("Moving to normal cartridge position");
  return rotateHandlerToC1();
}

/**
 * @brief Move handler to blood cartridge position
 * MOVE BLOOD command implementation
 */
bool HardwareControl::moveToBlood() {
  DEBUG_SERIAL.println("Moving to blood cartridge position");
  return rotateHandlerToC2();
}

/**
 * @brief Move handler to chocolat cartridge position
 * MOVE CHOCOLAT command implementation
 */
bool HardwareControl::moveToChocolat() {
  DEBUG_SERIAL.println("Moving to chocolat cartridge position");
  return rotateHandlerToC3();
}

// ============================================================================
// LIFT SEMANTIC WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Lift storage (restacker) up
 * LIFT STRG Top command implementation
 */
bool HardwareControl::liftStorageTop() {
  DEBUG_SERIAL.println("Lifting storage top");
  return moveRestackerTop();
}

/**
 * @brief Lift storage (restacker) up
 * LIFT STRG UP command implementation
 */
bool HardwareControl::liftStorageUp() {
  DEBUG_SERIAL.println("Lifting storage up");
  return moveRestackerUp();
}

/**
 * @brief Lift storage (restacker) mid
 * LIFT STRG MID command implementation
 */
bool HardwareControl::liftStorageMid() {
  DEBUG_SERIAL.println("Lifting storage mid");
  return moveRestackerMid();
}

/**
 * @brief Lift storage (restacker) down
 * LIFT STRG DOWN command implementation
 */
bool HardwareControl::liftStorageDown() {
  DEBUG_SERIAL.println("Lifting storage down");
  return moveRestackerDown();
}

/**
 * @brief Lift normal cartridge top
 * LIFT NORMAL UP command implementation
 */
bool HardwareControl::liftNormalTop() {
  DEBUG_SERIAL.println("Lifting normal cartridge top");
  return moveCartridgeTop(1);
}

/**
 * @brief Lift normal cartridge up
 * LIFT NORMAL UP command implementation
 */
bool HardwareControl::liftNormalUp() {
  DEBUG_SERIAL.println("Lifting normal cartridge up");
  return moveCartridgeUp(1);
}

/**
 * @brief Lift normal cartridge mid
 * LIFT NORMAL UP command implementation
 */
bool HardwareControl::liftNormalMid() {
  DEBUG_SERIAL.println("Lifting normal cartridge mid");
  return moveCartridgeMid(1);
}

/**
 * @brief Lift normal cartridge down
 * LIFT NORMAL DOWN command implementation
 */
bool HardwareControl::liftNormalDown() {
  DEBUG_SERIAL.println("Lifting normal cartridge down");
  return moveCartridgeDown(1);
}

/**
 * @brief Lift blood cartridge top
 * LIFT NORMAL UP command implementation
 */
bool HardwareControl::liftBloodTop() {
  DEBUG_SERIAL.println("Lifting blood cartridge top");
  return moveCartridgeTop(2);
}

/**
 * @brief Lift blood cartridge up
 * LIFT BLOOD UP command implementation
 */
bool HardwareControl::liftBloodUp() {
  DEBUG_SERIAL.println("Lifting blood cartridge up");
  return moveCartridgeUp(2);
}

/**
 * @brief Lift blood cartridge mid
 * LIFT BLOOD UP command implementation
 */
bool HardwareControl::liftBloodMid() {
  DEBUG_SERIAL.println("Lifting blood cartridge mid");
  return moveCartridgeMid(2);
}

/**
 * @brief Lift blood cartridge down
 * LIFT BLOOD DOWN command implementation
 */
bool HardwareControl::liftBloodDown() {
  DEBUG_SERIAL.println("Lifting blood cartridge down");
  return moveCartridgeDown(2);
}

/**
 * @brief Lift chocolat cartridge up
 * LIFT CHOCOLAT UP command implementation
 */
bool HardwareControl::liftChocolatTop() {
  DEBUG_SERIAL.println("Lifting chocolat cartridge top");
  return moveCartridgeTop(3);
}

/**
 * @brief Lift chocolat cartridge up
 * LIFT CHOCOLAT UP command implementation
 */
bool HardwareControl::liftChocolatUp() {
  DEBUG_SERIAL.println("Lifting chocolat cartridge up");
  return moveCartridgeUp(3);
}

/**
 * @brief Lift chocolat cartridge mid
 * LIFT CHOCOLAT DOWN command implementation
 */
bool HardwareControl::liftChocolatMid() {
  DEBUG_SERIAL.println("Lifting chocolat cartridge mid");
  return moveCartridgeMid(3);
}

/**
 * @brief Lift chocolat cartridge down
 * LIFT CHOCOLAT DOWN command implementation
 */
bool HardwareControl::liftChocolatDown() {
  DEBUG_SERIAL.println("Lifting chocolat cartridge down");
  return moveCartridgeDown(3);
}

/**
 * @brief Lift all cartridges up
 * LIFT ALL UP command implementation
 */
bool HardwareControl::liftAllTop() {
  DEBUG_SERIAL.println("Lifting all cartridges top");
  bool success = true;
  success &= moveCartridgeTop(1);
  success &= moveCartridgeTop(2);
  success &= moveCartridgeTop(3);
  success &= moveCartridgeTop(4);
  return success;
}

bool HardwareControl::liftAllTopNB(){
  dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_TOP);
  dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_TOP);
  dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_TOP);
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_TOP);
  waitForMotors();
  return true;
}

/**
 * @brief Lift all cartridges up
 * LIFT ALL UP command implementation
 */
bool HardwareControl::liftAllUp() {
  DEBUG_SERIAL.println("Lifting all cartridges up");
  bool success = true;
  success &= moveCartridgeUp(1);
  success &= moveCartridgeUp(2);
  success &= moveCartridgeUp(3);
  success &= moveCartridgeUp(4);
  return success;
}

bool HardwareControl::liftAllUpNB(){
  dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_UP);
  dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_UP);
  dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_UP);
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_UP);
  waitForMotors();
  return true;
}

/**
 * @brief Lift all cartridges mid
 * LIFT ALL UP command implementation
 */
bool HardwareControl::liftAllMid() {
  DEBUG_SERIAL.println("Lifting all cartridges mid");
  bool success = true;
  success &= moveCartridgeMid(1);
  success &= moveCartridgeMid(2);
  success &= moveCartridgeMid(3);
  success &= moveCartridgeMid(4);
  return success;
}
bool HardwareControl::liftAllMidNB(){
  dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_MID);
  dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_MID);
  dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_MID);
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_MID);
  waitForMotors();
  return true;
}


/**
 * @brief Lift all cartridges down
 * LIFT ALL DOWN command implementation
 */
bool HardwareControl::liftAllDown() {
  DEBUG_SERIAL.println("Lifting all cartridges down");
  return homeAllCartridges();
}

// ============================================================================
// SUCTION SEMANTIC WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Turn on rotation platform suction
 * SUCTION ROT ON command implementation
 */
bool HardwareControl::suctionRotationOn() {
  DEBUG_SERIAL.println("Rotation suction ON");
  return platformSuctionOn();
}

/**
 * @brief Turn off rotation platform suction
 * SUCTION ROT OFF command implementation
 */
bool HardwareControl::suctionRotationOff() {
  DEBUG_SERIAL.println("Rotation suction OFF");
  return platformSuctionOff();
}

/**
 * @brief Turn on lid suction
 * SUCTION LID ON command implementation
 */
bool HardwareControl::suctionLidOn() {
  DEBUG_SERIAL.println("Lid suction ON");
  return LidSuctionOn();
}

/**
 * @brief Turn off lid suction
 * SUCTION LID OFF command implementation
 */
bool HardwareControl::suctionLidOff() {
  DEBUG_SERIAL.println("Lid suction OFF");
  return LidSuctionOff();
}

// ============================================================================
// LID SEMANTIC WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Open lid (complete removal sequence)
 * LID OPEN command implementation
 */
bool HardwareControl::lidOpen() {
  DEBUG_SERIAL.println("Opening lid (removal sequence)");

  bool success = true;

  // Complete lid removal sequence
  success &= suctionLidOn();
  delay(200);
  success &= lowerLidLifter();
  delay(500);
  success &= raiseLidLifter();

  return success;
}

/**
 * @brief Close lid (complete replacement sequence)
 * LID CLOSE command implementation
 */
bool HardwareControl::lidClose() {
  DEBUG_SERIAL.println("Closing lid (replacement sequence)");

  bool success = true;

  // Complete lid replacement sequence
  success &= lowerLidLifter();
  delay(100);
  success &= suctionLidOff();
  delay(300);
  success &= raiseLidLifter();

  return success;
}

// ============================================================================
// FETCH AND SAMPLE SEMANTIC WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Move to sample fetch position
 * FETCH command implementation
 */
bool HardwareControl::fetchSample() {
  DEBUG_SERIAL.println("Moving to fetch sample position");
  return movePolarArmToVial();
}

/**
 * @brief Prepare for cutting operation
 * CUT command implementation
 */
bool HardwareControl::prepareCut() {
  DEBUG_SERIAL.println("Preparing for cut operation");
  return movePolarArmToCutting();
}

/**
 * @brief Prepare for extruding operation
 * EXTRUDE command implementation
 */
bool HardwareControl::extrude() {
  DEBUG_SERIAL.println("Preparing for cut operation");
  return movePolarArmToPlatform();
}

// ============================================================================
// CARTRIDGE AND RESTACKER MOVEMENT FUNCTIONS
// ============================================================================
/**
 * @brief Move restacker to top position
 * @return true if successful
 */
bool HardwareControl::moveRestackerTop() {
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_TOP);
  waitForMotors(DXL_RESTACKER);
  return true;
}

/**
 * @brief Move restacker to up position
 * @return true if successful
 */
bool HardwareControl::moveRestackerUp() {
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_UP);
  waitForMotors(DXL_RESTACKER);
  return true;
}

/**
 * @brief Move restacker to mid position
 * @return true if successful
 */
bool HardwareControl::moveRestackerMid() {
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_MID);
  waitForMotors(DXL_RESTACKER);
  return true;
}

/**
 * @brief Move restacker to down position  
 * @return true if successful
 */
bool HardwareControl::moveRestackerDown() {
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_HOME);
  waitForMotors(DXL_RESTACKER);
  return true;
}

/**
 * @brief Move specified cartridge to top position
 * @param cartridge_id Cartridge number (1, 2, or 3)
 * @return true if successful
 */
bool HardwareControl::moveCartridgeTop(uint8_t cartridge_id) {
  switch (cartridge_id) {
    case 1:
      dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_TOP);
      waitForMotors(DXL_CARTRIDGE1);
      break;
    case 2:
      dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_TOP);
      waitForMotors(DXL_CARTRIDGE2);
      break;
    case 3:
      dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_TOP);
      waitForMotors(DXL_CARTRIDGE3);
      break;
    case 4:
      dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_TOP);
      waitForMotors(DXL_RESTACKER);
      break;
    default:
      return false;
  }
  return true;
}

/**
 * @brief Move specified cartridge to up position
 * @param cartridge_id Cartridge number (1, 2, or 3)
 * @return true if successful
 */
bool HardwareControl::moveCartridgeUp(uint8_t cartridge_id) {
  switch (cartridge_id) {
    case 1:
      dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_UP);
      waitForMotors(DXL_CARTRIDGE1);
      break;
    case 2:
      dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_UP);
      waitForMotors(DXL_CARTRIDGE2);
      break;
    case 3:
      dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_UP);
      waitForMotors(DXL_CARTRIDGE3);
      break;
    case 4:
      dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_UP);
      waitForMotors(DXL_RESTACKER);
      break;
    default:
      return false;
  }
  return true;
}

/**
 * @brief Move specified cartridge to mid position
 * @param cartridge_id Cartridge number (1, 2, or 3)
 * @return true if successful
 */
bool HardwareControl::moveCartridgeMid(uint8_t cartridge_id) {
  switch (cartridge_id) {
    case 1:
      dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_MID);
      waitForMotors(DXL_CARTRIDGE1);
      break;
    case 2:
      dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_MID);
      waitForMotors(DXL_CARTRIDGE2);
      break;
    case 3:
      dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_MID);
      waitForMotors(DXL_CARTRIDGE3);
      break;
    case 4:
      dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_MID);
      waitForMotors(DXL_RESTACKER);
      break;
    default:
      return false;
  }
  return true;
}

/**
 * @brief Move specified cartridge to down position
 * @param cartridge_id Cartridge number (1, 2, or 3)
 * @return true if successful
 */
bool HardwareControl::moveCartridgeDown(uint8_t cartridge_id) {
  switch (cartridge_id) {
    case 1:
      dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_HOME);
      waitForMotors(DXL_CARTRIDGE1);
      break;
    case 2:
      dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_HOME);
      waitForMotors(DXL_CARTRIDGE2);
      break;
    case 3:
      dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_HOME);
      waitForMotors(DXL_CARTRIDGE3);
      break;
    default:
      return false;
  }
  return true;
}

/**
 * @brief Home all cartridges to down position
 * @return true if successful
 */
bool HardwareControl::homeAllCartridges() {
  dxl.setGoalPosition(DXL_CARTRIDGE1, CARTRIDGE1_HOME);
  dxl.setGoalPosition(DXL_CARTRIDGE2, CARTRIDGE2_HOME);
  dxl.setGoalPosition(DXL_CARTRIDGE3, CARTRIDGE3_HOME);
  dxl.setGoalPosition(DXL_RESTACKER, RESTACKER_HOME);
  waitForMotors();
  return true;
}

// ============================================================================
// SERVO CONTROL FUNCTIONS (GRIPPER)
// ============================================================================

/**
 * @brief Open gripper using servo sequence
 * @return true if successful
 */
bool HardwareControl::openGripper() {
  // Step 1: Move Servo 1 to 110
  servo1.write(110);
  delay(2000);

  // Step 2: Move Servo 2 to 40
  servo2.write(40);
  delay(3000);

  // Step 3: Return Servo 2 to 90
  servo2.write(90);
  delay(2000);

  // Step 4: Return Servo 1 to 60
  servo1.write(60);
  delay(1000);

  return true;
}

/**
 * @brief Close gripper to default position
 * @return true if successful
 */
bool HardwareControl::closeGripper() {
  servo1.write(60);
  servo2.write(90);
  return true;
}

// ============================================================================
// HANDLER MOVEMENT FUNCTIONS
// ============================================================================

bool HardwareControl::rotateToStreakingStation() {
  if (!setHandlerGoalPosition(STREAKING_STATION)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::rotateHandlerToInitial() {
  if (!setHandlerGoalPosition(HANDLER_HOME)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::rotateHandlerToC1() {
  if (!setHandlerGoalPosition(HANDLER_C1)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::rotateHandlerToC2() {
  if (!setHandlerGoalPosition(HANDLER_C2)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::rotateHandlerToC3() {
  if (!setHandlerGoalPosition(HANDLER_C3)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::rotateHandlerToFinished() {
  if (!setHandlerGoalPosition(HANDLER_RESTACKER)) {
    return false;
  }
  waitForMotors();
  return true;
}

bool HardwareControl::setHandlerGoalPosition(float position) {
  // Safety threshold - motors considered "up" if above home + threshold
  float thres = 50;

  // Get current positions and check if motors are in "up" positions
  float platform_pos = dxl.getPresentPosition(DXL_PLATFORM);
  float restacker_pos = dxl.getPresentPosition(DXL_RESTACKER);
  float c1_pos = dxl.getPresentPosition(DXL_CARTRIDGE1);
  float c2_pos = dxl.getPresentPosition(DXL_CARTRIDGE2);
  float c3_pos = dxl.getPresentPosition(DXL_CARTRIDGE3);

  // Check if any motor is above its home position (indicating "up" state)
  if (platform_pos > (PLATFORM_HOME + thres) || restacker_pos > (RESTACKER_HOME + thres) || c1_pos > (CARTRIDGE1_HOME + thres) || c2_pos > (CARTRIDGE2_HOME + thres) || c3_pos > (CARTRIDGE3_HOME + thres)) {

    DEBUG_SERIAL.println("ERROR: Motors in UP position! Can't move handler safely!");
    return false;
  }

  // Safe to move - set goal position
  dxl.setGoalPosition(DXL_HANDLER, position);
  return true;
}

// ============================================================================
// PLATFORM CONTROL FUNCTIONS *NOT USED SINCE HANDLED IN ARDUINO*
// ============================================================================

bool HardwareControl::platformGearUp() {
  dxl.setGoalPosition(DXL_PLATFORM, PLATFORM_UP);
  waitForMotors();
  return true;
}

bool HardwareControl::platformGearDown() {
  dxl.setGoalPosition(DXL_PLATFORM, PLATFORM_HOME);
  waitForMotors();
  return true;
}

// ============================================================================
// SUCTION CONTROL FUNCTIONS
// ============================================================================

bool HardwareControl::platformSuctionOn() {
  digitalWrite(PLATFORM_SUCTION, HIGH);
  return true;
}

bool HardwareControl::platformSuctionOff() {
  digitalWrite(PLATFORM_SOLENOID, HIGH);
  digitalWrite(PLATFORM_SUCTION, LOW);
  delay(10);
  digitalWrite(PLATFORM_SOLENOID, LOW);
  return true;
}

bool HardwareControl::LidSuctionOn() {
  digitalWrite(LID_SUCTION, HIGH);
  return true;
}

bool HardwareControl::LidSuctionOff() {
  digitalWrite(LID_SOLENOID, HIGH);
  digitalWrite(LID_SUCTION, LOW);
  delay(100);
  digitalWrite(LID_SOLENOID, LOW);
  return true;
}

// ============================================================================
// LID LIFTER FUNCTIONS
// ============================================================================

bool HardwareControl::lowerLidLifter() {
  dxl.setGoalPosition(DXL_LID_LIFTER, LID_LIFTER_DOWN);
  waitForMotors();
  return true;
}

bool HardwareControl::raiseLidLifter() {
  dxl.setGoalPosition(DXL_LID_LIFTER, LID_LIFTER_HOME);
  waitForMotors();
  return true;
}

// ============================================================================
// POLAR ARM FUNCTIONS
// ============================================================================

bool HardwareControl::movePolarArmToVial() {
  dxl.setGoalPosition(DXL_POLAR_ARM, POLAR_ARM_TO_VIAL);
  waitForMotors();
  return true;
}

bool HardwareControl::movePolarArmToCutting() {
  dxl.setGoalPosition(DXL_POLAR_ARM, POLAR_ARM_TO_CUT);
  waitForMotors();
  return true;
}

bool HardwareControl::movePolarArmToPlatform() {
  dxl.setGoalPosition(DXL_POLAR_ARM, POLAR_ARM_SWABBING);
  waitForMotors();
  return true;
}

// ============================================================================
// COORDINATE SYSTEM AND PATTERNS
// ============================================================================

uint16_t HardwareControl::degToRaw(float degrees) {
  float d = fmod(degrees, 360.0f);
  if (d < 0) d += 360.0f;
  return uint16_t((d / 360.0f) * 4096.0f) & 0x0FFF;
}

float HardwareControl::rawToDeg(uint16_t raw) {
  return (raw & 0x0FFF) * 360.0f / 4096.0f;
}

bool HardwareControl::homePosition() {
  dxl.setGoalPosition(DXL_PLATFORM, (uint32_t)PLATFORM_HOME);
  waitForMotors(DXL_PLATFORM);
  dxl.setGoalPosition(DXL_LID_LIFTER, (uint32_t)LID_LIFTER_HOME);
  waitForMotors(DXL_LID_LIFTER);
  dxl.setGoalPosition(DXL_POLAR_ARM, (uint32_t)POLAR_ARM_NO_OBSTRUCT_HOME);
  dxl.setGoalPosition(DXL_HANDLER, (uint32_t)HANDLER_HOME);
  waitForMotors();
  return true;
}

bool HardwareControl::drawPlatformPoint(float rx, float ry) {
  // Constrain to platform radius
  float r = sqrt(rx * rx + ry * ry);
  
  if (r < 1.0f) {  // Within 1mm of center
    DEBUG_SERIAL.println("Skipping near-origin point (geometric singularity)");
    return true;  // Skip this point, continue pattern
  }
  
  if (r > platform_radius) {
    rx = (platform_radius / r) * rx;
    ry = (platform_radius / r) * ry;
  }

  // Calculate original point angle in platform coordinates
  float original_angle = atan2(ry, rx);
  float platform_radius_point = sqrt(rx * rx + ry * ry);

  // Find intersection of circles
  float center_dist = sqrt(platform_center_x * platform_center_x + platform_center_y * platform_center_y);

  // Check if circles can intersect
  if (center_dist > POLAR_ARM_LENGTH + platform_radius_point || center_dist < abs(POLAR_ARM_LENGTH - platform_radius_point)) {
    DEBUG_SERIAL.print("center_dist: ");
    DEBUG_SERIAL.println(center_dist);
    DEBUG_SERIAL.print("polar arm length: ");
    DEBUG_SERIAL.println(POLAR_ARM_LENGTH);
    DEBUG_SERIAL.print("platfrom radiu point: ");
    DEBUG_SERIAL.println(platform_radius_point);

    return false;
  }

  // Calculate intersection parameters
  float a = (POLAR_ARM_LENGTH * POLAR_ARM_LENGTH - platform_radius_point * platform_radius_point + center_dist * center_dist) / (2.0f * center_dist);
  float h = sqrt(POLAR_ARM_LENGTH * POLAR_ARM_LENGTH - a * a);

  // Center point along the line between circle centers
  float x2 = platform_center_x * a / center_dist;
  float y2 = platform_center_y * a / center_dist;

  // Calculate both possible intersections
  float intersection1_x = x2 + h * (-platform_center_y) / center_dist;
  float intersection1_y = y2 + h * platform_center_x / center_dist;

  float intersection2_x = x2 - h * (-platform_center_y) / center_dist;
  float intersection2_y = y2 - h * platform_center_x / center_dist;

  // Calculate lever angles for both solutions
  float lever_angle1 = atan2(intersection1_y, intersection1_x);
  float lever_angle2 = atan2(intersection2_y, intersection2_x);

  // Choose solution with minimum movement
  float theta1, theta2;
  float movement1, movement2;

  if (first_move) {
    movement1 = abs(atan2(intersection1_y - platform_center_y, intersection1_x - platform_center_x) - original_angle);
    movement2 = abs(atan2(intersection2_y - platform_center_y, intersection2_x - platform_center_x) - original_angle);
    first_move = false;
  } else {
    movement1 = abs(lever_angle1 - current_polar_angle) + abs((atan2(intersection1_y - platform_center_y, intersection1_x - platform_center_x) - original_angle) - current_platform_angle);
    movement2 = abs(lever_angle2 - current_polar_angle) + abs((atan2(intersection2_y - platform_center_y, intersection2_x - platform_center_x) - original_angle) - current_platform_angle);
  }
  //DEBUG_SERIAL.println("MOVEMENT 1");
  //DEBUG_SERIAL.println(movement1);
  //DEBUG_SERIAL.println("MOVEMENT 2");
  // /DEBUG_SERIAL.println(movement2);
  if (movement1 <= movement2) {
    theta1 = lever_angle1;
    theta2 = atan2(intersection1_y - platform_center_y, intersection1_x - platform_center_x) - original_angle;
  } else {
    theta1 = lever_angle2;
    theta2 = atan2(intersection2_y - platform_center_y, intersection2_x - platform_center_x) - original_angle;
  }

  // Store current positions
  current_polar_angle = theta1;
  current_platform_angle = theta2;

  // Convert to motor positions
  // Polar arm: standard position control
  float deg1 = fmod(degrees(theta1), 360.0f);
  if (deg1 < 0) deg1 += 360.0f;
  deg1 = deg1 + (POLAR_ARM_HOME / 4096.0f * 360.0f);

  // Platform: use extended position control to avoid discontinuities
  float deg2 = degrees(theta2) + (PLATFORM_HOME / 4096.0f * 360.0f);
  //DEBUG_SERIAL.println("CHECK 1");
  // Set motor positions
  dxl.setGoalPosition(DXL_POLAR_ARM, degToRaw(deg1));
  dxl.setGoalPosition(DXL_PLATFORM, extendedPlatformPosition(deg2));  // Use extended position

  waitForMotorsMin();
  //DEBUG_SERIAL.println("CHECK 2");
  return true;
}

bool HardwareControl::moveToCoordinate(float x, float y) {
  return drawPlatformPoint(x, y);
}

bool HardwareControl::drawLine(float x1, float y1, float x2, float y2, int num_points) {
  num_points = max(2, num_points);
  bool success = true;

  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);
    float rx = x1 + t * (x2 - x1);
    float ry = y1 + t * (y2 - y1);

    if (!drawPlatformPoint(rx, ry)) {
      success = false;
    }
  }

  return success;
}

bool HardwareControl::drawCircle(float radius, int num_points) {
  bool success = true;

  for (int i = 0; i < num_points; i++) {
    float angle = (2.0f * PI * i) / num_points;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);

    if (!drawPlatformPoint(rx, ry)) {
      success = false;
    }
  }

  return success;
}

bool HardwareControl::drawSpiral(float max_radius, float revolutions, int num_points) {
  bool success = true;

  for (int i = 0; i < num_points; i++) {
    float t = (float) i / (num_points - 1);
    t*=-1;
    float angle = t * revolutions * 2.0f * PI;
    float radius = t * max_radius;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    DEBUG_SERIAL.print("x: ");
    DEBUG_SERIAL.print(rx);
    DEBUG_SERIAL.print(",y: ");
    DEBUG_SERIAL.print(ry);
    if (!drawPlatformPoint(rx, ry)) {
      DEBUG_SERIAL.println("FALSE?");
      success = false;
    }
  }
  resetEncoder(DXL_PLATFORM);
  return success;
}

bool HardwareControl::drawFlower(float radius, float amplitude, int petals, int num_points) {
  bool success = true;

  for (int i = 0; i < num_points; i++) {
    float angle = (2.0f * PI * i) / num_points;
    float r = radius + amplitude * sin(petals * angle);
    float rx = r * cos(angle);
    float ry = r * sin(angle);

    if (!drawPlatformPoint(rx, ry)) {
      success = false;
    }
  }

  return success;
}

bool HardwareControl::executeStreakPattern(uint8_t pattern_id) {
  switch (pattern_id) {
    case 0:  // Simple 3-streak pattern
      return drawLine(-40, 0, 40, 0, 60);

    case 1:  // Spiral streak
      return drawSpiral(20, 2, 50);

    case 2:  // Quadrant streak
      {
        bool success = drawLine(-25, -25, 25, -25, 20);
        success &= drawLine(25, -25, 25, 25, 20);
        success &= drawLine(25, 25, -25, 25, 20);
        success &= drawLine(-25, 25, -25, -25, 20);
        return success;
      }

    case 3:  // Zigzag streak
      {
        float x = -30;
        float y = -30;
        float step = 10;
        bool success = true;

        success &= drawPlatformPoint(x, y);

        for (int i = 0; i < 6; i++) {
          y = (i % 2 == 0) ? 30 : -30;
          x += step;
          success &= drawPlatformPoint(x, y);
        }

        return success;
      }

    default:
      return drawLine(-30, 0, 30, 0, 30);
  }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool HardwareControl::shakeHandler() {
  float pos = dxl.getPresentPosition(DXL_HANDLER);

  for (int i = 0; i < 10; i++) {
    dxl.setGoalPosition(DXL_HANDLER, pos + 50);
    waitForMotors(DXL_HANDLER);
    dxl.setGoalPosition(DXL_HANDLER, pos - 50);
    waitForMotors(DXL_HANDLER);
  }

  dxl.setGoalPosition(DXL_HANDLER, pos);
  waitForMotors(DXL_HANDLER);

  return true;
}



bool HardwareControl::resetEncoder(uint8_t motorId) {
  dxl.torqueOff(motorId);
  dxl.setOperatingMode(motorId, OP_POSITION);
  dxl.setOperatingMode(motorId, 4);
  dxl.torqueOn(motorId);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, motorId, 100);
  return true;
}

uint16_t HardwareControl::getMotorPosition(uint8_t motorId) {
  return dxl.getPresentPosition(motorId);
}

bool HardwareControl::isMotorMoving(uint8_t motorId) {
  return dxl.readControlTableItem(ControlTableItem::MOVING, motorId) != 0;
}

bool HardwareControl::isDishPresent() {
  return true;
}
bool HardwareControl::areMoreDishesAvailable() {
  return true;
}
bool HardwareControl::isSampleCollected() {
  return true;
}