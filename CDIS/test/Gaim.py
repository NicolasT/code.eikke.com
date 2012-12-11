import gobject
import CdisInstantMessenger

class Gaim(CdisInstantMessenger.CdisInstantMessenger):
        def __init__(self):
                CdisInstantMessenger.CdisInstantMessenger.__init__(self, "net.sf.gaim", "/net/sf/gaim/StatusHandler")
                self.SetStatusHandler(self.SetStatusInt)
                print "Setting initial presence to 'online'"
                self.status = "online"
                print "Initialized"
                print ""

        def SetStatusInt(self, status):
                print "Changed status to %s" % status

gaim = Gaim()
mainloop = gobject.MainLoop()
mainloop.run()
