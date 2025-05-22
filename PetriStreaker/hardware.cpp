#include "Hardware.h"
#include <math.h>

// Global hardware instance
HardwareControl hardware;

HardwareControl::HardwareControl() : dxl(DXL_SERIAL, DXL_DIR_PIN) {
  current_polar_angle = 0.0f;
  current_platform_angle = 0.0f;
  is_initialized = false;
  first_move = true;
  
  // Platform geometry settings
  platform_center_x = 70.0f;  // Cx
  platform_center_y = 70.0f;  // Cy
  platform_radius = 45.0f;    // Rplat
}

void HardwareControl::initialize() {
  DEBUG_SERIAL.println("Initializing hardware...");
  
  // Initialize Dynamixel
  dxl.begin(DXL_BAUD_RATE);
  dxl.setPortProtocolVersion(2.0); // Set Protocol version
  
  // Initialize polar arm motor
  dxl.torqueOff(DXL_LID_LIFTER);
  dxl.setOperatingMode(DXL_LID_LIFTER, OP_POSITION);
  dxl.torqueOn(DXL_LID_LIFTER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_LID_LIFTER, LID_LIFTER_SPEED);

  // Initialize polar arm motor
  dxl.torqueOff(DXL_POLAR_ARM);
  dxl.setOperatingMode(DXL_POLAR_ARM, OP_POSITION);
  dxl.torqueOn(DXL_POLAR_ARM);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_POLAR_ARM, POLAR_ARM_SPEED);
  
  // Initialize platform motor
  dxl.torqueOff(DXL_PLATFORM);
  dxl.setOperatingMode(DXL_PLATFORM, OP_POSITION);
  dxl.torqueOn(DXL_PLATFORM);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_PLATFORM, PLATFORM_SPEED);

  // Initialize handler motor
  dxl.torqueOff(DXL_HANDLER);
  dxl.setOperatingMode(DXL_PLATFORM, OP_EXTENDED_POSITION); // USE IT TO RESET IT
  dxl.torqueOn(DXL_HANDLER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_HANDLER, HANDLER_SPEED);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_ACCELERATION, DXL_HANDLER, HANDLER_ACCEL);

  // Initialize (Off) Solenoid Valves and Pumps
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

void HardwareControl::homeAllAxes() {
  DEBUG_SERIAL.println("Homing all axes...");
  
  // Move motors to home positions
  dxl.setGoalPosition(DXL_PLATFORM, (uint32_t)PLATFORM_HOME);
  waitForMotors(DXL_PLATFORM); // Wait for Platform to Lower First 
  dxl.setGoalPosition(DXL_LID_LIFTER, (uint32_t)LID_LIFTER_HOME);
  waitForMotors(DXL_LID_LIFTER); // Wait for Platform to Lower First 
  dxl.setGoalPosition(DXL_POLAR_ARM, (uint32_t)POLAR_ARM_NO_OBSTRUCT_HOME);
  dxl.setGoalPosition(DXL_HANDLER, (uint32_t)HANDLER_HOME);
  
  // Wait for motors to reach position
  waitForMotors();
  
  // Reset current positions
  current_polar_angle = 0.0f;
  current_platform_angle = 0.0f;
  first_move = true;
  
  DEBUG_SERIAL.println("All axes homed");
}

void HardwareControl::waitForMotors(uint8_t motorId) {
  uint32_t m1, m2, m3, m4;
  
  // Initial delay to allow motor controller to update status registers
  delay(50);
  
  // Initial check to see if motors are moving
  if (motorId > 0) {
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    m2 = 0;
    m3 = 0;
    m4 = 0;
  } else {
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
    m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
    m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
    m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
  }
  
  // If no motors appear to be moving initially, wait a bit more and check again
  if (m1 == 0 && m2 == 0 && m3 == 0  && m4 == 0) {
    delay(100);
    
    // Double-check
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);

    }
    
    // If still no movement detected, assume the motors completed their movement quickly
    if (m1 == 0 && m2 == 0 && m3 == 0  && m4 == 0) {
      return;
    }
  }
  
  // Main waiting loop
  do {
    if (motorId > 0) {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, motorId);
      m2 = 0;
      m3 = 0;
      m4 = 0;
    } else {
      m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_POLAR_ARM);
      m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLATFORM);
      m3 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_HANDLER);
      m4 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LID_LIFTER);
    }
    delay(5);  // Small delay between checks
  } while (m1 == 1 || m2 == 1 || m3 == 1 || m4 == 1);
}

uint16_t HardwareControl::degToRaw(float degrees) {
  float d = fmod(degrees, 360.0f);
  if (d < 0) d += 360.0f;
  return uint16_t((d/360.0f)*4096.0f) & 0x0FFF;
}

float HardwareControl::rawToDeg(uint16_t raw) {
  return (raw & 0x0FFF) * 360.0f / 4096.0f;
}

bool HardwareControl::homePosition() {

  
    // Move motors to home positions
  dxl.setGoalPosition(DXL_PLATFORM, (uint32_t)PLATFORM_HOME);
  waitForMotors(DXL_PLATFORM); // Wait for Platform to Lower First 
  dxl.setGoalPosition(DXL_LID_LIFTER, (uint32_t)LID_LIFTER_HOME);
  waitForMotors(DXL_LID_LIFTER); // Wait for Platform to Lower First 
  dxl.setGoalPosition(DXL_POLAR_ARM, (uint32_t)POLAR_ARM_NO_OBSTRUCT_HOME);
  dxl.setGoalPosition(DXL_HANDLER, (uint32_t)HANDLER_HOME);
  // Wait for motors to complete
  waitForMotors();
  
  return true;
}

bool HardwareControl::drawPlatformPoint(float rx, float ry) {
  // 1. Constrain to platform radius
  float r = sqrt(rx*rx + ry*ry);
  if (r > platform_radius) {
    rx = (platform_radius/r) * rx;
    ry = (platform_radius/r) * ry;
    DEBUG_SERIAL.println("Point constrained to platform radius");
  }
  
  // Print platform-relative coordinates
  DEBUG_SERIAL.print("Platform coordinates: (");
  DEBUG_SERIAL.print(rx);
  DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(ry);
  DEBUG_SERIAL.println(")");
  
  // Calculate original point angle in platform coordinates
  float original_angle = atan2(ry, rx);
  float platform_radius_point = sqrt(rx*rx + ry*ry);
  
  DEBUG_SERIAL.print("Platform radius: ");
  DEBUG_SERIAL.println(platform_radius_point);
  
  // Find the intersection of:
  // 1. Circle with radius POLAR_ARM_LENGTH centered at origin (lever's reach)
  // 2. Circle with radius 'platform_radius_point' centered at platform center
  
  // Distance between circle centers
  float center_dist = sqrt(platform_center_x*platform_center_x + platform_center_y*platform_center_y);
  
  // Check if circles can intersect
  if (center_dist > POLAR_ARM_LENGTH + platform_radius_point || 
      center_dist < abs(POLAR_ARM_LENGTH - platform_radius_point)) {
    DEBUG_SERIAL.println("No intersection possible - point cannot be reached");
    return false;
  }
  
  // Calculate parameters for intersection
  float a = (POLAR_ARM_LENGTH*POLAR_ARM_LENGTH - platform_radius_point*platform_radius_point + center_dist*center_dist) / (2.0f * center_dist);
  float h = sqrt(POLAR_ARM_LENGTH*POLAR_ARM_LENGTH - a*a);
  
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
  
  // Calculate intersection angles from platform center
  float intersection1_angle_from_platform = atan2(intersection1_y - platform_center_y, 
                                                 intersection1_x - platform_center_x);
  float intersection2_angle_from_platform = atan2(intersection2_y - platform_center_y, 
                                                 intersection2_x - platform_center_x);
  
  // Calculate platform angles for both solutions
  float platform_angle1 = intersection1_angle_from_platform - original_angle;
  float platform_angle2 = intersection2_angle_from_platform - original_angle;
  
  // Normalize platform angles to [-π, π]
  while (platform_angle1 > PI) platform_angle1 -= 2*PI;
  while (platform_angle1 < -PI) platform_angle1 += 2*PI;
  while (platform_angle2 > PI) platform_angle2 -= 2*PI;
  while (platform_angle2 < -PI) platform_angle2 += 2*PI;
  
  // Choose the solution with the minimum movement from current position
  float theta1, theta2;
  float movement1, movement2;
  
  if (first_move) {
    // For first move, prefer the solution with platform angle closer to 0
    movement1 = abs(platform_angle1);
    movement2 = abs(platform_angle2);
    first_move = false;
  } else {
    // Calculate total angular distance for both solutions
    movement1 = abs(lever_angle1 - current_polar_angle) + abs(platform_angle1 - current_platform_angle);
    movement2 = abs(lever_angle2 - current_polar_angle) + abs(platform_angle2 - current_platform_angle);
  }
  
  // Choose the solution with less movement
  if (movement1 <= movement2) {
    theta1 = lever_angle1;
    theta2 = platform_angle1;
    DEBUG_SERIAL.println("Using first intersection (less movement)");
    DEBUG_SERIAL.print("Movement cost: ");
    DEBUG_SERIAL.println(movement1);
    
    DEBUG_SERIAL.print("World coordinates: (");
    DEBUG_SERIAL.print(intersection1_x);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(intersection1_y);
    DEBUG_SERIAL.println(")");
  } else {
    theta1 = lever_angle2;
    theta2 = platform_angle2;
    DEBUG_SERIAL.println("Using second intersection (less movement)");
    DEBUG_SERIAL.print("Movement cost: ");
    DEBUG_SERIAL.println(movement2);
    
    DEBUG_SERIAL.print("World coordinates: (");
    DEBUG_SERIAL.print(intersection2_x);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(intersection2_y);
    DEBUG_SERIAL.println(")");
  }
  
  // Store current positions for next movement
  current_polar_angle = theta1;
  current_platform_angle = theta2;
  
  // Print angles
  DEBUG_SERIAL.print("Platform angle (theta2) (deg): ");
  DEBUG_SERIAL.println(degrees(theta2));
  DEBUG_SERIAL.print("Lever angle (theta1) (deg): ");
  DEBUG_SERIAL.println(degrees(theta1));
  
  // Convert to motor positions
  float deg1 = fmod(degrees(theta1), 360.0f);
  if (deg1 < 0) deg1 += 360.0f;
  deg1 = deg1 + (POLAR_ARM_HOME/4096.0f*360.0f);
  
  float deg2 = fmod(degrees(theta2), 360.0f);
  if (deg2 < 0) deg2 += 360.0f;
  deg2 = deg2 + (PLATFORM_HOME/4096.0f*360.0f);
  
  // Set motor positions
  dxl.setGoalPosition(DXL_POLAR_ARM, degToRaw(deg1));
  dxl.setGoalPosition(DXL_PLATFORM, degToRaw(deg2));
  
  waitForMotors();
  return true;
}


bool HardwareControl::moveToCoordinate(float x, float y) {
  return drawPlatformPoint(x, y);
}

bool HardwareControl::drawLine(float x1, float y1, float x2, float y2, int num_points) {
  DEBUG_SERIAL.print("Drawing line from (");
  DEBUG_SERIAL.print(x1);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print(y1);
  DEBUG_SERIAL.print(") to (");
  DEBUG_SERIAL.print(x2);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print(y2);
  DEBUG_SERIAL.println(")");
  
  // Need at least 2 points
  num_points = max(2, num_points);
  
  bool success = true;
  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);  // Parameter [0,1]
    float rx = x1 + t * (x2 - x1);  // Linear interpolation
    float ry = y1 + t * (y2 - y1);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    
    bool point_success = drawPlatformPoint(rx, ry);
    if (!point_success) {
      DEBUG_SERIAL.println("Point was not reachable");
      success = false;
    }
  }
  
  DEBUG_SERIAL.println("Line complete");
  return success;
}

bool HardwareControl::drawCircle(float radius, int num_points) {
  DEBUG_SERIAL.print("Drawing circle with radius ");
  DEBUG_SERIAL.print(radius);
  DEBUG_SERIAL.print(" using ");
  DEBUG_SERIAL.print(num_points);
  DEBUG_SERIAL.println(" points");
  
  bool success = true;
  for (int i = 0; i < num_points; i++) {
    float angle = (2.0f * PI * i) / num_points;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    
    bool point_success = drawPlatformPoint(rx, ry);
    if (!point_success) {
      DEBUG_SERIAL.println("Point was not reachable");
      success = false;
    }
  }
  
  DEBUG_SERIAL.println("Circle complete");
  return success;
}

bool HardwareControl::drawSpiral(float max_radius, float revolutions, int num_points) {
  DEBUG_SERIAL.print("Drawing spiral with ");
  DEBUG_SERIAL.print(revolutions);
  DEBUG_SERIAL.print(" revolutions, max radius ");
  DEBUG_SERIAL.print(max_radius);
  DEBUG_SERIAL.println(" mm");
  
  bool success = true;
  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);  // normalized parameter [0,1]
    float angle = t * revolutions * 2.0f * PI;
    float radius = t * max_radius;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    
    bool point_success = drawPlatformPoint(rx, ry);
    if (!point_success) {
      DEBUG_SERIAL.println("Point was not reachable");
      success = false;
    }
  }
  
  DEBUG_SERIAL.println("Spiral complete");
  return success;
}

bool HardwareControl::drawFlower(float radius, float amplitude, int petals, int num_points) {
  DEBUG_SERIAL.print("Drawing flower pattern with ");
  DEBUG_SERIAL.print(petals);
  DEBUG_SERIAL.print(" petals, radius ");
  DEBUG_SERIAL.print(radius);
  DEBUG_SERIAL.print(", amplitude ");
  DEBUG_SERIAL.print(amplitude);
  DEBUG_SERIAL.println(" mm");
  
  bool success = true;
  for (int i = 0; i < num_points; i++) {
    float angle = (2.0f * PI * i) / num_points;
    // Flower equation: r = base_radius + amplitude * sin(petals * angle)
    float r = radius + amplitude * sin(petals * angle);
    float rx = r * cos(angle);
    float ry = r * sin(angle);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    
    bool point_success = drawPlatformPoint(rx, ry);
    if (!point_success) {
      DEBUG_SERIAL.println("Point was not reachable");
      success = false;
    }
  }
  
  DEBUG_SERIAL.println("Flower pattern complete");
  return success;
}

bool HardwareControl::executeStreakPattern(uint8_t pattern_id) {
  // Implement different streaking patterns based on pattern_id
  switch (pattern_id) {
    case 0:  // Simple 3-streak pattern
      return drawLine(-40, 0, -10, 0, 10);
      
    case 1:  // Spiral streak
      return drawSpiral(30, 2, 50);
      
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
        
        // Move to starting position
        success &= drawPlatformPoint(x, y);
        
        // Draw zigzag
        for (int i = 0; i < 6; i++) {
          y = (i % 2 == 0) ? 30 : -30;
          x += step;
          success &= drawPlatformPoint(x, y);
        }
        
        return success;
      }
      
    default:
      // Default to simple streak
      return drawLine(-30, 0, 30, 0, 30);
  }
}

// Placeholder implementations for hardware control functions
bool HardwareControl::doPurgeOperation() { return true; }
bool HardwareControl::openFirstFingers() { return true; }
bool HardwareControl::clampFirstFingers() { return true; }
bool HardwareControl::openSecondFingers() { return true; }
bool HardwareControl::closeSecondFingers() { return true; }

bool HardwareControl::movePolarArmToVial() {
  dxl.setGoalPosition(DXL_POLAR_ARM, degToRaw(280.9)); // 290.9 for now but needs refinement
  waitForMotors();
  return true; 
}

bool HardwareControl::movePolarArmToPlatform(){
  dxl.setGoalPosition(DXL_POLAR_ARM, POLAR_ARM_NO_OBSTRUCT_HOME); // Change but can work for now
  waitForMotors();
  return true; 
}


bool HardwareControl::extrudeSample() { return true; }
bool HardwareControl::retractSample() { return true; }

bool HardwareControl::rotateToStreakingStation() { 
  dxl.setGoalPosition(DXL_HANDLER, STREAKING_STATION);
  waitForMotors();

  return true; }

bool HardwareControl::rotateHandlerToInitial() { 
  dxl.setGoalPosition(DXL_HANDLER, HANDLER_HOME);
  waitForMotors();

  return true; }

bool HardwareControl::rotateHandlerToFinished() { 
  dxl.setGoalPosition(DXL_HANDLER, HANDLER_RESTACKER);
  waitForMotors();

  return true; }

bool HardwareControl::resetEncoder(uint8_t motorId) { 

  dxl.torqueOff(motorId);
  dxl.setOperatingMode(motorId, OP_POSITION); // USE IT TO RESET IT
  dxl.setOperatingMode(motorId, 4); 
  dxl.torqueOn(motorId);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_HANDLER, HANDLER_SPEED);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_ACCELERATION, DXL_HANDLER, HANDLER_ACCEL);

  return true; }

bool HardwareControl::platformGearUp() { 
  dxl.setGoalPosition(DXL_PLATFORM, PLATFORM_UP);
  waitForMotors();
  return true; 
}

bool HardwareControl::shakeHandler() {
  // Get current position
  float pos = dxl.getPresentPosition(DXL_HANDLER);
  
  // Shake 10 times
  for (int i = 0; i < 10; i++) {
    // Move handler to positive offset
    dxl.setGoalPosition(DXL_HANDLER, pos + 50);
    waitForMotors(DXL_HANDLER);

    // Move handler to negative offset
    dxl.setGoalPosition(DXL_HANDLER, pos - 50);
    waitForMotors(DXL_HANDLER);
  }
  
  // Return to original position
  dxl.setGoalPosition(DXL_HANDLER, pos);
  waitForMotors(DXL_HANDLER);
  
  return true;
}

bool HardwareControl::platformGearDown() {
  dxl.setGoalPosition(DXL_PLATFORM, PLATFORM_HOME); // Home and Down are down.
  waitForMotors();  
  return true; 
}

bool HardwareControl::platformSuctionOn() { 
  digitalWrite(PLATFORM_SUCTION, HIGH);  
  return true; 
}

bool HardwareControl::platformSuctionOff() { 
  digitalWrite(PLATFORM_SOLENOID, HIGH); // Release Pressure
  digitalWrite(PLATFORM_SUCTION, LOW);  // Turn Motor Off
  delay(10);
  digitalWrite(PLATFORM_SOLENOID, LOW); // Turn back solenoid Off
  return true; 
}

bool HardwareControl::LidSuctionOn(){
  digitalWrite(LID_SUCTION, HIGH); 
  return true; 
}

bool HardwareControl::LidSuctionOff(){
  digitalWrite(LID_SOLENOID, HIGH); // Release Pressure
  digitalWrite(LID_SUCTION, LOW);  // Turn Motor Off
  delay(10);
  digitalWrite(LID_SOLENOID, LOW); // Turn back solenoid Off
  return true; 
}

bool HardwareControl::lowerLidLifter() { 
  dxl.setGoalPosition(DXL_LID_LIFTER, LID_LIFTER_DOWN); // Home and Down are down.
  waitForMotors();  
  return true; 
}

bool HardwareControl::lowerLidLifterNoContact() { 
  dxl.setGoalPosition(DXL_LID_LIFTER, LID_LIFTER_DOWN + 150); // Home and Down are down.
  waitForMotors();  
  return true; 
}

bool HardwareControl::raiseLidLifter() { 
  dxl.setGoalPosition(DXL_LID_LIFTER, LID_LIFTER_HOME); // Home and Down are down.
  waitForMotors();  
  return true; 
}
bool HardwareControl::cutFilament() { return true; }
bool HardwareControl::extrudeFilament(float amount) { return true; }
bool HardwareControl::solenoidLift() { return true; }
bool HardwareControl::solenoidDown() { return true; }

void HardwareControl::setMotorSpeed(uint8_t motorId, uint32_t speed) {
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, motorId, speed);
}

uint16_t HardwareControl::getMotorPosition(uint8_t motorId) {
  return dxl.getPresentPosition(motorId);
}

bool HardwareControl::isMotorMoving(uint8_t motorId) {
  return dxl.readControlTableItem(ControlTableItem::MOVING, motorId) != 0;
}

bool HardwareControl::isDishPresent() { return true; }
bool HardwareControl::areMoreDishesAvailable() { return true; }
bool HardwareControl::isSampleCollected() { return true; }