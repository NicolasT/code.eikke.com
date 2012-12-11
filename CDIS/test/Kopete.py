import gobject
import CdisInstantMessenger

class Kopete(CdisInstantMessenger.CdisInstantMessenger):
        def __init__(self):
                CdisInstantMessenger.CdisInstantMessenger.__init__(self, "org.kde.kopete", "/org/kde/kopete/KopeteIMStatusHandler")
                self.SetStatusHandler(self.SetStatusInt)
                print "Setting initial presence to 'away'"
                self.status = "away"
                print "Done initializing, chat away"
                print ""

        def SetStatusInt(self, status):
                print "Changed status to %s" % status

kopete = Kopete()
mainloop = gobject.MainLoop()
mainloop.run()
