include			../../../Makefile.common
include			../../Makefile.common
include			../../serial/Makefile.common
include			../Makefile.common

INCDIR 		= $(WMS_WXT536_SRC_HOME)/include
BINDIR		= $(WMS_WXT536_BIN_HOME)/test/$(HOSTTYPE)
DOCSDIR 	= $(WMS_WXT536_DOC_HOME)/cdocs

SERIAL_INCDIR	= $(WMS_SERIAL_SRC_HOME)/include


LOGGINGCFLAGS 	= -DLOGGING=10

CFLAGS = -g $(CCHECKFLAG) $(SHARED_LIB_CFLAGS) -I$(INCDIR) -I$(SERIAL_INCDIR) \
	 -L$(LT_LIB_HOME) $(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS)

DOCFLAGS = -static
SRCS 		= 	wxt536_send_command.c
OBJS		=	$(SRCS:%.c=$(BINDIR)/%.o)
EXES		=	$(SRCS:%.c=$(BINDIR)/%)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/%: %.o 
	cc -o $@ $^ -L$(LT_LIB_HOME) -l$(WMS_WXT536_LIBNAME) -l$(WMS_SERIAL_LIBNAME) $(TIMELIB) $(SOCKETLIB) -lm -lc

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(PROGS) $(OBJS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

