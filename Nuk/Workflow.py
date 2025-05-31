import serial
import tkinter as tk
import time
import MEGAobj
import OPBobj


# init
mega=MEGAobj.MegaObj(port='/dev/cu.usbmodem21301', baudrate=115200, timeout=1)

mega.initCom()

opd = OPBobj.OPBobj(port='/dev/cu.usbmodem21201', baudrate=115200, timeout=1)



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

Start=True

while True: #This is the GUI loop
    STYLE   =    "SPIRAL"
    TYPE    =    "NORMAL"

    
    try:
        if Start:

            opd.moveArm(TYPE)
            wait_for_confirmation(opd,"MOVE COMPLETED")

            opd.getDish(TYPE)
            wait_for_confirmation(opd,"DISH RDY")

            opd.moveArm("WORK AREA")
            wait_for_confirmation(opd,"MOVE COMPLETED")

            opd.Nai("UP")
            wait_for_confirmation(opd,"NAI UP")

            opd.sucction("ON")
            wait_for_confirmation(opd,"SUCC ON")

            opd.lid("OPEN")
            wait_for_confirmation(opd,"LID REMOVED")


            opd.fetch()
            wait_for_confirmation(opd,"FETCH RDY")

            mega.fetch()
            wait_for_confirmation(mega, "FETCH START")
            wait_for_confirmation(mega, "FETCH COMPLETED")


            opd.extrude()
            wait_for_confirmation(opd,"EXTRUDE RDY")

            mega.extrude()
            wait_for_confirmation(mega, "EXTRUDE START")
            wait_for_confirmation(mega, "EXTRUDE COMPLETED","EXTRUDE FAILED")


            opd.swab(TYPE)
            wait_for_confirmation(opd,"SWAB COMPLETED")

            mega.prepCut()
            wait_for_confirmation(mega, "PREP START")
            wait_for_confirmation(mega, "FILAMENT RDY")

            opd.cut()
            wait_for_confirmation(opd,"CUT RDY")

            mega.cut()
            wait_for_confirmation(mega, "CUT START")
            wait_for_confirmation(mega, "CUT COMPLETED")


            opd.lid("CLOSE")
            wait_for_confirmation(opd,"LID ON")

            opd.moveArm("STRG")
            wait_for_confirmation(opd,"MOVE COMPLETED")

            opd.lift("STRG", "UP")
            wait_for_confirmation(opd,"LIFT UP")

            opd.lift("STRG", "DOWN")
            wait_for_confirmation(opd,"LIFT UP")

            Start=False

    except:
        print("################################")
        print('\nWe have a bobo\n\n')


    


