include			../../../Makefile.common
include			../../Makefile.common
include			../Makefile.common

INCDIR 		= $(WMS_SERIAL_SRC_HOME)/include
BINDIR		= $(WMS_SERIAL_BIN_HOME)/c/$(HOSTTYPE)
DOCSDIR 	= $(WMS_SERIAL_DOC_HOME)/cdocs

# Do we want to enable logging?
# no logging
#LOGGINGCFLAGS = -DLOGGING=0
LOGGINGCFLAGS = -DLOGGING=10

#Do we want MUTEX protected locking around controller command communication?
MUTEXCFLAGS = -DWMS_SERIAL_MUTEXED=1
#MUTEXCFLAGS = 

# Disable Nagle's algorithm?
#TCP_NODELAY = -DWMS_SERIAL_TCP_NODELAY=1
TCP_NODELAY = 
CFLAGS = -g $(CCHECKFLAG) $(SHARED_LIB_CFLAGS) -I$(INCDIR) -L$(LT_LIB_HOME) $(MUTEXCFLAGS) \
	$(TCP_NODELAY) $(LOGGINGCFLAGS) $(LOG_UDP_CFLAGS)

LINTFLAGS = -I$(INCDIR) -I$(JNIINCDIR) -I$(JNIMDINCDIR)
DOCFLAGS = -static
SRCS 		= 	wms_serial_general.c wms_serial_serial.c 
HEADERS		=	$(SRCS:%.c=%.h)
OBJS		=	$(SRCS:%.c=$(BINDIR)/%.o)
DOCS 		= 	$(SRCS:%.c=$(DOCSDIR)/%.html)

top: shared docs 

shared: $(LT_LIB_HOME)/lib$(WMS_SERIAL_LIBNAME).so

$(LT_LIB_HOME)/lib$(WMS_SERIAL_LIBNAME).so: $(OBJS)
	cc $(CCSHAREDFLAG) $(CFLAGS) $(OBJS) -o $@ $(TIMELIB) 

static: $(LT_LIB_HOME)/lib$(WMS_SERIAL_LIBNAME).a

$(LT_LIB_HOME)/lib$(WMS_SERIAL_LIBNAME).a: $(OBJS)
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
	-$(RM) $(RM_OPTIONS) $(OBJS) $(LT_LIB_HOME)/$(WMS_SERIAL_LIBNAME).so $(LT_LIB_HOME)/$(WMS_SERIAL_LIBNAME).a $(TIDY_OPTIONS)

tidy:
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	-(cd $(INCDIR); $(RM) $(RM_OPTIONS) $(TIDY_OPTIONS);)
