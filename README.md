# InoQ - Automated Petri Dish Streaking System

## Table of Contents
- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Hardware Setup](#hardware-setup)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)

---

## Overview
This system automates the biological sample streaking process using three coordinated microcontrollers:

- **ORB1 (OpenRB-150)**: Primary motion and pneumatic system control 
- **ORB2 (OpenRB-150)**: Gripper ops for cartridge
- **MEGA (Arduino Mega 2560)**: Additional operations

---

## Hardware Requirements

### Power Supplies
- 19V power adapter for Intel NUC host computer
- 12V DC power supply for actuators and motors
- Both power supplies must be connected before system startup

### Microcontrollers
- 2x ROBOTIS OpenRB-150 controllers
- 1x Arduino Mega 2560
- 3x USB cables for serial communication

### Electronic Components
- Intel NUC or compatible computer
- 5" capacitive touchscreen (HDMI + USB)
- Dynamixel servo motors (8 units)
- Stepper motors with DRV8825 drivers
- HX711 load cell amplifier
- Pneumatic system components


## Software Requirements

### Host Computer (Python)
- Python 3.9 or compatible
- Operating System: Windows/Linux/macOS

#### Required Python Libraries
```bash
pip install pyserial
```
*Note: `tkinter` is included with most Python installations*

### Arduino Development Environment
- Arduino IDE (any recent version)
- OpenRB-150 Board Package: Download from ROBOTIS eManual

#### Required Arduino Libraries (via Library Manager):
- Dynamixel2Arduino
- AccelStepper
- HX711

Built-in: `Wire`, `Servo`

---

## Installation

### 1. Clone Repository
```bash
git clone [repository-url]
cd inoq-petri-streaker
```

### 2. Python Setup
```bash
pip install pyserial
python --version  # Should show 3.9.x
```

### 3. Arduino IDE Setup
- Install Arduino IDE
- Add OpenRB-150 board package:
  - Go to **File → Preferences**
  - Add this to *Additional Board Manager URLs*:
    ```
    https://raw.githubusercontent.com/ROBOTIS-GIT/OpenRB-150/master/package_openrb_index.json
    ```
  - Go to **Tools → Board → Board Manager**
  - Search and install **OpenRB-150**

- Install libraries:
  - **Tools → Manage Libraries**
  - Search and install: Dynamixel2Arduino, AccelStepper, HX711

### 4. Flash Firmware
Upload the following files to respective controllers:
- **ORB1 (COM11)**: `commandHandler.ino`
- **ORB2 (COM15)**: `gripper_control.ino`
- **MEGA (COM12)**: `Extrude_LoadCell_fullyIntegared.ino`

NOTE: THESE COMS WERE ASSIGNED WHEN WE FLASHED THE FIRMWARE INTO THE MCUs USING THE NUC. IT IS IMPORTANT 
TO CHANGE THE COMS INSIDE THE PYTHON SCRIPT IF THEY CHANGE DURING FLASHING.

---

## Hardware Setup

### Power Connection Sequence
1. Connect 12V DC supply to barrel jack (motors/actuators)
2. Connect 19V adapter to Intel NUC
3. Connect USB cables:
   - ORB1 → COM11
   - ORB2 → COM15
   - MEGA → COM12

### Verification
- All three controllers should appear in Device Manager/System
- Power LEDs should be active
- Touchscreen should display desktop

---

## Usage

### Startup Sequence
1. Power on all supplies (12V + 19V)
2. Start host computer and wait for desktop
3. Run application:
```bash
python ControllGUI.py
```
4. System initializes automatically
5. Follow GUI prompts

### Operating Workflow
- Select dish type: **Macconkey**, **Blood**, **Chocolate agar**
- Choose streaking pattern: **Line**, **Quadrant**, **Spiral**
- Set batch quantity
- Confirm and start
- Monitor progress in GUI
- Handle cartridge changes as prompted

### Cartridge 
- Follow on-screen instructions

---

## Troubleshooting

### Communication Issues
- Check USB connections
- Verify COM ports: COM11, COM12, COM15
- Restart application if needed

### Hardware Problems
- Check power (both 12V and 19V must be active)
- Verify motor connections
- Use emergency stop if needed (GUI)

### Common Errors
- **"Timeout waiting for response"** → Check serial connections
- **"Motor position error"** → Check power and wiring
