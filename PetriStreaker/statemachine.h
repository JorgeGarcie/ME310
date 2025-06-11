#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "Config.h"

// Global state enum definition
enum GlobalState {
  CYCLE_PURGE,
  CYCLE_HOME,
  CYCLE_WAIT_CONFIRM,
  CYCLE_LOWER_DISH,
  CYCLE_ROTATE_TO_STREAK,
  CYCLE_COLLECT_SAMPLE,
  CYCLE_EXECUTE_STREAK,
  CYCLE_CUT_FILAMENT,
  CYCLE_RESTACK_DISH,
  CYCLE_IDLE
};

class StateMachine {
  private:
    GlobalState currentState;
    bool moreDishesInCartridge;
    bool cycleRunning;
    unsigned long stateStartTime;
    
    // State update functions
    bool updatePurgeState();
    bool updateHomeState();
    bool updateWaitConfirmState();
    bool updateLowerDishState();
    bool updateRotateToStreakState();
    bool updateCollectSampleState();
    bool updateExecuteStreakState();
    bool updateCutFilamentState();
    bool updateRestackDishState();
    void updateIdleState();
    
    // Internal helpers
    void transitionToState(GlobalState newState);
    
  public:
    StateMachine();
    
    // Main functions
    void initialize();
    void update();
    
    // Control functions
    void startCycle();
    void stopCycle();
    void emergencyStop();
    
    // Status functions
    GlobalState getCurrentState() const;
    const char* getCurrentStateAsString() const;
    bool isCycleRunning() const;
    unsigned long getTimeInCurrentState() const;
};

// Global state machine instance
extern StateMachine stateMachine;

#endif // STATE_MACHINE_H