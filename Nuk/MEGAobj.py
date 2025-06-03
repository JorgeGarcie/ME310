import serial
import tkinter as tk
import time


class MegaObj:
    def __init__(self, port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1):
        self.com = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)

    def write(self, data):
        data = data + "\n"  # Ensure the data ends with a newline
        self.com.write(data.encode())

    def read(self):
        return self.com.readline().decode().strip()

    def extrude(self):
        self.write(f"EXTRUDE")

    def prepCut(self):
        self.write(f"PREP CUT")

    def cut(self):
        self.write(f"CUT")

    def fetch(self):
        self.write(f"FETCH")
    
    def Nai(self, dir):
        self.write(f"PLATFORM LIFT {dir}")

    def initCom(self,timeout=5):
        start_time = time.time()
        while True:
            if self.read() == "Bob":
                self.write("Hi Bob")
                print("MegaObj initialized successfully.")
                return
            if time.time() - start_time > timeout:
                raise TimeoutError(f"Timeout waiting for init comunication")