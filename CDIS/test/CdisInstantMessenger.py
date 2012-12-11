import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
        import dbus.glib

class CdisInstantMessenger(dbus.service.Object):
        def __init__(self, service_name, object_path):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName(service_name, bus=self.bus)
                dbus.service.Object.__init__(self, self.bus_name, object_path)

                #Register ourself to the concentrator
                self.cdis_proxy = self.bus.get_object('org.freedesktop.CDIS', '/org/freedesktop/CdisConcentrator')
                self.cdis_iface = dbus.Interface(self.cdis_proxy, 'org.freedesktop.CDISIFace')
                self.cdis_iface.RegisterService(service_name, object_path, "org.freedesktop.CDIS.InstantMessenger")

        def SetStatusHandler(self, func):
                self.setstatushandler = func

        @dbus.service.method('org.freedesktop.CDIS.InstantMessenger')
        def SetStatus(self, status):
                self.setstatushandler(status)
