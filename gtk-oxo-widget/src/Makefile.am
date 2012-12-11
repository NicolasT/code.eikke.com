INCLUDES = $(GTK_CFLAGS)

bin_PROGRAMS = test-gtk-oxo-widget

test_gtk_oxo_widget_SOURCES = gtk-oxo-widget.c test-gtk-oxo-widget.c
test_gtk_oxo_widget_LDADD = $(GTK_LIBS)

%.c %.h %-private.h: %.gob
	@GOB2@ $<
			
