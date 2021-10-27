include			../../Makefile.common
include			../Makefile.common
include			../serial/Makefile.common
include			../qli50/Makefile.common
include			../wxt536/Makefile.common

INCDIR 		= $(WMS_QLI50_WXT536_SRC_HOME)/include
BINDIR		= $(WMS_QLI50_WXT536_BIN_HOME)/c/$(HOSTTYPE)
DOCSDIR 	= $(WMS_QLI50_WXT536_DOC_HOME)/cdocs


LOGGINGCFLAGS 	= -DLOGGING=10

SERIAL_INCDIR	= $(WMS_SERIAL_SRC_HOME)/include
SERIAL_LDFLAGS	= -l$(WMS_SERIAL_LIBNAME)

QLI50_INCDIR	= $(WMS_QLI50_SRC_HOME)/include
QLI50_LDFLAGS	= -l$(WMS_QLI50_LIBNAME)

WXT536_INCDIR	= $(WMS_WXT536_SRC_HOME)/include
WXT536_LDFLAGS	= -l$(WMS_WXT536_LIBNAME)


CFLAGS 	= -g $(CCHECKFLAG) $(SHARED_LIB_CFLAGS) -I$(INCDIR) -I$(SERIAL_INCDIR) -I$(QLI50_INCDIR) -I$(WXT536_INCDIR) \
	  $(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS) $(CONFIG_CFLAGS)

LDFLAGS = -L$(LT_LIB_HOME) $(SERIAL_LDFLAGS) $(QLI50_LDFLAGS) $(WXT536_LDFLAGS) -llog_udp $(CONFIG_LDFLAGS) \
	  $(TIMELIB) $(SOCKETLIB) -lm -lc

DOCFLAGS = -static
EXE		= qli50-wxt536
SRCS 		= qli50_wxt536_main.c qli50_wxt536_general.c qli50_wxt536_config.c qli50_wxt536_server.c
OBJS		=	$(SRCS:%.c=$(BINDIR)/%.o)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(BINDIR)/$(EXE) docs

$(BINDIR)/$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BINDIR)/%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(PROGS) $(OBJS) $(EXES) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

