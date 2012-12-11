import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
        import dbus.glib

class CdisMusicPlayerController(dbus.service.Object):
        def __init__(self, service_name, object_path):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName(service_name, bus=self.bus)
                dbus.service.Object.__init__(self, self.bus_name, object_path)

        @dbus.service.signal("org.freedesktop.CDIS.MusicPlayer")
        def Pause(self):
                pass

        @dbus.service.signal("org.freedesktop.CDIS.MusicPlayer")
        def Resume(self):
                pass
                
