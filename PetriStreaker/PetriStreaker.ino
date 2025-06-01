/**
 * @file PetriStreaker.ino
 * @brief Main Arduino sketch for the Automated Petri Dish Streaker System
 * 
 * This system automates the process of streaking bacterial samples onto
 * petri dishes using coordinated motion control and sample handling.
 * 
 * Hardware components:
 * - Dynamixel motors for polar arm, platform, handler, lid lifter, restacker, and cartridges
 * - Servo motors for gripper control
 * - Solenoid valves and suction pumps
 * - I2C extruder controller
 */

#include <Dynamixel2Arduino.h>
#include "Config.h"
#include "Hardware.h"
#include "StateMachine.h"
#include <Wire.h>

/**
 * @brief Main Arduino setup function
 * 
 * Initializes serial communication, hardware components, and state machine.
 * Called once when the system starts up.
 */
void setup() {
  // Initialize Serial communication
  DEBUG_SERIAL.begin(SERIAL_BAUD_RATE);
  DEBUG_SERIAL.println("Automated Petri Dish Streaker System Starting...");
  
  // Initialize hardware components
  hardware.initialize();
  
  // Initialize and load state machine
  stateMachine.initialize();
  
  DEBUG_SERIAL.println("System initialized and ready.");
}

/**
 * @brief Main Arduino loop function
 * 
 * Continuously updates the state machine and processes serial commands.
 * Runs repeatedly after setup() completes.
 */
void loop() {
  // Update state machine
  stateMachine.update();
  
  // Check for serial commands
  if (DEBUG_SERIAL.available()) {
    String input = DEBUG_SERIAL.readStringUntil('\n');
    input.trim();
    handleSerialCommand(input);
  }
  
  delay(50);  // Small delay to prevent CPU hogging
}

/**
 * @brief Handle serial commands for manual control and testing
 * 
 * @param command String containing the command to execute
 * 
 * Available commands:
 * - status: Display current system status
 * - home: Move all axes to home position
 * - start: Start the streaking cycle
 * - stop: Stop the current cycle
 * - help: Display available commands
 * 
 * New motor test commands:
 * - restackerup/restackerdown: Test restacker movement
 * - cartridge1up/cartridge1down: Test cartridge 1 movement
 * - cartridge2up/cartridge2down: Test cartridge 2 movement  
 * - cartridge3up/cartridge3down: Test cartridge 3 movement
 * - opengrip/closegrip: Test gripper servos
 */
void handleSerialCommand(String command) {
  if (command.equals("status")) {
    DEBUG_SERIAL.print("Current state: ");
    DEBUG_SERIAL.println(stateMachine.getCurrentStateAsString());
  } 
  else if (command.equals("home")) {
    DEBUG_SERIAL.println("Manual home command received");
    hardware.homeAllAxes();
  }
  else if (command.equals("start")) {
    DEBUG_SERIAL.println("Starting cycle");
    stateMachine.startCycle();
  }
  else if (command.equals("stop")) {
    DEBUG_SERIAL.println("Stopping cycle");
    stateMachine.stopCycle();
  }
  
  // ========================================================================
  // NEW MOTOR TEST COMMANDS
  // ========================================================================
  
  // Restacker commands
  else if (command.equals("restackerup")) {
    DEBUG_SERIAL.println("Moving restacker up");
    hardware.moveRestackerUp();
  }
  else if (command.equals("restackerdown")) {
    DEBUG_SERIAL.println("Moving restacker down");
    hardware.moveRestackerDown();
  }
  
  // Cartridge 1 commands
  else if (command.equals("cartridge1up")) {
    DEBUG_SERIAL.println("Moving cartridge 1 up");
    hardware.moveCartridgeUp(1);
  }
  else if (command.equals("cartridge1down")) {
    DEBUG_SERIAL.println("Moving cartridge 1 down");
    hardware.moveCartridgeDown(1);
  }
  
  // Cartridge 2 commands
  else if (command.equals("cartridge2up")) {
    DEBUG_SERIAL.println("Moving cartridge 2 up");
    hardware.moveCartridgeUp(2);
  }
  else if (command.equals("cartridge2down")) {
    DEBUG_SERIAL.println("Moving cartridge 2 down");
    hardware.moveCartridgeDown(2);
  }
  
  // Cartridge 3 commands
  else if (command.equals("cartridge3up")) {
    DEBUG_SERIAL.println("Moving cartridge 3 up");
    hardware.moveCartridgeUp(3);
  }
  else if (command.equals("cartridge3down")) {
    DEBUG_SERIAL.println("Moving cartridge 3 down");
    hardware.moveCartridgeDown(3);
  }
  
  // Gripper commands
  else if (command.equals("opengrip")) {
    DEBUG_SERIAL.println("Opening gripper");
    hardware.openGripper();
  }
  else if (command.equals("closegrip")) {
    DEBUG_SERIAL.println("Closing gripper");
    hardware.closeGripper();
  }
  
  // Help command
  else if (command.equals("help")) {
    printHelp();
  }
  else {
    DEBUG_SERIAL.println("Unknown command. Type 'help' for available commands.");
  }
}

/**
 * @brief Print available serial commands to the console
 */
void printHelp() {
  DEBUG_SERIAL.println("Available commands:");
  DEBUG_SERIAL.println("=== SYSTEM CONTROL ===");
  DEBUG_SERIAL.println("- status       : Display current system status");
  DEBUG_SERIAL.println("- home         : Move all axes to home position");
  DEBUG_SERIAL.println("- start        : Start the streaking cycle");
  DEBUG_SERIAL.println("- stop         : Stop the current cycle");
  DEBUG_SERIAL.println("");
  DEBUG_SERIAL.println("=== MOTOR TESTING ===");
  DEBUG_SERIAL.println("- restackerup  : Move restacker up");
  DEBUG_SERIAL.println("- restackerdown: Move restacker down");
  DEBUG_SERIAL.println("- cartridge1up : Move cartridge 1 up");
  DEBUG_SERIAL.println("- cartridge1down: Move cartridge 1 down");
  DEBUG_SERIAL.println("- cartridge2up : Move cartridge 2 up");
  DEBUG_SERIAL.println("- cartridge2down: Move cartridge 2 down");
  DEBUG_SERIAL.println("- cartridge3up : Move cartridge 3 up");
  DEBUG_SERIAL.println("- cartridge3down: Move cartridge 3 down");
  DEBUG_SERIAL.println("- opengrip     : Open gripper servos");
  DEBUG_SERIAL.println("- closegrip    : Close gripper servos");
  DEBUG_SERIAL.println("- help         : Display this help message");
}