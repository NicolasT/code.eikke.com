import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
        import dbus.glib

class CdisSoftPhone(dbus.service.Object):
        def __init__(self, service_name, object_path):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName(service_name, bus=self.bus)
                dbus.service.Object.__init__(self, self.bus_name, object_path)

                self.bus.add_signal_receiver(self.idle_cb, signal_name = "Idle", dbus_interface = "org.freedesktop.CDIS.SoftPhone")

        def SetIdleHandler(self, func):
                self.idlehandler = func

        def idle_cb(self):
                if self.idlehandler:
                        self.idlehandler()

        @dbus.service.signal("org.freedesktop.CDIS.SoftPhone")
        def IncomingCall(self):
                pass

        @dbus.service.signal("org.freedesktop.CDIS.SoftPhone")
        def CallTerminated(self):
                pass
