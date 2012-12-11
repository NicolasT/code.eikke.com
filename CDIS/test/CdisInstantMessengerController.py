import dbus

bus = dbus.SessionBus()
proxy_obj = bus.get_object('org.freedesktop.CDIS', '/org/freedesktop/CdisConcentrator')
iface = dbus.Interface(proxy_obj, 'org.freedesktop.CDIS.InstantMessengerIface')

print "Setting status to 'idle'..."
iface.SetStatus("idle")
print "Done"
