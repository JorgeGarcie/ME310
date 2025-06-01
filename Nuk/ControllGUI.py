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

        self.screens = [gg.StartScreen, gg.PetriSelector, gg.SwabSelector3, gg.NumberSelector,gg.SummaryScreen,gg.RunningScreen,gg.WaitScreen]
        self.current_index = 0
        self.current_screen = None

        self.isRun=False
        
        self.current_run=0
    
        
        self.mega=MEGAobj.MegaObj(port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1)
        self.opd = OPBobj.OPBobj(port='/dev/cu.usbmodem21201', baudrate=115200, timeout=1)
        self.mega.initCom()



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
        self.opd.moveArm(self.petriDishType)
        self.wait_for_confirmation(self.opd,"MOVE COMPLETED")

        self.opd.getDish(self.petriDishType)
        self.wait_for_confirmation(self.opd,"DISH RDY")

        self.opd.moveArm("WORK AREA")
        self.wait_for_confirmation(self.opd,"MOVE COMPLETED")

        self.mega.Nai("UP")
        self.wait_for_confirmation(self.mega,"NAI UP")

        self.opd.sucction("ON")
        self.wait_for_confirmation(self.opd,"SUCC ON")

        self.opd.lid("OPEN")
        self.wait_for_confirmation(self.opd,"LID REMOVED")


        self.opd.fetch()
        self.wait_for_confirmation(self.opd,"FETCH RDY")

        self.mega.fetch()
        self.wait_for_confirmation(self.mega, "FETCH START")
        self.wait_for_confirmation(self.mega, "FETCH COMPLETED")


        self.opd.extrude()
        self.wait_for_confirmation(self.opd,"EXTRUDE RDY")

        self.mega.extrude()
        self.wait_for_confirmation(self.mega, "EXTRUDE START")
        self.wait_for_confirmation(self.mega, "EXTRUDE COMPLETED","EXTRUDE FAILED")


        self.opd.swab(self.petriDishType)
        self.wait_for_confirmation(self.opd,"SWAB COMPLETED")

        self.mega.prepCut()
        self.wait_for_confirmation(self.mega, "PREP START")
        self.wait_for_confirmation(self.mega, "FILAMENT RDY")

        self.opd.cut()
        self.wait_for_confirmation(self.opd,"CUT RDY")

        self.mega.cut()
        self.wait_for_confirmation(self.mega, "CUT START")
        self.wait_for_confirmation(self.mega, "CUT COMPLETED")


        self.opd.lid("CLOSE")
        self.wait_for_confirmation(self.opd,"LID ON")

        self.mega.Nai("DOWN")
        self.wait_for_confirmation(self.mega,"NAI DOWN")

        self.opd.moveArm("STRG")
        self.wait_for_confirmation(self.opd,"MOVE COMPLETED")

        self.opd.lift("STRG", "UP")
        self.wait_for_confirmation(self.opd,"LIFT UP")

        self.opd.lift("STRG", "DOWN")
        self.wait_for_confirmation(self.opd,"LIFT UP")


        self.current_screen.update_progress(self.current_run)
        self.current_run+=1
        
    def run(self):
        if(self.current_run<self.numberOfPlates):
            self.run_single()
            self.root.after(200,self.run)
        else:
            self.current_screen.update_progress(self.current_run)
            self.current_screen.enable_done_button()

    def RemoveCart(self):
        self.opd.write("REMOVECTRG")
        self.wait_for_confirmation(self.opd,"CTRG RDY")
        self.current_screen.update_message()

    @staticmethod
    def wait_for_confirmation(comObj : MEGAobj.MegaObj | OPBobj.OPBobj, tar_resp : str,err_resp: str=None , timeout : int=5):
        start_time = time.time()
        while True:
            response = comObj.read()
            
            if response == tar_resp:
                return True
            if response == err_resp:
                raise Exception(f"ERROR: {err_resp}" )
            if time.time() - start_time > timeout:
                raise TimeoutError(f"Timeout waiting for response: {tar_resp}")


if __name__ == "__main__":
    root = tk.Tk()
    app = AppController(root)
    app.update()
    root.mainloop()