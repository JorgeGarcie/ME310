import serial
import tkinter as tk
import time



class ORB2:
    def __init__(self, port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1):
        self.com = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)

    def write(self, data):
        data = data + "\n"  # Ensure the data ends with a newline
        self.com.write(data.encode())

    def read(self):
        return self.com.readline().decode().strip()

    def GRAB(self,TYPE):
        self.write(f"OPEN {TYPE}")

    def RELEAS(self,TYPE):
        self.write(f"RELEASE {TYPE}")
