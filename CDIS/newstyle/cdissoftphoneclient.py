import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
        import dbus.glib

class CdisSoftPhoneClient(dbus.service.Object):
        def __init__(self, service_name, object_path):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName(service_name, bus=self.bus)
                dbus.service.Object.__init__(self, self.bus_name, object_path)

                self.bus.add_signal_receiver(self.incomingcall_cb, signal_name = "IncomingCall", dbus_interface = "org.freedesktop.CDIS.SoftPhone")
                self.bus.add_signal_receiver(self.callterminated_cb, signal_name = "CallTerminated", dbus_interface = "org.freedesktop.CDIS.SoftPhone")

        def SetIncomingCallHandler(self, func):
                self.incomingcallhandler = func

        def SetCallTerminatedHandler(self, func):
                self.callterminatedhandler = func

        def incomingcall_cb(self):
                if self.incomingcallhandler:
                        self.incomingcallhandler()

        def callterminated_cb(self):
                if self.callterminatedhandler:
                        self.callterminatedhandler()
