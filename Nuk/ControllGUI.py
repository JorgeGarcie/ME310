import gui as gg
import serial
import tkinter as tk
import time

import MEGAobj
import ORBobj 
import ORB2obj

class AppController:
    def __init__(self,root):
        self.photos = [
    tk.PhotoImage(file="Nuk/ImagesForGUI/image1.png"),
    tk.PhotoImage(file="Nuk/ImagesForGUI/image2.png"),
    tk.PhotoImage(file="Nuk/ImagesForGUI/image3.png")
]
        self.root=root
        # root.attributes("-fullscreen", True)
        self.numberOfPlates=1
        self.petriDishType=None
        self.swabStyle=None

        self.screens = [gg.StartScreen, gg.PetriSelector, gg.SwabSelector3, gg.NumberSelector,gg.SummaryScreen,gg.RunningScreen,gg.WaitScreen]
        self.current_index = 0
        self.current_screen = None

        self.isRun=False
        
        self.current_run=0
    
        
        # self.mega = MEGAobj.MegaObj(port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1)
        # self.orb = ORBobj.ORBobj(port='/dev/cu.usbmodem21201', baudrate=115200, timeout=1)
        # self.mega.initCom()
        # self.orb2 = ORB2obj.ORB2(port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1)



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
        self.orb.moveArm(self.petriDishType)
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")

        self.GetDish(self.petriDishType)

        self.orb.moveArm("WORK AREA")
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")

        self.mega.Nai("UP")
        self.wait_for_confirmation(self.mega,"NAI UP")

        self.orb.sucction("ON")
        self.wait_for_confirmation(self.orb,"SUCC ON")

        self.orb.lid("OPEN")
        self.wait_for_confirmation(self.orb,"LID REMOVED")

        self.orb.fetch()
        self.wait_for_confirmation(self.orb,"FETCH RDY")

        self.mega.fetch()
        self.wait_for_confirmation(self.mega, "FETCH START")
        self.wait_for_confirmation(self.mega, "FETCH COMPLETED")


        self.orb.extrude()
        self.wait_for_confirmation(self.orb,"EXTRUDE RDY")

        self.mega.extrude()
        self.wait_for_confirmation(self.mega, "EXTRUDE START")
        self.wait_for_confirmation(self.mega, "EXTRUDE COMPLETED","EXTRUDE FAILED")


        self.orb.swab(self.petriDishType)
        self.wait_for_confirmation(self.orb,"SWAB COMPLETED")

        self.mega.prepCut()
        self.wait_for_confirmation(self.mega, "PREP START")
        self.wait_for_confirmation(self.mega, "FILAMENT RDY")

        self.orb.cut()
        self.wait_for_confirmation(self.orb,"CUT RDY")

        self.mega.cut()
        self.wait_for_confirmation(self.mega, "CUT START")
        self.wait_for_confirmation(self.mega, "CUT COMPLETED")


        self.orb.lid("CLOSE")
        self.wait_for_confirmation(self.orb,"LID ON")

        self.orb.sucction("OFF")
        self.wait_for_confirmation(self.orb,"SUCC OFF")


        self.mega.Nai("DOWN")
        self.wait_for_confirmation(self.mega,"NAI DOWN")

        self.orb.moveArm("STRG")
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")

        self.orb.lift("STRG", "UP")
        self.wait_for_confirmation(self.orb,"LIFT UP")

        self.orb.lift("STRG", "DOWN")
        self.wait_for_confirmation(self.orb,"LIFT UP")


        self.current_screen.update_progress(self.current_run)
        self.current_run+=1

    def GetDish(self,TYPE):
        self.orb.lift(TYPE,"UP")
        self.wait_for_confirmation(self.orb,"LIFT UP")
        self.orb2.releas(TYPE)
        self.wait_for_confirmation("RELEASED")
        self.orb.lift(TYPE,"MID")
        self.wait_for_confirmation(self.orb,"LIFT MID")
        self.orb2.grab(TYPE)
        self.wait_for_confirmation("GRABBED")
        self.orb.lift(TYPE,"DOWN")
        self.wait_for_confirmation(self.orb,"LIFT DOWN")
        
    def run(self):
        if(self.current_run<self.numberOfPlates):
            self.run_single()
            self.root.after(200,self.run)
        else:
            self.current_screen.update_progress(self.current_run)
            self.current_screen.enable_done_button()

    def RemoveCart(self):
        self.orb.liftAll("UP")
        self.wait_for_confirmation(self.orb,"ALL LIFT UP")
        self.orb2.releasAll()
        self.wait_for_confirmation("RELEASED ALL")
        self.orb.liftAll("TOP")
        self.wait_for_confirmation(self.orb,"ALL LIFT TOP")
        self.wait_for_confirmation(self.orb,"CTRG RDY")
        self.current_screen.update_message()
    
    def LoadCart(self):
        self.orb.liftAll("UP")
        self.wait_for_confirmation(self.orb,"ALL LIFT UP")
        self.orb2.grabAll()
        self.wait_for_confirmation("GRABBED ALL")
        self.orb.liftAll("DOWN")
        self.wait_for_confirmation(self.orb,"ALL LIFT DOWN")

        self.numberOfPlates=1
        self.petriDishType=None
        self.swabStyle=None      
        self.current_index = 0 
        self.update()


    @staticmethod
    def wait_for_confirmation(comObj : MEGAobj.MegaObj | ORBobj.ORBobj, tar_resp : str,err_resp: str=None , timeout : int=5):
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