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
#define DXL_LID_LIFTER   1  // Lid Lifer / lever motor ID
#define DXL_POLAR_ARM    2  // Polar arm / lever motor ID
#define DXL_PLATFORM     3  // Platform motor ID NOT USE FOR NOW
#define DXL_HANDLER      4  // Handler for dish stacking motor ID
#define DXL_RESTACKER    5  // Handler for dish stacking motor ID
#define DXL_CARTRIDGE1   8  // Handler for dish stacking motor ID
#define DXL_CARTRIDGE2   6  // Handler for dish stacking motor ID
#define DXL_CARTRIDGE3   7  // Handler for dish stacking motor ID

// Hardware pin definitions
#define DXL_DIR_PIN     -1  // Direction pin for Dynamixel communication
#define CONFIRM_BUTTON_PIN  8  // Pin for confirmation button
#define STATUS_LED_PIN     13  // Pin for status LED

// Cartridge parameters
#define MAX_DISHES        10  // Maximum number of dishes in cartridge

// Geometry & homes (in raw units, 0–4095)
#define LID_LIFTER_HOME   3849.0f
#define POLAR_ARM_LENGTH   98.995f  // Polar arm length [mm]
//#define POLAR_ARM_HOME    (56.25f/360.0f*4096.0f)  // DO NOT MODIFY USEFUL FOR CALCULATIONS - DO NOT GO THERE
//#define POLAR_ARM_HOME    (178.51f/360.0f*4096.0f)  // DO NOT MODIFY USEFUL FOR CALCULATIONS - DO NOT GO THERE
#define POLAR_ARM_HOME    (0.51f/360.0f*4096.0f)  // DO NOT MODIFY USEFUL FOR CALCULATIONS - DO NOT GO THERE
#define POLAR_ARM_NO_OBSTRUCT_HOME (236.25f/360.0f*4096.0f) // ACTUAL POSITION TO RES      T
#define PLATFORM_HOME     1238.0f  // Platform home position to not obstruct 
#define HANDLER_HOME      1947.0f  // Platform home position means that it goes to cartridge. Middle is 1705 units 2112. New is 1540 so 407 units
#define RESTACKER_HOME    2700.0f  // Restacker position down
#define CARTRIDGE1_HOME   1320.0f  // C3 position down
#define CARTRIDGE2_HOME   0.0f     // C1 position down
#define CARTRIDGE3_HOME   2650.0f  // C2 position down


// Predetermined Positions in Units for System
#define STREAKING_STATION 3585.0f // Handler
#define HANDLER_RESTACKER 4416.0f 
#define HANDLER_C1 2762.0f 
#define HANDLER_C2 1947.0f 
#define HANDLER_C3 1123.0f 
#define POLAR_ARM_TO_VIAL 2042.0f
#define POLAR_ARM_TO_CUT 2590.0f
#define POLAR_ARM_SWABBING   621.0f

#define RESTACKER_UP    3207.0f
#define PLATFORM_UP     790.0f
#define LID_LIFTER_DOWN 3000.0f  // Drop the lid down

#define RESTACKER_TOP    6000.0f  // Restacker position down
#define CARTRIDGE1_TOP   4095.0f  // C3 position down
#define CARTRIDGE2_TOP   1976.0f     // C1 position down
#define CARTRIDGE3_TOP   6000.0f  // C2 position down

#define RESTACKER_UP    4900.0f  // Restacker position down
#define CARTRIDGE1_UP   4095.0f  // C3 position down
#define CARTRIDGE2_UP   1976.0f     // C1 position down
#define CARTRIDGE3_UP   4095.0f  // C2 position down

#define RESTACKER_MID    4300.0f  // Restacker position down
#define CARTRIDGE1_MID   4095.0f  // C3 position down
#define CARTRIDGE2_MID   1976.0f     // C1 position down
#define CARTRIDGE3_MID   4095.0f  // C2 position down

/* 
POSITIONS 

STORAGE CARTRIDGE (EX POS) ID 5
top: 6000, up: 4900, mid: 4300, down: 2700. HANDLER 300

CHOCOLAT C3 CARTRIDGE (EX POS) ID 7
top: 6000, up: 4900, mid: 4300, down: 2700.  HANDLER 1120

BLOOD C2 CARTRIDGE (EX POS) ID 6
top: 3450, up: 2350, mid: 1750, down: 0 OR 150.  HANDLER 1950

NORMAL C1 CARTRIDGE (EX POS) ID 8
top: 4840, up: 3640, mid: 3090, down: 1340.  HANDLER 2762

*/
// Motion Profiles
#define LID_LIFTER_SPEED    50   // Extruder movement speed
#define POLAR_ARM_SPEED     100  // Polar arm movement speed
#define PLATFORM_SPEED      100  // Platform movement speed
#define HANDLER_SPEED       100  // Handler movement speed
#define HANDLER_ACCEL       20   // Handler Accel
#define RESTACKER_SPEED    100  // Restacker position down
#define CARTRIDGE1_SPEED   100     // C1 position down
#define CARTRIDGE2_SPEED   100  // C2 position down
#define CARTRIDGE3_SPEED   100  // C3 position down

// Operation timeouts (ms)
#define PURGE_TIMEOUT    2000  // Maximum time for purge operation
#define HOME_TIMEOUT     5000  // Maximum time for homing operation
#define DISH_LOAD_TIMEOUT 3000 // Maximum time for dish loading

// Determine The Pins for Solenoid Valves and Diaphrams
//#define LID_SUCTION 5
//#define LID_SOLENOID 6
//#define PLATFORM_SUCTION 7
//#define PLATFORM_SOLENOID 4
#define LID_SUCTION 4
#define LID_SOLENOID 6
#define PLATFORM_SUCTION 5
#define PLATFORM_SOLENOID 7

// EEPROM addresses
//#define EEPROM_STATE_ADDRESS  0  // Address to store current state

#endif // CONFIG_H