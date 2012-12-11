SUBDIRS = mixer

INCLUDES = $(GSTREAMER_CFLAGS)

check_PROGRAMS = xmltest1 spectrumtest
check_LTLIBRARIES = libgstfftw.la
TESTS = $(check_PROGRAMS)

xmltest1_SOURCES = xmltest1.c
xmltest1_LDADD = $(GSTREAMER_LIBS)

spectrumtest_SOURCES = spectrumtest.c
spectrumtest_CFLAGS = $(GTK2_CFLAGS)
spectrumtest_LDADD = $(GSTREAMER_LIBS) $(GTK2_LIBS)

libgstfftw_la_SOURCES = gstfftw.c
libgstfftw_la_CFLAGS = $(GSTREAMER_CFLAGS) $(FFTW_CFLAGS)
libgstfftw_la_LDFLAGS = $(GSTREAMER_LIBS)
libgstfftw_la_LIBADD = $(GSTREAMER_LIBS) $(FFTW_LIBS)
