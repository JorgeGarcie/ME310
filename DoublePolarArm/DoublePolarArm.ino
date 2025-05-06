#include <Dynamixel2Arduino.h>
#include <math.h>

// Hardware setup: OpenRB-150
#define DXL_SERIAL    Serial1
#define DEBUG_SERIAL  Serial
#define DXL_PROTOCOL  2.0
const int DXL_DIR_PIN = -1;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

// IDs
const uint8_t DXL_LEVER = 1;
const uint8_t DXL_PLAT  = 2;

// Geometry & homes (in raw units, 0–4095)
const float R0        = 98.995f;             // lever length [mm]
const float HOME_LEV  = 95.36f/360*4096.0f;  // lever zero offset
const float HOME_PLAT =    0.0f;             // platform zero offset

// Platform center + radius
const float Cx    = 70.0f;
const float Cy    = 70.0f;
const float Rplat = 45.0f;

// Configuration options
bool useElbowUp = true;  // true = use first intersection (elbow up), false = use second intersection (elbow down)

// wrap degrees -> [0,360) -> raw [0..4095]
uint16_t deg2raw(float deg) {
  float d = fmod(deg, 360.0f);
  if (d<0) d += 360.0f;
  return uint16_t((d/360.0f)*4096.0f) & 0x0FFF;
}

void setup() {
  DEBUG_SERIAL.begin(57600);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(DXL_PROTOCOL);

  // Lever motor
  dxl.torqueOff(DXL_LEVER);
  dxl.setOperatingMode(DXL_LEVER, OP_POSITION);
  dxl.torqueOn(DXL_LEVER);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_LEVER, 100);

  // Platform motor
  dxl.torqueOff(DXL_PLAT);
  dxl.setOperatingMode(DXL_PLAT, OP_POSITION);
  dxl.torqueOn(DXL_PLAT);
  dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY, DXL_PLAT, 100);

  // Move both to their home positions
  dxl.setGoalPosition(DXL_LEVER, HOME_LEV);
  dxl.setGoalPosition(DXL_PLAT,  HOME_PLAT);
  delay(500);
  
  DEBUG_SERIAL.println("Initialization complete");
  DEBUG_SERIAL.println("Commands:");
  DEBUG_SERIAL.println("- x,y          : Move to point (e.g., 10,20)");
  DEBUG_SERIAL.println("- h or home    : Return to home position");
  DEBUG_SERIAL.println("- e            : Toggle elbow configuration (up/down)");
  DEBUG_SERIAL.println("- l:x1,y1:x2,y2 : Draw line from (x1,y1) to (x2,y2) (e.g., l:10,0:0,10)");
  DEBUG_SERIAL.println("- c:r          : Draw circle with radius r (e.g., c:30)");
  DEBUG_SERIAL.println("- s:r:t:n      : Draw spiral with radius r, t turns, n points (e.g., s:40:3:50)");
  DEBUG_SERIAL.println("- f:r:a:p:n    : Draw flower with radius r, amplitude a, p petals, n points");
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
 * Using the intersection of lever arc and platform distance
 */
bool drawPlatformPoint(float rx, float ry) {
  // 1. Constrain to platform radius
  float r = sqrt(rx*rx + ry*ry);
  if (r > Rplat) {
    rx = (Rplat/r) * rx;
    ry = (Rplat/r) * ry;
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
  float platform_radius = sqrt(rx*rx + ry*ry);
  
  DEBUG_SERIAL.print("Platform radius: ");
  DEBUG_SERIAL.println(platform_radius);
  
  // Find the intersection of:
  // 1. Circle with radius R0 centered at origin (lever's reach)
  // 2. Circle with radius 'platform_radius' centered at platform center
  
  // Distance between circle centers
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
  
  // Two intersection points - choose based on elbow configuration
  float intersection_x, intersection_y;
  
  if (useElbowUp) {
    // First solution ("elbow up")
    intersection_x = x2 + h * (-Cy) / center_dist;
    intersection_y = y2 + h * Cx / center_dist;
    DEBUG_SERIAL.println("Using elbow UP configuration");
  } else {
    // Second solution ("elbow down")
    intersection_x = x2 - h * (-Cy) / center_dist;
    intersection_y = y2 - h * Cx / center_dist;
    DEBUG_SERIAL.println("Using elbow DOWN configuration");
  }
  
  DEBUG_SERIAL.print("Intersection world coordinates: (");
  DEBUG_SERIAL.print(intersection_x);
  DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(intersection_y);
  DEBUG_SERIAL.println(")");
  
  // Calculate angle of intersection point relative to platform center
  float intersection_angle_from_platform = atan2(intersection_y - Cy, intersection_x - Cx);
  
  // Calculate required platform rotation to bring the target point to the intersection
  float theta2 = intersection_angle_from_platform - original_angle;
  
  // Calculate lever angle to reach the intersection point
  float theta1 = atan2(intersection_y, intersection_x);
  
  // Print angles
  DEBUG_SERIAL.print("Platform angle (theta2) (deg): ");
  DEBUG_SERIAL.println(degrees(theta2));
  DEBUG_SERIAL.print("Lever angle (theta1) (deg): ");
  DEBUG_SERIAL.println(degrees(theta1));
  
  // Convert to motor positions
  float deg1 = fmod(degrees(theta1), 360.0f);
  if (deg1 < 0) deg1 += 360.0f;
  deg1 = deg1 + (HOME_LEV/4096.0f*360.0f);
  
  float deg2 = fmod(degrees(theta2), 360.0f);
  if (deg2 < 0) deg2 += 360.0f;
  deg2 = deg2 + (HOME_PLAT/4096.0f*360.0f);
  
  // Print raw motor positions
  DEBUG_SERIAL.print("Motor 1 position (raw): ");
  DEBUG_SERIAL.println(deg2raw(deg1));
  DEBUG_SERIAL.print("Motor 2 position (raw): ");
  DEBUG_SERIAL.println(deg2raw(deg2));
  
  // Set motor positions
  dxl.setGoalPosition(DXL_LEVER, deg2raw(deg1));
  dxl.setGoalPosition(DXL_PLAT, deg2raw(deg2));
  
  waitForMotors();
  return true;
}

void returnToHome() {
  DEBUG_SERIAL.println("Returning to home position");
  dxl.setGoalPosition(DXL_LEVER, HOME_LEV);
  dxl.setGoalPosition(DXL_PLAT, HOME_PLAT);
  waitForMotors();
  DEBUG_SERIAL.println("At home position");
}

void toggleElbowConfiguration() {
  useElbowUp = !useElbowUp;
  DEBUG_SERIAL.print("Elbow configuration set to: ");
  DEBUG_SERIAL.println(useElbowUp ? "UP" : "DOWN");
}

/**
 * Draw a circle pattern on the platform
 * @param radius - Radius of the circle in mm
 * @param num_points - Number of points to draw
 */
void drawCircle(float radius, int num_points) {
  DEBUG_SERIAL.print("Drawing circle with radius ");
  DEBUG_SERIAL.print(radius);
  DEBUG_SERIAL.print(" using ");
  DEBUG_SERIAL.print(num_points);
  DEBUG_SERIAL.println(" points");
  
  for (int i = 0; i < num_points; i++) {
    float angle = (2.0f * PI * i) / num_points;
    float rx = radius * cos(angle);
    float ry = radius * sin(angle);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    DEBUG_SERIAL.print(": (");
    DEBUG_SERIAL.print(rx);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(ry);
    DEBUG_SERIAL.println(")");
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
  }
  
  DEBUG_SERIAL.println("Circle complete");
}

/**
 * Draw a spiral pattern on the platform
 * @param max_radius - Maximum radius of spiral in mm
 * @param revolutions - Number of revolutions to complete
 * @param num_points - Number of points to draw
 */
void drawSpiral(float max_radius, float revolutions, int num_points) {
  DEBUG_SERIAL.print("Drawing spiral with ");
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
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
  }
  
  DEBUG_SERIAL.println("Spiral complete");
}

/**
 * Draw a flower pattern with petals
 * @param radius - Base radius in mm
 * @param amplitude - Petal size in mm
 * @param petals - Number of petals
 * @param num_points - Number of points to draw
 */
void drawFlower(float radius, float amplitude, int petals, int num_points) {
  DEBUG_SERIAL.print("Drawing flower pattern with ");
  DEBUG_SERIAL.print(petals);
  DEBUG_SERIAL.print(" petals, radius ");
  DEBUG_SERIAL.print(radius);
  DEBUG_SERIAL.print(", amplitude ");
  DEBUG_SERIAL.print(amplitude);
  DEBUG_SERIAL.println(" mm");
  
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
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
  }
  
  DEBUG_SERIAL.println("Flower pattern complete");
}

/**
 * Draw a straight line between two points on the platform
 * @param x1, y1 - Starting point in platform coordinates
 * @param x2, y2 - Ending point in platform coordinates
 * @param num_points - Number of points to draw along the line
 */
void drawLine(float x1, float y1, float x2, float y2, int num_points) {
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
  
  for (int i = 0; i < num_points; i++) {
    float t = (float)i / (num_points - 1);  // Parameter [0,1]
    float rx = x1 + t * (x2 - x1);  // Linear interpolation
    float ry = y1 + t * (y2 - y1);
    
    DEBUG_SERIAL.print("Point ");
    DEBUG_SERIAL.print(i+1);
    DEBUG_SERIAL.print("/");
    DEBUG_SERIAL.print(num_points);
    DEBUG_SERIAL.print(": (");
    DEBUG_SERIAL.print(rx);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(ry);
    DEBUG_SERIAL.println(")");
    
    bool success = drawPlatformPoint(rx, ry);
    if (!success) {
      DEBUG_SERIAL.println("Point was not reachable");
    }
  }
  
  DEBUG_SERIAL.println("Line complete");
}

void processCommand(String command) {
  // Check for home command
  if (command.equals("h") || command.equals("home")) {
    returnToHome();
    return;
  }
  
  // Check for elbow configuration toggle
  if (command.equals("e")) {
    toggleElbowConfiguration();
    return;
  }
  
  // Check for line command (format: l:x1,y1:x2,y2)
  if (command.startsWith("l:")) {
    String params = command.substring(2);
    int firstColon = params.indexOf(':');
    
    if (firstColon > 0) {
      // Parse starting point
      String startPoint = params.substring(0, firstColon);
      int startComma = startPoint.indexOf(',');
      
      // Parse ending point
      String endPoint = params.substring(firstColon + 1);
      int endComma = endPoint.indexOf(',');
      
      if (startComma > 0 && endComma > 0) {
        float x1 = startPoint.substring(0, startComma).toFloat();
        float y1 = startPoint.substring(startComma + 1).toFloat();
        float x2 = endPoint.substring(0, endComma).toFloat();
        float y2 = endPoint.substring(endComma + 1).toFloat();
        
        // Calculate line length to determine number of points
        float distance = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
        int numPoints = max(10, int(distance * 1.5));  // More points for longer lines
        
        drawLine(x1, y1, x2, y2, numPoints);
      } else {
        DEBUG_SERIAL.println("Invalid line format. Use l:x1,y1:x2,y2");
      }
    } else {
      DEBUG_SERIAL.println("Invalid line format. Use l:x1,y1:x2,y2");
    }
    return;
  }
  
  // Check for circle command (format: c:radius)
  if (command.startsWith("c:")) {
    String params = command.substring(2);
    float radius = params.toFloat();
    int num_points = max(36, int(radius * 2));  // More points for larger circles
    
    if (radius > 0 && radius <= Rplat) {
      drawCircle(radius, num_points);
    } else {
      DEBUG_SERIAL.println("Invalid circle radius. Must be between 0 and 45 mm.");
    }
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
  
  // Check for flower command (format: f:radius:amplitude:petals:points)
  if (command.startsWith("f:")) {
    String params = command.substring(2);
    int firstColon = params.indexOf(':');
    int secondColon = params.indexOf(':', firstColon + 1);
    int thirdColon = params.indexOf(':', secondColon + 1);
    
    if (firstColon > 0 && secondColon > 0 && thirdColon > 0) {
      float radius = params.substring(0, firstColon).toFloat();
      float amplitude = params.substring(firstColon + 1, secondColon).toFloat();
      int petals = params.substring(secondColon + 1, thirdColon).toInt();
      int points = params.substring(thirdColon + 1).toInt();
      
      if (radius > 0 && amplitude > 0 && petals > 0 && points > 0) {
        drawFlower(radius, amplitude, petals, points);
      } else {
        DEBUG_SERIAL.println("Invalid flower parameters. Format: f:radius:amplitude:petals:points");
      }
    } else {
      DEBUG_SERIAL.println("Invalid flower command. Format: f:radius:amplitude:petals:points");
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
  DEBUG_SERIAL.println("Invalid command. Enter x,y coordinates or h for home.");
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