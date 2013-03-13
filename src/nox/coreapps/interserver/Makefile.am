include ../../../Make.vars 

CONFIGURE_DEPENCIES = $(srcdir)/Makefile.am

EXTRA_DIST =\
	meta.json

pkglib_LTLIBRARIES =		\
	interserver.la \
	request_processor.la \
	inter_sw_config.la

interserver_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
interserver_la_SOURCES = interserver.cc http_event.cc
interserver_la_LIBADD = -lmicrohttpd
interserver_la_LDFLAGS = -module -export-dynamic -L/usr/local/lib

request_processor_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
request_processor_la_SOURCES = request_processor.cc http_event.cc
request_processor_la_LDFLAGS = -module -export-dynamic

inter_sw_config_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
inter_sw_config_la_SOURCES = inter_sw_config.cc
inter_sw_config_la_LDFLAGS = -module -export-dynamic

NOX_RUNTIMEFILES = meta.json

all-local: nox-all-local
clean-local: nox-clean-local 
install-exec-hook: nox-install-local
