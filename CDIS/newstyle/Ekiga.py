import gobject 
import dbus
if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
        import dbus.glib
from cdismusicplayercontroller import CdisMusicPlayerController
from cdissoftphone import CdisSoftPhone
from datetime import datetime
from time import sleep

class Ekiga(CdisSoftPhone):
        def __init__(self):
                CdisSoftPhone.__init__(self, "org.ekiga.Ekiga", "/org/ekiga/Ekiga/CdisServer")
                self.cmpc = CdisMusicPlayerController("org.ekiga.Ekiga", "/org/ekiga/Ekiga/CdisController")
                self.SetIdleHandler(self.Idle)
                
        def IncomingCall(self):
                #Jay, someone likes me
                print "[" + str(datetime.now()) + "] Ekiga: Got an incoming call. Music players, please pause"
                self.cmpc.Pause()
                print "[" + str(datetime.now()) + "] Ekiga: Broadcasting IncomingCall event"
                CdisSoftPhone.IncomingCall(self)

        def TerminateCall(self):
                print "[" + str(datetime.now()) + "] Ekiga: Call terminated"
                self.cmpc.Resume()
                CdisSoftPhone.CallTerminated(self)

        def Idle(self):
                print "[" + str(datetime.now()) + "] Ekiga: Going idle"

e = Ekiga()
print "[" + str(datetime.now()) + "] Ekiga: started"
sleep(2)
e.IncomingCall()
sleep(10)
e.TerminateCall()
mainloop = gobject.MainLoop()
mainloop.run()
