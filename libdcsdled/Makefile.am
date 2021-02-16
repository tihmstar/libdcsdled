AM_CFLAGS = -I$(top_srcdir)/include/libdcsdled $(GLOBAL_CFLAGS)  $(libftdi_CFLAGS) $(libgeneral_CFLAGS)
AM_CXXFLAGS = $(AM_CFLAGS) $(GLOBAL_CXXFLAGS)
AM_LDFLAGS = $(libftdi_LIBS) $(libgeneral_LIBS)

lib_LTLIBRARIES = libdcsdled.la

libdcsdled_la_CFLAGS = $(AM_CFLAGS)
libdcsdled_la_CXXFLAGS = $(AM_CXXFLAGS)
libdcsdled_la_LDFLAGS = $(AM_LDFLAGS)
libdcsdled_la_SOURCES = 	DCSDLed.cpp
