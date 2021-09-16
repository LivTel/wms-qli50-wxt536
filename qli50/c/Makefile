include			../../../Makefile.common
include			../../Makefile.common
include			../../serial/Makefile.common
include			../Makefile.common

INCDIR 		= $(WMS_QLI50_SRC_HOME)/include
BINDIR		= $(WMS_QLI50_BIN_HOME)/c/$(HOSTTYPE)
DOCSDIR 	= $(WMS_QLI50_DOC_HOME)/cdocs

SERIAL_INCDIR	= $(WMS_SERIAL_SRC_HOME)/include
# Do we want to enable logging?
# no logging
#LOGGINGCFLAGS = -DLOGGING=0
LOGGINGCFLAGS = -DLOGGING=10

#Do we want MUTEX protected locking around controller command communication?
MUTEXCFLAGS = -DWMS_QLI50_MUTEXED=1
#MUTEXCFLAGS =

CFLAGS = -g $(CCHECKFLAG) $(SHARED_LIB_CFLAGS) -I$(INCDIR) -I$(SERIAL_INCDIR) -L$(LT_LIB_HOME) $(MUTEXCFLAGS) \
	$(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS)

LINTFLAGS = -I$(INCDIR) -I$(SERIAL_INCDIR)
DOCFLAGS = -static
SRCS 		= 	wms_qli50_general.c
# wms_qli50_connection.c wms_qli50_command.c 
HEADERS		=	$(SRCS:%.c=%.h)
OBJS		=	$(SRCS:%.c=$(BINDIR)/%.o)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)

top: shared docs

shared: $(LT_LIB_HOME)/lib$(WMS_QLI50_LIBNAME).so

$(LT_LIB_HOME)/lib$(WMS_QLI50_LIBNAME).so: $(OBJS)
	cc $(CCSHAREDFLAG) $(CFLAGS) $(OBJS) -o $@ $(TIMELIB) -l$(WMS_SERIAL_LIBNAME)

static: $(LT_LIB_HOME)/lib$(WMS_WXT536_LIBNAME).a

$(LT_LIB_HOME)/lib$(WMS_QLI50_LIBNAME).a: $(OBJS)
	ar rcv $@ $?

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	-$(RM) $(RM_OPTIONS) $(OBJS) $(LT_LIB_HOME)/$(WMS_QLI50_LIBNAME).so $(LT_LIB_HOME)/$(WMS_QLI50_LIBNAME).a $(TIDY_OPTIONS)

tidy:
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	-(cd $(INCDIR); $(RM) $(RM_OPTIONS) $(TIDY_OPTIONS);)
