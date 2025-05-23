#include <Dynamixel2Arduino.h>
#include "Config.h"
#include "Hardware.h"
#include "StateMachine.h"
#include <Wire.h>

// Main Arduino setup function
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

// Main Arduino loop function
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

// Handle serial commands for manual control and testing
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
  else if (command.equals("help")) {
    printHelp();
  }
  else if (command.equals("suction on")) {
  DEBUG_SERIAL.println("Turn On Suction");
  hardware.platformSuctionOn();
  hardware.LidSuctionOn();
  }
  else if (command.equals("suction off")) {
  DEBUG_SERIAL.println("Turn Off Suction");
  hardware.platformSuctionOff();
  hardware.LidSuctionOff();
  }
  else if (command.equals("lgl")) {
  DEBUG_SERIAL.println("Lower Lid Gripper");
  hardware.lowerLidLifter();

  }
  else if (command.equals("lgh")) {
  DEBUG_SERIAL.println("Lower Lid Gripper");
  hardware.raiseLidLifter();
  }
  else {
    DEBUG_SERIAL.println("Unknown command. Type 'help' for available commands.");
  }
}

void printHelp() {
  DEBUG_SERIAL.println("Available commands:");
  DEBUG_SERIAL.println("- status      : Display current system status");
  DEBUG_SERIAL.println("- home        : Move all axes to home position");
  DEBUG_SERIAL.println("- start       : Start the streaking cycle");
  DEBUG_SERIAL.println("- stop        : Stop the current cycle");
  DEBUG_SERIAL.println("- help        : Display this help message");
  DEBUG_SERIAL.println("- suction on  : Turn on suction ");
  DEBUG_SERIAL.println("- suction off : Turn off suction");
  DEBUG_SERIAL.println("- lgl: Lower Lid Gripper");
  DEBUG_SERIAL.println("- lgh: Raise Lid Gripper");
}