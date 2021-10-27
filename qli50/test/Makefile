include			../../../Makefile.common
include			../../Makefile.common
include			../../serial/Makefile.common
include			../Makefile.common

INCDIR 		= $(WMS_QLI50_SRC_HOME)/include
BINDIR		= $(WMS_QLI50_BIN_HOME)/test/$(HOSTTYPE)
DOCSDIR 	= $(WMS_QLI50_DOC_HOME)/test

SERIAL_INCDIR	= $(WMS_SERIAL_SRC_HOME)/include


LOGGINGCFLAGS 	= -DLOGGING=10

CFLAGS = -g $(CCHECKFLAG) $(SHARED_LIB_CFLAGS) -I$(INCDIR) -I$(SERIAL_INCDIR) \
	 -L$(LT_LIB_HOME) $(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS)

DOCFLAGS = -static
SRCS 		=	qli50_command_close.c qli50_command_echo.c qli50_command_open.c qli50_command_par.c \
			qli50_command_reset.c qli50_command_sta.c \
			qli50_command_read_sensors.c qli50_command_send_results.c \
			qli50_server_test.c 
OBJS		=	$(SRCS:%.c=$(BINDIR)/%.o)
EXES		=	$(SRCS:%.c=$(BINDIR)/%)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/%: %.o 
	cc -o $@ $^ -L$(LT_LIB_HOME) -l$(WMS_QLI50_LIBNAME) -l$(WMS_SERIAL_LIBNAME) $(TIMELIB) $(SOCKETLIB) -lm -lc

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(PROGS) $(OBJS) $(EXES) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

