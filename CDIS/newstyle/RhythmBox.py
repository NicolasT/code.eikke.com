import gobject
import cdismusicplayer
from datetime import datetime

class RhythmBox(cdismusicplayer.CdisMusicPlayer):
        def __init__(self):
                cdismusicplayer.CdisMusicPlayer.__init__(self, "org.gnome.RhythmBox", "/org/gnome/RhythoBox/CDISController")
                self.SetPauseHandler(self.PauseHandler)
                self.SetResumeHandler(self.ResumeHandler)
                print "[" + str(datetime.now()) + "] RB: Starting music playback"
                self.playing = True

        def PauseHandler(self):
                if self.playing == True:
                        print "[" + str(datetime.now()) + "] RB: Request to pause playback"
                        self.playing = False
                else:
                        print "[" + str(datetime.now()) + "] RB: Request to pause playback, but we weren't spinning anything. Ignored!"

        def ResumeHandler(self):
                if self.playing == True:
                        print "[" + str(datetime.now()) + "] RB: Request to resume, but we were already playing. Strange"
                else:
                        print "[" + str(datetime.now()) + "] RB: Request to resume playback. Jay!"
                        self.playing = True

rb = RhythmBox()
print "[" + str(datetime.now()) + "] RB: started"
mainloop = gobject.MainLoop()
mainloop.run()
