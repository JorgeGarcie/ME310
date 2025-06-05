import gui as gg
import serial
import tkinter as tk
import time
import os

import MEGAobj
import ORBobj 
import ORB2obj

class AppController:
    def __init__(self,root):
        self.photos_Swab = [
    tk.PhotoImage(file="Nuk/ImagesForGUI/line.png"),
    tk.PhotoImage(file="Nuk/ImagesForGUI/quad.png"),
    tk.PhotoImage(file="Nuk/ImagesForGUI/spiral.png")
]
        self.photos_Dish=[
            tk.PhotoImage(file="Nuk/ImagesForGUI/blood.png"),
            tk.PhotoImage(file="Nuk/ImagesForGUI/macconkey.png"),
             tk.PhotoImage(file="Nuk/ImagesForGUI/chocolate.png")
        ]

        self.photoStart=tk.PhotoImage(file="Nuk/ImagesForGUI/start.png")
        self.photoBack=tk.PhotoImage(file="Nuk/ImagesForGUI/back.png")
        self.photoPlus=tk.PhotoImage(file="Nuk/ImagesForGUI/plus.png")
        self.photoNeg=tk.PhotoImage(file="Nuk/ImagesForGUI/minus.png")
        self.photoCCrtg=tk.PhotoImage(file="Nuk/ImagesForGUI/changecartridge.png")
        self.photoNext=tk.PhotoImage(file="Nuk/ImagesForGUI/next.png")
        self.photoCancel=tk.PhotoImage(file="Nuk/ImagesForGUI/cancel.png")
        self.photoRun=tk.PhotoImage(file="Nuk/ImagesForGUI/run.png")
        self.photoDone=tk.PhotoImage(file="Nuk/ImagesForGUI/done.png")

        self.photoRemove=tk.PhotoImage(file="Nuk/ImagesForGUI/RemoveIMG.png")
        self.photoInsert=tk.PhotoImage(file="Nuk/ImagesForGUI/Re-insertIMG.png")

        self.root=root
        # root.attributes("-fullscreen", True)
        self.numberOfPlates=1
        self.petriDishType=None
        self.swabStyle=None

        self.SWABlist=["LINE","SPIRAL","QUADRANT","ZIGZAG"]



        self.screens = [gg.StartScreen, gg.PetriSelector, gg.SwabSelector3, gg.NumberSelector,gg.SummaryScreen,gg.RunningScreen,gg.WaitScreen,gg.RemoveCRTG,gg.InsertCRTG]
        self.current_index = 0    
        self.current_screen = None

        self.isRun=False
        self.input_locked = False  # To prevent double taps


        self.current_run=0

        
        self.mega = MEGAobj.MegaObj(port='COM12', baudrate=115200, timeout=1)
        self.mega.initCom()
        self.orb = ORBobj.ORBobj(port='COM18', baudrate=115200, timeout=1)
        self.orb.initCom()
        self.orb2 = ORB2obj.ORB2(port='COM15', baudrate=115200, timeout=1)
        # self.CutFirst()
              
        
    
    def CutFirst(self):
        self.orb.cut()
        self.wait_for_confirmation(self.orb,"CUT RDY")

        self.mega.cut()
        self.wait_for_confirmation(self.mega, "CUT START")
        self.wait_for_confirmation(self.mega, "CUT COMPLETED")

        self.mega.cutopen()
        self.wait_for_confirmation(self.mega, "CUTOPEN COMPLETED")
        
    def show_screen(self, index):
        if self.current_screen:
            self.current_screen.frame.destroy()

        screen_class = self.screens[index]
        self.current_screen = screen_class(self.root, self)

    def lock_input(self):
        self.input_locked = True
        self.root.after(200, self.unlock_input)  # 500 ms = 0.5 seconds

    def unlock_input(self):
        self.input_locked = False

    def go_back(self):
        if not self.input_locked and self.current_index > 0:
            self.current_index -= 1
            self.show_screen(self.current_index)
            self.lock_input()

    def go_forward(self):
        if not self.input_locked and self.current_index < len(self.screens) - 1:
            self.current_index += 1
            self.show_screen(self.current_index)
            self.lock_input()

    def update(self):
        self.show_screen(self.current_index)
       
        
    def run_single(self):
     
        self.orb.moveArm(self.petriDishType)
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")

        self.GetDish(self.petriDishType)

        self.orb.moveArm("WORK AREA")
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")
            
        self.mega.Nai("UP")
        self.wait_for_confirmation(self.mega,"PLATFORM LIFT UP")

        self.orb.sucction("ON")
        self.wait_for_confirmation(self.orb,"SUCC ON")
            
        self.orb.lid("OPEN")
        self.wait_for_confirmation(self.orb,"LID REMOVED")

        self.orb.fetch()
        self.wait_for_confirmation(self.orb,"FETCH RDY")

        self.mega.fetch()
        self.wait_for_confirmation(self.mega, "FETCH START")
        self.wait_for_confirmation(self.mega, "FETCH COMPLETED",timeout=15)


        self.orb.extrude()
        self.wait_for_confirmation(self.orb,"EXTRUDE RDY")

        self.mega.extrude()
        self.wait_for_confirmation(self.mega, "EXTRUDE START")
        self.wait_for_confirmation(self.mega, "EXTRUDE COMPLETED","EXTRUDE FAILED")
        
        self.orb.swab(self.swabStyle)
        self.wait_for_confirmation(self.orb,"SWAB COMPLETED",timeout=60)

        self.mega.prepCut()
        self.wait_for_confirmation(self.mega, "PREP START")
        self.wait_for_confirmation(self.mega, "FILAMENT RDY")

        self.orb.sucction("OFF")
        self.wait_for_confirmation(self.orb,"SUCC OFF")

        self.mega.Nai("DOWN")
        self.wait_for_confirmation(self.mega,"PLATFORM LIFT DOWN")
        
        self.orb.cut()
        self.wait_for_confirmation(self.orb,"CUT RDY")

        self.mega.cut()
        self.wait_for_confirmation(self.mega, "CUT START")
        self.wait_for_confirmation(self.mega, "CUT COMPLETED")

        time.sleep(0.2)
        
        self.orb.fetch()
        self.wait_for_confirmation(self.orb,"FETCH RDY")

        self.mega.cutopen()
        self.wait_for_confirmation(self.mega, "CUTOPEN COMPLETED")
        
        self.orb.lid("CLOSE")
        self.wait_for_confirmation(self.orb,"LID ON")

        self.orb.moveArm("STRG")
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")

        self.StrgDish()
        
        self.current_run+=1
        

    def GetDish(self,TYPE):
        self.orb.lift(TYPE,"UP")
        self.wait_for_confirmation(self.orb,"LIFT UP")
        
        self.orb2.releas(TYPE)
        self.wait_for_confirmation(self.orb2,"RELEASED")
        
        self.orb.lift(TYPE,"MID")
        self.wait_for_confirmation(self.orb,"LIFT MID")
        
        self.orb2.grab(TYPE)
        self.wait_for_confirmation(self.orb2,"GRABBED")
        
        self.orb.lift(TYPE,"DOWN")
        self.wait_for_confirmation(self.orb,"LIFT DOWN")
    
    def StrgDish(self):
        self.orb.moveArm("STRG")
        self.wait_for_confirmation(self.orb,"MOVE COMPLETED")
        
        self.orb.lift("STRG","MID")
        self.wait_for_confirmation(self.orb,"LIFT MID")
        
        self.orb2.releas("STRG")
        self.wait_for_confirmation(self.orb2,"RELEASED")
        
        self.orb.lift("STRG", "UP")
        self.wait_for_confirmation(self.orb,"LIFT UP")
        
        self.orb2.grab("STRG")
        self.wait_for_confirmation(self.orb2,"GRABBED")

        self.orb.lift("STRG", "DOWN")
        self.wait_for_confirmation(self.orb,"LIFT DOWN")
    
    def run(self):
        if(self.current_run<self.numberOfPlates):
            
            self.run_single()
            self.current_screen.update_progress(self.current_run)
            
            self.root.after(200,self.run)
        else:
            self.current_screen.enable_done_button()

    def RemoveCart(self):
        
        self.orb.liftAll("UP")
        self.wait_for_confirmation(self.orb,"ALL LIFT UP")
        
        self.orb2.releasAll()
        self.wait_for_confirmation(self.orb2,"RELEASED ALL")
        
        self.orb.liftAll("TOP")
        self.wait_for_confirmation(self.orb,"ALL LIFT TOP")
        
        self.orb2.grabAll()
        self.wait_for_confirmation(self.orb2,"GRABBED ALL")
        
        self.current_screen.update_message()
    
    def LoadCart(self):
        self.orb.liftAll("TOP")
        self.wait_for_confirmation(self.orb,"ALL LIFT TOP")
        
        self.orb2.releasAll()
        self.wait_for_confirmation(self.orb2,"RELEASED ALL")
        
        self.orb.liftAll("UP")
        self.wait_for_confirmation(self.orb,"ALL LIFT UP")
        
        self.orb2.grabAll()
        self.wait_for_confirmation(self.orb2,"GRABBED ALL")
        
        self.orb.liftAll("DOWN")
        self.wait_for_confirmation(self.orb,"ALL LIFT DOWN")

        self.numberOfPlates=1
        self.petriDishType=None
        self.swabStyle=None      
        self.current_index = 0 
        self.update()


    @staticmethod
    def wait_for_confirmation(comObj : MEGAobj.MegaObj | ORBobj.ORBobj | ORB2obj.ORB2, tar_resp : str,err_resp: str=None , timeout : int=10):
        start_time = time.time()
        while True:
            response = comObj.read()
            print(response)
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