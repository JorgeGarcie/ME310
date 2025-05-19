#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Serial configuration
#define DEBUG_SERIAL  Serial
#define DXL_SERIAL    Serial1
#define SERIAL_BAUD_RATE 115200
#define DXL_BAUD_RATE    57600
#define DXL_PROTOCOL 2.0

// Motor IDs
#define DXL_POLAR_ARM    2  // Polar arm / lever motor ID
#define DXL_PLATFORM     3  // Platform motor ID
#define DXL_HANDLER      4  // Handler for dish stacking motor ID
#define DXL_EXTRUDER     5  // Extruder for sample dispensing motor ID

// Hardware pin definitions
#define DXL_DIR_PIN     -1  // Direction pin for Dynamixel communication
#define CONFIRM_BUTTON_PIN  8  // Pin for confirmation button
#define STATUS_LED_PIN     13  // Pin for status LED

// Cartridge parameters
#define MAX_DISHES        10  // Maximum number of dishes in cartridge

// Geometry & homes (in raw units, 0–4095)
#define POLAR_ARM_LENGTH   98.995f  // Polar arm length [mm]
#define POLAR_ARM_HOME    (56.25f/360.0f*4096.0f)  // Polar arm home position
#define PLATFORM_HOME     600.0f  // Platform home position to not obstruct
#define HANDLER_HOME     1705.0f  // Platform home position

// Movement speeds
#define POLAR_ARM_SPEED   100  // Polar arm movement speed
#define PLATFORM_SPEED    100  // Platform movement speed
#define HANDLER_SPEED      100  // Handler movement speed
#define EXTRUDER_SPEED    50   // Extruder movement speed

// Operation timeouts (ms)
#define PURGE_TIMEOUT    2000  // Maximum time for purge operation
#define HOME_TIMEOUT     5000  // Maximum time for homing operation
#define DISH_LOAD_TIMEOUT 3000 // Maximum time for dish loading

// Predetermined Positions in Units for the Cartridges
#define HOMING_HANDLER 1705
#define HANDLER2PLATFORM 3753
#define POLAR_ARM_HOME_NO_OBSTRUCTION 640 + 300

// Determine The Pins for Solenoid Valves and Diaphrams
#define LID_SUCTION 5
#define LID_SOLENOID 6
#define PLATFORM_SUCTION 7
#define PLATFORM_SOLENOID 4

// EEPROM addresses
//#define EEPROM_STATE_ADDRESS  0  // Address to store current state

#endif // CONFIG_H