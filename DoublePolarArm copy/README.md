# Platform Lever Polar System

## System Overview

This project controls a two-motor system for drawing patterns:

- **Lever Motor (ID 1)**: Controls a 99mm rigid arm that rotates around the origin
- **Platform Motor (ID 2)**: Rotates a 45mm radius platform (petri dish) centered at (70mm, 70mm)
- **Stylus**: Fixed to the end of the lever, making contact with the platform surface

## Kinematics Solution

The core challenge is to calculate the correct motor angles to make the stylus tip touch a specific point on the platform. This is solved using geometric intersection:

### Mathematical Approach

1. **Point Selection**:
   - User specifies a point (rx, ry) in platform-relative coordinates
   - (0,0) is the center of the platform, with +x to the right and +y upward

2. **Geometric Constraint**:
   - The lever traces a circle of radius 99mm around the origin
   - The target point is at a fixed distance from the platform center
   - We find the intersection of these two circles to determine where they can meet

3. **Dual Solutions**:
   - There are typically two possible intersection points (corresponding to "elbow up" and "elbow down" configurations)
   - The code currently uses the first solution by default

4. **Motor Angles**:
   - Lever angle (θ1): Points directly at the intersection point
   - Platform angle (θ2): Rotates to bring the target point to the intersection

### Key Formulas

- **Lever angle**: θ1 = atan2(intersection_y, intersection_x)
- **Platform angle**: θ2 = intersection_angle_from_platform - original_angle
- Where:
  - original_angle = atan2(ry, rx)
  - intersection_angle_from_platform = atan2(intersection_y - Cy, intersection_x - Cx)

## Hardware Configuration

- **OpenRB-150** controller
- **Dynamixel** servo motors (IDs 1 and 2)
- **Geometry**:
  - Lever length (R0): 98.995mm
  - Platform center: (70mm, 70mm)
  - Platform radius: 45mm

## Usage

### Basic Point Testing

Send commands via Serial Monitor (57600 baud):
- **Point coordinates**: Enter as "x,y" (e.g., "10,20" or "-15,5")
- **Home position**: Enter "h" or "home"

### Pattern Generation

The system supports various pattern types:
- **Straight Lines**: Draws a line between two points with smooth interpolation
- **Circles**: Draws a circular pattern with specified radius
- **Spirals**: Creates an outward/inward spiral pattern
- **Flowers**: Creates petal patterns using sinusoidal modulation
- **Custom patterns**: Can be defined as parametric equations

## Tips and Troubleshooting

- **Unreachable Points**: Some points cannot be reached if they require the lever to extend beyond its fixed length
- **Elbow Configuration**: To select between the two possible solutions, uncomment the alternative intersection calculation lines
- **Platform Constraints**: Points are automatically constrained to the platform radius if they exceed it

## Future Improvements

- Explicit control over "elbow up" vs "elbow down" configuration
- Smooth interpolation between points for continuous patterns
- Velocity control for consistent drawing speed