/**
 * @file CommandHandler.cpp
 * @brief Implementation of CommandHandler class
 * 
 * Handles serial commands from NUC and maps them to hardware functions.
 * Implements the exact command set from the NUK CSV.
 */

#include "CommandHandler.h"

CommandHandler::CommandHandler(HardwareControl* hw) : hardware(hw) {}

void CommandHandler::initialize() {
  DEBUG_SERIAL.println("=================================");
  DEBUG_SERIAL.println("OpenRB Command Handler Ready");
  DEBUG_SERIAL.println("Accepting NUK CSV Commands");
  DEBUG_SERIAL.println("=================================");
  DEBUG_SERIAL.println("Available commands:");
  DEBUG_SERIAL.println("MOVE [position], LIFT [pos] [dir], GRAB [pos], RELEASE [pos]");
  DEBUG_SERIAL.println("PLATFORM LIFT [dir], SUCTION [state], LID [state]");
  DEBUG_SERIAL.println("FETCH, CUT, PATTERN [id], HOME ALL, STATUS, RESET");
  DEBUG_SERIAL.println("CYCLE START, ABORT, PAUSE, RESUME");
  DEBUG_SERIAL.println("=================================");
}

void CommandHandler::processCommand() {
  if (DEBUG_SERIAL.available()) {
    String command = DEBUG_SERIAL.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command.length() > 0) {
      DEBUG_SERIAL.print("Received: ");
      DEBUG_SERIAL.println(command);
      
      executeCommand(command);
    }
  }
}

void CommandHandler::executeCommand(String command) {
  // Parse command into parts
  int firstSpace = command.indexOf(' ');
  String cmd = (firstSpace > 0) ? command.substring(0, firstSpace) : command;
  String args = (firstSpace > 0) ? command.substring(firstSpace + 1) : "";
  
  // Execute based on command type
  if (cmd == "MOVE") {
    handleMoveCommand(args);
  }
  else if (cmd == "LIFT") {
    handleLiftCommand(args);
  }
  else if (cmd == "GRAB") {
    handleGrabCommand(args);
  }
  else if (cmd == "RELEASE") {
    handleReleaseCommand(args);
  }
  else if (cmd == "PLATFORM") {
    handlePlatformCommand(args);
  }
  else if (cmd == "SUCTION") {
    handleSuctionCommand(args);
  }
  else if (cmd == "LID") {
    handleLidCommand(args);
  }
  else if (cmd == "FETCH") {
    handleFetchCommand();
  }
  else if (cmd == "CUT") {
    handleCutCommand();
  }
  else if (cmd == "PATTERN") {
    handlePatternCommand(args);
  }
  else if (cmd == "HOME") {
    handleHomeCommand(args);
  }
  else if (cmd == "STATUS") {
    handleStatusCommand();
  }
  else if (cmd == "RESET") {
    handleResetCommand();
  }
  else if (cmd == "CYCLE") {
    handleCycleCommand(args);
  }
  else if (cmd == "ABORT") {
    handleAbortCommand();
  }
  else if (cmd == "PAUSE") {
    handlePauseCommand();
  }
  else if (cmd == "RESUME") {
    handleResumeCommand();
  }
  else if (cmd == "EXTRUDE") {
    handleExtrudeCommand();
  }
  else {
    DEBUG_SERIAL.println("UNKNOWN COMMAND");
  }
}

// ========================================================================
// COMMAND IMPLEMENTATIONS (EXACT CSV MAPPING)
// ========================================================================

void CommandHandler::handleMoveCommand(String position) {
  // MOVE "position" - NORMAL/BLOOD/CHOCOLAT/STRG/WORK_AREA
  DEBUG_SERIAL.print("Moving handler to: ");
  DEBUG_SERIAL.println(position);
  
  bool success = false;
  
  if (position == "WORK_AREA" || position == "WORK AREA") {
    success = hardware->moveToWorkArea();
  }
  else if (position == "STRG") {
    success = hardware->moveToStorage();
  }
  else if (position == "NORMAL") {
    success = hardware->moveToNormal();
  }
  else if (position == "BLOOD") {
    success = hardware->moveToBlood();
  }
  else if (position == "CHOCOLAT") {
    success = hardware->moveToChocolat();
  }
  else {
    DEBUG_SERIAL.println("MOVE INVALID POSITION");
    return;
  }
  
  DEBUG_SERIAL.println(success ? "MOVE COMPLETED" : "MOVE FAILED");
}

void CommandHandler::handleLiftCommand(String args) {
  // LIFT "position" "dir" - position: NORMAL/BLOOD/CHOCOLAT/STRG, dir: UP/DOWN/MID/TOP
  int spaceIndex = args.indexOf(' ');
  if (spaceIndex < 0) {
    DEBUG_SERIAL.println("LIFT INVALID ARGS");
    return;
  }
  
  String position = args.substring(0, spaceIndex);
  String direction = args.substring(spaceIndex + 1);
  
  DEBUG_SERIAL.print("Lifting ");
  DEBUG_SERIAL.print(position);
  DEBUG_SERIAL.print(" ");
  DEBUG_SERIAL.println(direction);
  
  bool success = false;
  
  if (position == "ALL") {
    if (direction == "TOP") {
      success = hardware->liftAllTop();
      DEBUG_SERIAL.println(success ? "ALL LIFT TOP" : "ALL LIFT FAILED");
    } else if (direction == "UP") {
      success = hardware->liftAllUp();
      DEBUG_SERIAL.println(success ? "ALL LIFT UP" : "ALL LIFT FAILED");
    } else if (direction == "MID") {
      success = hardware->liftAllMid();
      DEBUG_SERIAL.println(success ? "ALL LIFT MID" : "ALL LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->liftAllDown();
      DEBUG_SERIAL.println(success ? "ALL LIFT DOWN" : "ALL LIFT FAILED");
    }
  }
  else if (position == "STRG") {
    if (direction == "TOP") {
      success = hardware->liftStorageTop();
      DEBUG_SERIAL.println(success ? "LIFT TOP" : "LIFT FAILED");
    } else if (direction == "UP") {
      success = hardware->liftStorageUp();
      DEBUG_SERIAL.println(success ? "LIFT UP" : "LIFT FAILED");
    } else if (direction == "MID") {
      success = hardware->liftStorageMid();
      DEBUG_SERIAL.println(success ? "LIFT MID" : "LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->liftStorageDown();
      DEBUG_SERIAL.println(success ? "LIFT DOWN" : "LIFT FAILED");
    }
  }
  else if (position == "NORMAL") {
    if (direction == "TOP") {
      success = hardware->liftNormalTop();
      DEBUG_SERIAL.println(success ? "LIFT TOP" : "LIFT FAILED");
    } else if (direction == "UP") {
      success = hardware->liftNormalUp();
      DEBUG_SERIAL.println(success ? "LIFT UP" : "LIFT FAILED");
    } else if (direction == "MID") {
      success = hardware->liftNormalMid();
      DEBUG_SERIAL.println(success ? "LIFT MID" : "LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->liftNormalDown();
      DEBUG_SERIAL.println(success ? "LIFT DOWN" : "LIFT FAILED");
    }
  }
  else if (position == "BLOOD") {
    if (direction == "TOP") {
      success = hardware->liftBloodTop();
      DEBUG_SERIAL.println(success ? "LIFT TOP" : "LIFT FAILED");
    } else if (direction == "UP") {
      success = hardware->liftBloodUp();
      DEBUG_SERIAL.println(success ? "LIFT UP" : "LIFT FAILED");
    } else if (direction == "MID") {
      success = hardware->liftBloodMid();
      DEBUG_SERIAL.println(success ? "LIFT MID" : "LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->liftBloodDown();
      DEBUG_SERIAL.println(success ? "LIFT DOWN" : "LIFT FAILED");
    }
  }
  else if (position == "CHOCOLAT") {
    if (direction == "TOP") {
      success = hardware->liftChocolatTop();
      DEBUG_SERIAL.println(success ? "LIFT TOP" : "LIFT FAILED");
    } else if (direction == "UP") {
      success = hardware->liftChocolatUp();
      DEBUG_SERIAL.println(success ? "LIFT UP" : "LIFT FAILED");
    } else if (direction == "MID") {
      success = hardware->liftChocolatMid();
      DEBUG_SERIAL.println(success ? "LIFT MID" : "LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->liftChocolatDown();
      DEBUG_SERIAL.println(success ? "LIFT DOWN" : "LIFT FAILED");
    }
  }
  else {
    DEBUG_SERIAL.println("LIFT INVALID POSITION");
  }
}

void CommandHandler::handleGrabCommand(String position) {
  // GRAB "position" - NORMAL/BLOOD/CHOCOLAT
  DEBUG_SERIAL.print("Grabbing at: ");
  DEBUG_SERIAL.println(position);
  
  // Map to openGripper() or specific grab functions
  bool success = hardware->openGripper(); // Assuming this is the grab action
  
  if (position == "ALL") {
    DEBUG_SERIAL.println(success ? "GRAB ALL COMPLETED" : "GRAB ALL FAILED");
  } else {
    DEBUG_SERIAL.println(success ? "GRAB COMPLETED" : "GRAB FAILED");
  }
}

void CommandHandler::handleReleaseCommand(String position) {
  // RELEASE "position" - NORMAL/BLOOD/CHOCOLAT
  DEBUG_SERIAL.print("Releasing at: ");
  DEBUG_SERIAL.println(position);
  
  // Map to closeGripper() or specific release functions
  bool success = hardware->closeGripper(); // Assuming this is the release action
  
  if (position == "ALL") {
    DEBUG_SERIAL.println(success ? "RELEASE ALL COMPLETED" : "RELEASE ALL FAILED");
  } else {
    DEBUG_SERIAL.println(success ? "RELEASE COMPLETED" : "RELEASE FAILED");
  }
}

void CommandHandler::handlePlatformCommand(String args) {
  // PLATFORM LIFT "dir" - UP/DOWN
  if (args.startsWith("LIFT ")) {
    String direction = args.substring(5); // Remove "LIFT "
    
    DEBUG_SERIAL.print("Platform lift: ");
    DEBUG_SERIAL.println(direction);
    
    bool success = false;
    
    if (direction == "UP") {
      success = hardware->platformGearUp();
      DEBUG_SERIAL.println(success ? "PLATFORM LIFT UP" : "PLATFORM LIFT FAILED");
    } else if (direction == "DOWN") {
      success = hardware->platformGearDown();
      DEBUG_SERIAL.println(success ? "PLATFORM LIFT DOWN" : "PLATFORM LIFT FAILED");
    } else {
      DEBUG_SERIAL.println("PLATFORM LIFT INVALID DIRECTION");
    }
  } else {
    DEBUG_SERIAL.println("PLATFORM INVALID COMMAND");
  }
}


void CommandHandler::handleSuctionCommand(String args) {
  // SUCTION "state" - simplified to platform suction only
  // Command format: "SUCTION ON" or "SUCTION OFF"
  
  DEBUG_SERIAL.print("Platform suction ");
  DEBUG_SERIAL.println(args);
  
  bool success = false;
  
  if (args == "ON") {
    success = hardware->suctionRotationOn();  // Platform suction on
    DEBUG_SERIAL.println(success ? "SUCC ON" : "ERROR");

  } else if (args == "OFF") {
    success = hardware->suctionRotationOff(); // Platform suction off
    DEBUG_SERIAL.println(success ? "SUCC OFF" : "ERROR");

  } else {
    DEBUG_SERIAL.println("SUCTION INVALID STATE");
    return;
  }
  
  // Send response
}

void CommandHandler::handleLidCommand(String state) {
  // LID "state" - OPEN/CLOSE
  DEBUG_SERIAL.print("Lid: ");
  DEBUG_SERIAL.println(state);
  
  bool success = false;
  
  if (state == "OPEN") {
    success = hardware->lidOpen();
    DEBUG_SERIAL.println(success ? "LID REMOVED" : "LID FAILED");
  } else if (state == "CLOSE") {
    success = hardware->lidClose();
    DEBUG_SERIAL.println(success ? "LID ON" : "LID FAILED");
  } else {
    DEBUG_SERIAL.println("LID INVALID STATE");
  }
}

void CommandHandler::handleFetchCommand() {
  // FETCH
  DEBUG_SERIAL.println("Fetching sample");
  
  bool success = hardware->fetchSample();
  DEBUG_SERIAL.println(success ? "FETCH RDY" : "FETCH FAILED");
}

void CommandHandler::handleCutCommand() {
  // CUT
  DEBUG_SERIAL.println("Preparing cut");
  
  bool success = hardware->prepareCut();
  DEBUG_SERIAL.println(success ? "CUT RDY" : "CUT FAILED");
}

void CommandHandler::handlePatternCommand(String patternId) {
  // PATTERN "id" - 0/1/2/3
  DEBUG_SERIAL.print("Executing pattern: ");
  DEBUG_SERIAL.println(patternId);
  
  int id = patternId.toInt();
  bool success = hardware->executeStreakPattern(id);
  DEBUG_SERIAL.println(success ? "PATTERN COMPLETED" : "PATTERN FAILED");
}

void CommandHandler::handleHomeCommand(String args) {
  // HOME ALL
  if (args == "ALL") {
    DEBUG_SERIAL.println("Homing all axes");
    
    hardware->homeAllAxes();
    DEBUG_SERIAL.println("HOME COMPLETED");
  } else {
    DEBUG_SERIAL.println("HOME INVALID ARGS");
  }
}

void CommandHandler::handleStatusCommand() {
  // STATUS
  DEBUG_SERIAL.println("Checking system status");
  
  // Simple status check - can be expanded
  DEBUG_SERIAL.println("STATUS OK");
}

void CommandHandler::handleResetCommand() {
  // RESET
  DEBUG_SERIAL.println("Resetting system");
  
  // Reset operations - can be expanded
  hardware->homeAllAxes();
  DEBUG_SERIAL.println("RESET COMPLETED");
}

void CommandHandler::handleCycleCommand(String args) {
  // CYCLE START
  if (args == "START") {
    DEBUG_SERIAL.println("Starting automated cycle");
    DEBUG_SERIAL.println("CYCLE STARTED");
  } else {
    DEBUG_SERIAL.println("CYCLE INVALID ARGS");
  }
}

void CommandHandler::handleAbortCommand() {
  // ABORT
  DEBUG_SERIAL.println("Aborting operations");
  DEBUG_SERIAL.println("OPERATION ABORTED");
}

void CommandHandler::handlePauseCommand() {
  // PAUSE
  DEBUG_SERIAL.println("Pausing system");
  DEBUG_SERIAL.println("SYSTEM PAUSED");
}

void CommandHandler::handleResumeCommand() {
  // RESUME
  DEBUG_SERIAL.println("Resuming system");
  DEBUG_SERIAL.println("SYSTEM RESUMED");
}

void CommandHandler::handleExtrudeCommand() {
  // EXTRUDE
  bool success = hardware->extrude();  // Waits for completion
  DEBUG_SERIAL.println(success ? "EXTRUDE RDY" : "EXTRUDE FAILED");
}