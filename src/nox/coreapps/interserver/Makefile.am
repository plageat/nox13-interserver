include ../../../Make.vars 

CONFIGURE_DEPENCIES = $(srcdir)/Makefile.am

EXTRA_DIST =\
	meta.json

pkglib_LTLIBRARIES =		\
	interserver.la \
	request_processor.la \
	msg_resolver.la	\
	facade_answer.la

interserver_la_CPPFLAGS = -I/usr/local/include/cpp-netlib  $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
interserver_la_SOURCES = interserver.cc http_event.cc
interserver_la_LIBADD = -lboost_system -lboost_regex -lboost_date_time -lboost_thread -lpthread
interserver_la_LDFLAGS = -module -export-dynamic -L/usr/lib

request_processor_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
request_processor_la_SOURCES = request_processor.cc http_event.cc interact_event.cc
request_processor_la_LDFLAGS = -module -export-dynamic

msg_resolver_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
msg_resolver_la_SOURCES = msg_resolver.cc interact_builders.cc
msg_resolver_la_LDFLAGS = -module -export-dynamic

facade_answer_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox -I. 
facade_answer_la_SOURCES = facade_answer.cc interact_event.cc interact_hreplies.cc
facade_answer_la_LDFLAGS = -module -export-dynamic

NOX_RUNTIMEFILES = meta.json

all-local: nox-all-local
clean-local: nox-clean-local 
install-exec-hook: nox-install-local
