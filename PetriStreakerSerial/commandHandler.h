/**
 * @file CommandHandler.h
 * @brief Command interface for OpenRB - handles NUK CSV commands
 * 
 * This class handles serial commands from NUC and maps them to 
 * hardware functions. Implements the exact command set from CSV.
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Hardware.h"

class CommandHandler {
private:
  HardwareControl* hardware;
  
  // Command parsing and execution
  void executeCommand(String command);
  
  // Individual command handlers (CSV mapping)
  void handleMoveCommand(String position);
  void handleLiftCommand(String args);
  void handleGrabCommand(String position);
  void handleReleaseCommand(String position);
  void handlePlatformCommand(String args);
  void handleSuctionCommand(String args);
  void handleLidCommand(String state);
  void handleFetchCommand();
  void handleCutCommand();
  void handlePatternCommand(String patternId);
  void handleHomeCommand(String args);
  void handleStatusCommand();
  void handleResetCommand();
  void handleCycleCommand(String args);
  void handleAbortCommand();
  void handlePauseCommand();
  void handleResumeCommand();
  
public:
  CommandHandler(HardwareControl* hw);
  
  void initialize();
  void processCommand();
};

#endif // COMMAND_HANDLER_H