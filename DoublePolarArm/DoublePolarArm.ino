#include <Dynamixel2Arduino.h>
#include <math.h>

// Hardware setup: OpenRB-150
#define DXL_SERIAL    Serial1
#define DEBUG_SERIAL  Serial
#define DXL_PROTOCOL  2.0
const int DXL_DIR_PIN = -1;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

// IDs - Updated to match new config
const uint8_t DXL_LEVER = 2;  // DXL_POLAR_ARM
const uint8_t DXL_PLAT  = 3;  // DXL_PLATFORM

// Geometry & homes - Updated to match new config
const float R0        = 98.995f;                    // POLAR_ARM_LENGTH [mm]
const float HOME_LEV  = (0.51f/360.0f*4096.0f);   // POLAR_ARM_HOME
const float HOME_PLAT = 1238.0f;                   // PLATFORM_HOME

// Platform center + radius
const float Cx    = 70.0f;
const float Cy    = 70.0f;
const float Rplat = 45.0f;

// Store the current motor positions to optimize movement
float current_lever_angle = 0;  // in radians
float current_platform_angle = 0;  // in radians
bool first_move = true;  // Flag for first movement

// Extended position tracking for platform motor
float cumulative_platform_degrees = 0;  // Track total rotation
float last_platform_degrees = 0;        // Previous target in degrees

// Standard deg2raw for lever (no discontinuity issues expected)
uint16_t deg2raw(float deg) {
  float d = fmod(deg, 360.0f);
  if (d<0) d += 360.0f;
  return uint16_t((d/360.0f)*4096.0f) & 0x0FFF;
}

// Extended position for platform motor to avoid discontinuities
int32_t extendedPlatformPosition(float target_degrees) {
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

void setup() {
  DEBUG_SERIAL.begin(115200);  // Updated baud rate from config
  dxl.begin(57600);           // DXL_BAUD_RATE from config
  dxl.setPortProtocolVersion(DXL_PROTOCOL);

  // Lever motor - standard position control
  dxl.torqueOff(DXL_LEVER);
  dxl.setOperatingMode(DXL_LEVER, OP_POSITION);
  dxl.torqueOn(DXL_LEVER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_LEVER, 100);

  // Platform motor - extended position control
  dxl.torqueOff(DXL_PLAT);
  dxl.setOperatingMode(DXL_PLAT, OP_EXTENDED_POSITION);  // Use extended position mode
  dxl.torqueOn(DXL_PLAT);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_PLAT, 100);

  // Move both to their home positions
  dxl.setGoalPosition(DXL_LEVER, HOME_LEV);
  dxl.setGoalPosition(DXL_PLAT, HOME_PLAT);
  
  // Initialize tracking variables - adjust for new home position
  cumulative_platform_degrees = (1238.0f / 4096.0f) * 360.0f;  // Start from actual home position
  last_platform_degrees = cumulative_platform_degrees;
  
  delay(500);
  
  DEBUG_SERIAL.println("Initialization complete - Updated Config");
  DEBUG_SERIAL.print("Polar Arm (ID 2) Home: ");
  DEBUG_SERIAL.println(HOME_LEV);
  DEBUG_SERIAL.print("Platform (ID 3) Home: ");
  DEBUG_SERIAL.println(HOME_PLAT);
  DEBUG_SERIAL.println("Extended Position Mode for Platform");
  DEBUG_SERIAL.println("Commands:");
  DEBUG_SERIAL.println("- t            : Run spiral test (continuous pattern)");
  DEBUG_SERIAL.println("- x,y          : Move to point (e.g., 10,20)");
  DEBUG_SERIAL.println("- h or home    : Return to home position");
  DEBUG_SERIAL.println("- s:r:t:n      : Draw spiral with radius r, t turns, n points");
}

void waitForMotors() {
  uint32_t m1, m2;
  do {
    // Read the MOVING flag (0 = stopped, 1 = still moving)
    m1 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_LEVER);
    m2 = dxl.readControlTableItem(ControlTableItem::MOVING, DXL_PLAT);
    delay(5);
  } while (m1 || m2);
}

/**
 * Draw a point defined in platform-relative coordinates 
 * Using extended position control for platform to avoid discontinuities
 */
bool drawPlatformPoint(float rx, float ry) {
  // 1. Constrain to platform radius
  float r = sqrt(rx*rx + ry*ry);
  if (r > Rplat) {
    rx = (Rplat/r) * rx;
    ry = (Rplat/r) * ry;
    DEBUG_SERIAL.println("Point constrained to platform radius");
  }
  
  // Calculate original point angle in platform coordinates
  float original_angle = atan2(ry, rx);
  float platform_radius = sqrt(rx*rx + ry*ry);
  
  // Find the intersection of lever arc and platform distance
  float center_dist = sqrt(Cx*Cx + Cy*Cy);
  
  // Check if circles can intersect
  if (center_dist > R0 + platform_radius || center_dist < abs(R0 - platform_radius)) {
    DEBUG_SERIAL.println("No intersection possible - point cannot be reached");
    return false;
  }
  
  // Calculate parameters for intersection
  float a = (R0*R0 - platform_radius*platform_radius + center_dist*center_dist) / (2.0f * center_dist);
  float h = sqrt(R0*R0 - a*a);
  
  // Center point along the line between circle centers
  float x2 = Cx * a / center_dist;
  float y2 = Cy * a / center_dist;
  
  // Calculate both possible intersections
  float intersection1_x = x2 + h * (-Cy) / center_dist;
  float intersection1_y = y2 + h * Cx / center_dist;
  
  float intersection2_x = x2 - h * (-Cy) / center_dist;
  float intersection2_y = y2 - h * Cx / center_dist;
  
  // Calculate lever angles for both solutions
  float lever_angle1 = atan2(intersection1_y, intersection1_x);
  float lever_angle2 = atan2(intersection2_y, intersection2_x);
  
  // Calculate intersection angles from platform center
  float intersection1_angle_from_platform = atan2(intersection1_y - Cy, intersection1_x - Cx);
  float intersection2_angle_from_platform = atan2(intersection2_y - Cy, intersection2_x - Cx);
  
  // Calculate platform angles for both solutions
  float platform_angle1 = intersection1_angle_from_platform - original_angle;
  float platform_angle2 = intersection2_angle_from_platform - original_angle;
  
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
    movement1 = abs(lever_angle1 - current_lever_angle) + abs(platform_angle1 - current_platform_angle);
    movement2 = abs(lever_angle2 - current_lever_angle) + abs(platform_angle2 - current_platform_angle);
  }
  
  // Choose the solution with less movement
  if (movement1 <= movement2) {
    theta1 = lever_angle1;
    theta2 = platform_angle1;
  } else {
    theta1 = lever_angle2;
    theta2 = platform_angle2;
  }
  
  // Store current positions for next movement
  current_lever_angle = theta1;
  current_platform_angle = theta2;
  
  // Convert to motor positions
  // Lever: standard position control
  float deg1 = fmod(degrees(theta1), 360.0f);
  if (deg1 < 0) deg1 += 360.0f;
  deg1 = deg1 + (HOME_LEV/4096.0f*360.0f);
  
  // Platform: extended position control to avoid discontinuities
  float deg2 = degrees(theta2) + (HOME_PLAT/4096.0f*360.0f);
  
  // Set motor positions
  dxl.setGoalPosition(DXL_LEVER, deg2raw(deg1));
  dxl.setGoalPosition(DXL_PLAT, extendedPlatformPosition(deg2));  // Use extended position
  
  waitForMotors();
  return true;
}

void returnToHome() {
  DEBUG_SERIAL.println("Returning to home position");
  dxl.setGoalPosition(DXL_LEVER, HOME_LEV);
  dxl.setGoalPosition(DXL_PLAT, HOME_PLAT);
  
  // Reset tracking variables to home position
  cumulative_platform_degrees = (HOME_PLAT / 4096.0f) * 360.0f;
  last_platform_degrees = cumulative_platform_degrees;
  first_move = true;
  
  waitForMotors();
  DEBUG_SERIAL.println("At home position");
}

/**
 * Standalone spiral test to demonstrate continuous platform rotation
 * This test specifically targets the discontinuity problem
 */
void spiralTest() {
  DEBUG_SERIAL.println("=== SPIRAL TEST - Continuous Platform Rotation ===");
  DEBUG_SERIAL.println("This test will draw a spiral that requires continuous platform rotation");
  DEBUG_SERIAL.println("Watch for smooth motion without sudden jumps at 360° boundaries");
  
  float max_radius = 35.0f;     // Stay well within platform bounds
  float revolutions = 4.0f;     // Multiple full rotations to test discontinuity
  int num_points = 120;         // Dense points for smooth motion
  
  DEBUG_SERIAL.print("Parameters: radius=");
  DEBUG_SERIAL.print(max_radius);
  DEBUG_SERIAL.print("mm, revolutions=");
  DEBUG_SERIAL.print(revolutions);
  DEBUG_SERIAL.print(", points=");
  DEBUG_SERIAL.println(num_points);
  
  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);  // normalized parameter [0,1]
    float angle = t * revolutions * 2.0f * PI;
    float radius = t * max_radius;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    
    DEBUG_SERIAL.print("Spiral point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    DEBUG_SERIAL.print(" (");
    DEBUG_SERIAL.print(rx);
    DEBUG_SERIAL.print(",");
    DEBUG_SERIAL.print(ry);
    DEBUG_SERIAL.print(") angle=");
    DEBUG_SERIAL.print(degrees(angle));
    DEBUG_SERIAL.println("°");
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
    
    // Small delay to observe motion
    delay(200);
  }
  
  DEBUG_SERIAL.println("=== SPIRAL TEST COMPLETE ===");
  DEBUG_SERIAL.println("Check if motion was smooth without discontinuities");
}

/**
 * Draw a spiral pattern with extended position control
 */
void drawSpiral(float max_radius, float revolutions, int num_points) {
  DEBUG_SERIAL.print("Drawing spiral with extended position control: ");
  DEBUG_SERIAL.print(revolutions);
  DEBUG_SERIAL.print(" revolutions, max radius ");
  DEBUG_SERIAL.print(max_radius);
  DEBUG_SERIAL.println(" mm");
  
  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);  // normalized parameter [0,1]
    float angle = t * revolutions * 2.0f * PI;
    float radius = t * max_radius;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
  }
  
  DEBUG_SERIAL.println("Spiral complete");
}

void processCommand(String command) {
  // Check for spiral test command
  if (command.equals("t")) {
    spiralTest();
    return;
  }
  
  // Check for home command
  if (command.equals("h") || command.equals("home")) {
    returnToHome();
    return;
  }
  
  // Check for spiral command (format: s:radius:turns:points)
  if (command.startsWith("s:")) {
    String params = command.substring(2);
    int firstColon = params.indexOf(':');
    int secondColon = params.indexOf(':', firstColon + 1);
    
    if (firstColon > 0 && secondColon > 0) {
      float radius = params.substring(0, firstColon).toFloat();
      float turns = params.substring(firstColon + 1, secondColon).toFloat();
      int points = params.substring(secondColon + 1).toInt();
      
      if (radius > 0 && radius <= Rplat && turns > 0 && points > 0) {
        drawSpiral(radius, turns, points);
      } else {
        DEBUG_SERIAL.println("Invalid spiral parameters. Format: s:radius:turns:points");
      }
    } else {
      DEBUG_SERIAL.println("Invalid spiral command. Format: s:radius:turns:points");
    }
    return;
  }
  
  // Try to parse as x,y coordinates
  int commaIndex = command.indexOf(',');
  if (commaIndex > 0) {
    float x = command.substring(0, commaIndex).toFloat();
    float y = command.substring(commaIndex + 1).toFloat();
    
    DEBUG_SERIAL.print("Moving to point: ");
    DEBUG_SERIAL.print(x);
    DEBUG_SERIAL.print(",");
    DEBUG_SERIAL.println(y);
    
    bool success = drawPlatformPoint(x, y);
    
    if (success) {
      DEBUG_SERIAL.println("Point reached successfully");
    } else {
      DEBUG_SERIAL.println("Failed to reach point");
    }
    return;
  }
  
  // If we get here, command was not recognized
  DEBUG_SERIAL.println("Invalid command. Available commands:");
  DEBUG_SERIAL.println("- t: Run spiral test");
  DEBUG_SERIAL.println("- h: Home");
  DEBUG_SERIAL.println("- x,y: Move to point");
  DEBUG_SERIAL.println("- s:r:t:n: Draw spiral");
}

void loop() {
  // Process serial commands
  if (DEBUG_SERIAL.available()) {
    String input = DEBUG_SERIAL.readStringUntil('\n');
    input.trim();
    processCommand(input);
    
    DEBUG_SERIAL.println("Enter next command:");
  }
  
  delay(50);
}