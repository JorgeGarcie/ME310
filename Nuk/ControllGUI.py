import gui as gg
import serial
import tkinter as tk
import time

import MEGAobj
import OPBobj

class AppController:
    def __init__(self,root):
        self.root=root
        self.numberOfPlates=1
        self.petriDishType=None
        self.swabStyle=None

        self.screens = [gg.StartScreen, gg.PetriSelector, gg.SwabSelector, gg.NumberSelector,gg.SummaryScreen,gg.RunningScreen,gg.WaitScreen]
        self.current_index = 0
        self.current_screen = None

        self.isRun=False
        
        self.current_run=0
    
        
        # self.mega=MEGAobj.MegaObj(port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1)
        # self.opd = OPBobj.OPBobj(port='/dev/cu.usbmodem21201', baudrate=115200, timeout=1)


        # self.mega.initCom()



    def show_screen(self, index):
        if self.current_screen:
            self.current_screen.frame.destroy()

        screen_class = self.screens[index]
        self.current_screen = screen_class(self.root,self)
        
    
    def go_back(self):
        if self.current_index >= 0:
            self.current_index -= 1
            self.show_screen(self.current_index)
            self.update()

    def go_forward(self):
        if self.current_index < len(self.screens) - 1:
            self.current_index += 1
            self.show_screen(self.current_index)
            self.update()

    def update(self):
        self.show_screen(self.current_index)
        
    def run_single(self):
        ### the whole process
        print(self.current_run)
        self.current_screen.update_progress(self.current_run)
        self.current_run+=1
        
    def run(self):
        if(self.current_run<self.numberOfPlates):
            self.run_single()
            self.root.after(1000,self.run)
        else:
            self.current_screen.update_progress(self.current_run)
            self.current_screen.enable_done_button()

    def RemoveCart(self):
        #### send function call
        self.current_screen.update_message()



if __name__ == "__main__":
    root = tk.Tk()
    app = AppController(root)
    app.update()
    root.mainloop()