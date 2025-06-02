/**
 * @file main.ino
 * @brief OpenRB Petri Dish Streaker - Command Interface Mode
 * 
 * Refactored to work as slave controller responding to NUC commands.
 * State machine logic moved to NUC Python code.
 * 
 * Hardware: OpenRB-150 + Dynamixel motors + pneumatics
 * Communication: Serial commands from NUC
 */

#include "Hardware.h"
#include "CommandHandler.h"

// Global instances
HardwareControl hardware;
CommandHandler commandHandler(&hardware);

void setup() {
  // Initialize serial communication
  DEBUG_SERIAL.begin(SERIAL_BAUD_RATE);
  while (!DEBUG_SERIAL) {
    ; // Wait for serial port to connect
  }
  
  DEBUG_SERIAL.println("========================================");
  DEBUG_SERIAL.println("OpenRB Petri Dish Streaker v2.0");
  DEBUG_SERIAL.println("Command Interface Mode");
  DEBUG_SERIAL.println("State Machine: NUC Python");
  DEBUG_SERIAL.println("Hardware Control: OpenRB");
  DEBUG_SERIAL.println("========================================");
  
  // Initialize hardware (Dynamixel motors + pneumatics)
  hardware.initialize();
  
  // Initialize command handler (CSV command interface)
  commandHandler.initialize();
  
  DEBUG_SERIAL.println("System ready!");
  DEBUG_SERIAL.println("Awaiting commands from NUC or Serial Monitor...");
  DEBUG_SERIAL.println();
}

void loop() {
  // Process incoming serial commands from NUC (or serial monitor)
  commandHandler.processCommand();
  
  // Small delay to prevent overwhelming the serial port
  delay(10);
}

/*
========================================
USAGE EXAMPLES
========================================

Serial Monitor Testing:
- MOVE WORK_AREA
- LIFT STRG UP
- GRAB NORMAL
- PLATFORM LIFT UP
- SUCTION ROT ON
- LID OPEN
- PATTERN 0
- HOME ALL
- STATUS

NUC Python Integration:
import serial
ser = serial.Serial('/dev/ttyUSB0', 115200)
ser.write(b'MOVE WORK_AREA\n')
response = ser.readline().decode().strip()
print(response)  # Should print "MOVE COMPLETED"

Complete Workflow Example (NUC Python):
1. ser.write(b'HOME ALL\n')         # Initialize
2. ser.write(b'MOVE WORK_AREA\n')   # Position handler
3. ser.write(b'PLATFORM LIFT UP\n') # Engage platform
4. ser.write(b'LID OPEN\n')         # Remove lid
5. ser.write(b'FETCH\n')            # Get sample
6. ser.write(b'PATTERN 0\n')        # Execute streak
7. ser.write(b'LID CLOSE\n')        # Replace lid

========================================
FILE STRUCTURE
========================================
OpenRB/
├── main.ino           ← This file (Arduino main)
├── Hardware.cpp       ← Motor control functions  
├── Hardware.h         ← Hardware class definition
├── CommandHandler.cpp ← Command interface implementation
├── CommandHandler.h   ← Command interface definition
└── Config.h          ← Pin definitions and constants

NUC/
├── state_machine.py   ← Python state machine (replaces StateMachine.cpp)
├── openrb_interface.py ← Serial communication with OpenRB
└── main_controller.py  ← Main NUC controller

========================================
*/