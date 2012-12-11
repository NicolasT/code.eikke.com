import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
        import dbus.glib

class CdisMusicPlayer(dbus.service.Object):
        def __init__(self, service_name, object_path):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName(service_name, bus=self.bus)
                dbus.service.Object.__init__(self, self.bus_name, object_path)

                self.bus.add_signal_receiver(self.pause_cb, signal_name = "Pause", dbus_interface = "org.freedesktop.CDIS.MusicPlayer")
                self.bus.add_signal_receiver(self.resume_cb, signal_name = "Resume", dbus_interface = "org.freedesktop.CDIS.MusicPlayer")

        def SetPauseHandler(self, func):
                self.pausehandler = func

        def SetResumeHandler(self, func):
                self.resumehandler = func

        def pause_cb(self):
                if self.pausehandler:
                        self.pausehandler()

        def resume_cb(self):
                if self.resumehandler:
                        self.resumehandler()

