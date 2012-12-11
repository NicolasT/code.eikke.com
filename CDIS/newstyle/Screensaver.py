import gobject
from cdissoftphoneclient import CdisSoftPhoneClient
from cdissoftphonecontroller import CdisSoftPhoneController
from datetime import datetime
from time import sleep

class ScreenSaver:
        def __init__(self):
                self.cspc = CdisSoftPhoneClient("org.gnome.ScreenSaver", "/org/gnome/ScreenSaver")
                self.cspc.SetIncomingCallHandler(self.incoming_call)
                self.cspc.SetCallTerminatedHandler(self.call_terminated)

        def incoming_call(self):
                print "[" + str(datetime.now()) + "] SS: Incoming call, not blanking"
                #Stop our internal timer, whatever

        def call_terminated(self):
                print "[" + str(datetime.now()) + "] SS: Call terminated, restarting timer"
                #Restart timer
                #This is fake, obviously
                sleep(5)
                self.blank()

        def blank(self):
                print "[" + str(datetime.now()) + "] SS: Session idle, blanking"
                #Obviously, we should implement CdisScreenSaver, and emit a "Blanking" signal on the correct interface, which should be picked up by any CdisSoftPhone to go idle
                #As an example, we do tell SoftPhone's to go "idle" (mark the user as "Away")
                c = CdisSoftPhoneController("org.gnome.ScreenSaver", "/org/gnome/ScreenSaver/SoftPhoneController")
                c.Idle()

ss = ScreenSaver()
print "[" + str(datetime.now()) + "] SS: started"
mainloop = gobject.MainLoop()
mainloop.run()
