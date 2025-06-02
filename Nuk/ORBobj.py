import serial
import tkinter as tk
import time


class ORBobj:
    def __init__(self, port='/dev/cu.usbmodem21201', baudrate=115200, timeout=1):
        self.com = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)

    def write(self, data):
        data = data + "\n"  # Ensure the data ends with a newline
        self.com.write(data.encode())

    def read(self):
        return self.com.readline().decode().strip()

    def initCom(self):
        pass

    def moveArm(self, position):
        self.write(f"MOVE {position}")

    def lift(self, id, dir):
        self.write(f"LIFT {id}")

    def grab(self, id):
        self.write(f"GRAB {id}")
    
    def release(self, id):
        self.write(f"RELEASE {id}")
    
    def sucction(self, state):
        self.write(f"SUCTION {state}")

    def lid(self, state):
        self.write(f"LID {state}")
    
    def fetch(self):
        self.write("FETCH")

    def cut(self):
        self.write("CUT")
    
    def extrude(self):
        self.write("EXTRUDE")
    
    def swab(self, id):
        self.write(f"SWAB {id}")
    
    def getDish(self, position):
        self.write(f"GET {position}")

    def liftAll(self,dir):
        self.write(f"LIFT ALL {dir}")
        