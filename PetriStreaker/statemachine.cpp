#include "StateMachine.h"
#include "Hardware.h"

// Global state machine instance
StateMachine stateMachine;

StateMachine::StateMachine() {
  currentState = CYCLE_PURGE;
  moreDishesInCartridge = true;
  cycleRunning = false;
  stateStartTime = 0;
}

void StateMachine::initialize() {
  DEBUG_SERIAL.println("Initializing state machine...");
  
  // Try to load state from EEPROM
  // For now, just start with PURGE state
  currentState = CYCLE_PURGE;
  stateStartTime = millis();
  
  DEBUG_SERIAL.print("Initial state: ");
  DEBUG_SERIAL.println(getCurrentStateAsString());
}

void StateMachine::update() {
  if (!cycleRunning && currentState != CYCLE_IDLE) {
    // Not running and not in IDLE - go to idle
    transitionToState(CYCLE_IDLE);
    return;
  }
  
  if (!cycleRunning) {
    // In IDLE and not running - just update idle state
    updateIdleState();
    return;
  }
  
  // Process current state
  bool stateComplete = false;
  
  switch (currentState) {
    case CYCLE_PURGE:
      stateComplete = updatePurgeState();
      if (stateComplete) transitionToState(CYCLE_HOME);
      break;
      
    case CYCLE_HOME:
      stateComplete = updateHomeState();
      if (stateComplete) transitionToState(CYCLE_WAIT_CONFIRM);
      break;
      
    case CYCLE_WAIT_CONFIRM:
      stateComplete = updateWaitConfirmState();
      if (stateComplete) transitionToState(CYCLE_LOWER_DISH);
      break;
      
    case CYCLE_LOWER_DISH:
      hardware.rotateHandlerToInitial();
      stateComplete = updateLowerDishState();
      if (stateComplete) {
        if (hardware.areMoreDishesAvailable()) {
          transitionToState(CYCLE_ROTATE_TO_STREAK);
        } else {
          transitionToState(CYCLE_IDLE);
          cycleRunning = false;
        }
      }
      break;
      
    case CYCLE_ROTATE_TO_STREAK:
      hardware.rotateToStreakingStation(); // Handler
      hardware.platformGearUp();
      hardware.platformSuctionOn();
      hardware.lowerLidLifter();
      hardware.LidSuctionOn();
      hardware.raiseLidLifter();
      hardware.movePolarArmToPlatform();

      stateComplete = updateRotateToStreakState();
      if (stateComplete) transitionToState(CYCLE_COLLECT_SAMPLE);
      break;
      
    case CYCLE_COLLECT_SAMPLE:
      hardware.movePolarArmToVial();
      hardware.extrudeFilament(100);
      hardware.retractSample();
      stateComplete = updateCollectSampleState();
      if (stateComplete) transitionToState(CYCLE_EXECUTE_STREAK);
      break;
      
    case CYCLE_EXECUTE_STREAK:
      hardware.extrudeFilament(100);
      hardware.executeStreakPattern(1); // Spiral Streak
      hardware.lowerLidLifter();
      hardware.LidSuctionOff();
      hardware.raiseLidLifter();
      hardware.retractSample();
      hardware.platformSuctionOff();
      hardware.platformGearDown();

      stateComplete = updateExecuteStreakState();
      if (stateComplete) transitionToState(CYCLE_CUT_FILAMENT);
      break;
      
    case CYCLE_CUT_FILAMENT:
      hardware.movePolarArmToVial(); // Will be changed to rotate to cutter instead
      hardware.extrudeSample();
      hardware.cutFilament();
      hardware.movePolarArmToPlatform(); // Maybe will change to platform plus offset to move away from lid lifter

      stateComplete = updateCutFilamentState();
      if (stateComplete) transitionToState(CYCLE_RESTACK_DISH);
      break;
      
    case CYCLE_RESTACK_DISH:
      hardware.rotateHandlerToInitial();
      hardware.solenoidLift();
      hardware.solenoidDown();
      hardware.rotateHandlerToInitial();

      stateComplete = updateRestackDishState();
      if (stateComplete) {
        moreDishesInCartridge = hardware.areMoreDishesAvailable();
        if (moreDishesInCartridge) {
          transitionToState(CYCLE_LOWER_DISH);
        } else {
          transitionToState(CYCLE_IDLE);
          cycleRunning = false;
        }
      }
      break;
      
    case CYCLE_IDLE:
      updateIdleState();
      break;
  }
}

void StateMachine::transitionToState(GlobalState newState) {
  DEBUG_SERIAL.print("Transitioning from ");
  DEBUG_SERIAL.print(getCurrentStateAsString());
  DEBUG_SERIAL.print(" to ");
  
  currentState = newState;
  stateStartTime = millis();
  
  DEBUG_SERIAL.println(getCurrentStateAsString());
  
  // Save state to EEPROM
  // saveStateToEEPROM(currentState);
}

void StateMachine::startCycle() {
  DEBUG_SERIAL.println("Starting cycle");
  cycleRunning = true;
  
  if (currentState == CYCLE_IDLE) {
    transitionToState(CYCLE_PURGE);
  }
}

void StateMachine::stopCycle() {
  DEBUG_SERIAL.println("Stopping cycle");
  cycleRunning = false;
}

void StateMachine::emergencyStop() {
  DEBUG_SERIAL.println("EMERGENCY STOP");
  cycleRunning = false;
  // Additional emergency actions
}

GlobalState StateMachine::getCurrentState() const {
  return currentState;
}

const char* StateMachine::getCurrentStateAsString() const {
  switch (currentState) {
    case CYCLE_PURGE: return "PURGE";
    case CYCLE_HOME: return "HOME";
    case CYCLE_WAIT_CONFIRM: return "WAIT_CONFIRM";
    case CYCLE_LOWER_DISH: return "LOWER_DISH";
    case CYCLE_ROTATE_TO_STREAK: return "ROTATE_TO_STREAK";
    case CYCLE_COLLECT_SAMPLE: return "COLLECT_SAMPLE";
    case CYCLE_EXECUTE_STREAK: return "EXECUTE_STREAK";
    case CYCLE_CUT_FILAMENT: return "CUT_FILAMENT";
    case CYCLE_RESTACK_DISH: return "RESTACK_DISH";
    case CYCLE_IDLE: return "IDLE";
    default: return "UNKNOWN";
  }
}

bool StateMachine::isCycleRunning() const {
  return cycleRunning;
}

unsigned long StateMachine::getTimeInCurrentState() const {
  return millis() - stateStartTime;
}

// State update function implementations
bool StateMachine::updatePurgeState() {
  static enum {INIT, RUNNING, COMPLETE} substate = INIT;
  
  switch (substate) {
    case INIT:
      DEBUG_SERIAL.println("Starting purge operation");
      // Initialize purge operation
      substate = RUNNING;
      return false;
      
    case RUNNING:
      // Execute purge operation
      if (hardware.doPurgeOperation() || getTimeInCurrentState() > PURGE_TIMEOUT) {
        substate = COMPLETE;
      }
      return false;
      
    case COMPLETE:
      DEBUG_SERIAL.println("Purge complete");
      substate = INIT;  // Reset for next time
      return true;
  }
  
  return false;
}

bool StateMachine::updateHomeState() {
  static enum {INIT, HOMING, COMPLETE} substate = INIT;
  
  switch (substate) {
    case INIT:
      DEBUG_SERIAL.println("Starting homing sequence");
      // Initialize homing sequence
      substate = HOMING;
      return false;
      
    case HOMING:
      // Execute homing sequence
      if (hardware.homePosition() || getTimeInCurrentState() > HOME_TIMEOUT) {
        substate = COMPLETE;
      }
      return false;
      
    case COMPLETE:
      DEBUG_SERIAL.println("Homing complete");
      substate = INIT;  // Reset for next time
      return true;
  }
  
  return false;
}

bool StateMachine::updateWaitConfirmState() {
  // Check for button press or serial command
  if (digitalRead(CONFIRM_BUTTON_PIN) == HIGH || getTimeInCurrentState() > 100) {
    // Auto-confirm for testing
    DEBUG_SERIAL.println("Confirmation received");
    return true;
  }
  return false;
}

bool StateMachine::updateLowerDishState() {
  static enum {INIT, OPEN_FIRST, CLAMP_FIRST, OPEN_SECOND, CLOSE_SECOND, CHECK_POSITION, COMPLETE} substate = INIT;
  
  switch (substate) {
    case INIT:
      DEBUG_SERIAL.println("Starting dish lowering");
      substate = OPEN_FIRST;
      return false;
      
    case OPEN_FIRST:
      if (hardware.openFirstFingers()) {
        DEBUG_SERIAL.println("First fingers opened");
        substate = CLAMP_FIRST;
      }
      return false;
      
    case CLAMP_FIRST:
      if (hardware.clampFirstFingers()) {
        DEBUG_SERIAL.println("First fingers clamped");
        substate = OPEN_SECOND;
      }
      return false;
      
    case OPEN_SECOND:
      if (hardware.openSecondFingers()) {
        DEBUG_SERIAL.println("Second fingers opened");
        substate = CLOSE_SECOND;
      }
      return false;
      
    case CLOSE_SECOND:
      if (hardware.closeSecondFingers()) {
        DEBUG_SERIAL.println("Second fingers closed");
        substate = CHECK_POSITION;
      }
      return false;
      
    case CHECK_POSITION:
      if (hardware.isDishPresent()) {
        DEBUG_SERIAL.println("Dish position verified");
        substate = COMPLETE;
      }
      return false;
      
    case COMPLETE:
      DEBUG_SERIAL.println("Dish lowering complete");
      substate = INIT;  // Reset for next time
      return true;
  }
  
  return false;
}

// Stub implementations for remaining state functions
bool StateMachine::updateRotateToStreakState() { return true; }
bool StateMachine::updateCollectSampleState() { return true; }
bool StateMachine::updateExecuteStreakState() { return true; }
bool StateMachine::updateCutFilamentState() { return true; }
bool StateMachine::updateRestackDishState() { return true; }
void StateMachine::updateIdleState() {}