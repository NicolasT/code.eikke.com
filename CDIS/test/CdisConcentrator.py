import gobject
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0) >= (0,41,0)):
                import dbus.glib

class CdisConcentrator(dbus.service.Object):
        def __init__(self):
                self.bus = dbus.SessionBus()
                self.bus_name = dbus.service.BusName('org.freedesktop.CDIS', bus=self.bus)
                self.cdis_objects = list()
                dbus.service.Object.__init__(self, self.bus_name, '/org/freedesktop/CdisConcentrator')
                print "Concentrator initialized, waiting for clients"
                print ""

        @dbus.service.method('org.freedesktop.CDISIFace')
        def RegisterService(self, service_name, object_name, interface_name):
                print "New object registered:"
                print "     Service: %s" % service_name
                print "     Object: %s" % object_name
                print "     Interface: %s" % interface_name
                print ""
                proxy = self.bus.get_object(service_name, object_name)
                o = {"service": service_name, "object": object_name, "interface": interface_name, "proxy": proxy}
                self.cdis_objects.append(o)

        @dbus.service.method('org.freedesktop.CDIS.InstantMessengerIface')
        def SetStatus(self, status):
                print "Request to set InstantMessenger status to %s" % status
                for o in self.cdis_objects:
                        if o["interface"] == 'org.freedesktop.CDIS.InstantMessenger':
                                iface = dbus.Interface(o["proxy"], o["interface"])
                                iface.SetStatus(status)



concentrator = CdisConcentrator()

mainloop = gobject.MainLoop()
mainloop.run()
