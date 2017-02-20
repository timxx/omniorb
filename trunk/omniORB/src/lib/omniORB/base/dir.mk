# dir.mk for omniORB base.

BASE_SRCS = \
            logIOstream.cc

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += $(OMNITHREAD_CPPFLAGS)
DIR_CPPFLAGS += -D_OMNIORB_BASE_LIBRARY
DIR_CPPFLAGS += -DOMNIORB_VERSION_STRING='"$(OMNIORB_VERSION)"'
DIR_CPPFLAGS += -DOMNIORB_VERSION_HEX='$(OMNIORB_VERSION_HEX)'

##########################################################################
# Add magic to find tcp transport source files
#CXXVPATH = $(VPATH):$(VPATH:%=%/tcp)
CXXVPATH = $(VPATH)

##########################################################################
ifdef OMNIORB_CONFIG_DEFAULT_LOCATION
  CONFIG_DEFAULT_LOCATION = $(OMNIORB_CONFIG_DEFAULT_LOCATION)
else
  ifdef UnixPlatform
    CONFIG_DEFAULT_LOCATION = /etc/omniORB.cfg
  endif
  ifdef Win32Platform
    CONFIG_DEFAULT_LOCATION = C:\\OMNIORB.CFG
  endif
  ifdef vxWorksPlatform
    CONFIG_DEFAULT_LOCATION = /a2/tmp/omniORB.cfg
  endif
endif
DIR_CPPFLAGS += -DCONFIG_DEFAULT_LOCATION='"$(CONFIG_DEFAULT_LOCATION)"'

##########################################################################
ifdef OMNIORB_CONFIG_ENV
  CONFIG_ENV = $(OMNIORB_CONFIG_ENV)
else
  CONFIG_ENV = OMNIORB_CONFIG
endif
DIR_CPPFLAGS += -DCONFIG_ENV='"$(CONFIG_ENV)"'

##########################################################################
ifdef UnixPlatform
#  CXXDEBUGFLAGS = -g
  DIR_CPPFLAGS += -DUnixArchitecture
  ifdef SunOS
    DIR_CPPFLAGS += -DBSD_COMP   # include BSD flags in ioctl.h
  endif
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture"
  EXTRA_LIBS    = $(SOCKET_LIB) $(patsubst %,$(LibNoDebugSearchPattern),advapi32)
  SHARED_ONLY_OBJS = msvcdllstub.o
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
ifndef ETSKernel
  ORB_SRCS += orbOptionsReg.cc
endif
endif

#########################################################################

BASE_OBJS     = $(BASE_SRCS:.cc=.o)
CXXSRCS       = $(BASE_SRCS)

vpath %.cc $(CXXVPATH)

LIB_NAME     := omniBase
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(BASE_OBJS)
LIB_IMPORTS  := $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk

#########################################################################
ifdef Win32Platform

stublib = static/$(patsubst %,$(LibNoDebugPattern),msvcstub)

all:: $(stublib)

$(stublib): static/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stublib)
	@$(ExportLibrary)

clean::
	$(RM) $(stublib)

veryclean::
	$(RM) $(stublib)

stubdblib = debug/$(patsubst %,$(LibDebugPattern),msvcstub)

all:: $(stubdblib)

$(stubdblib): debug/msvcdllstub.o
	@$(StaticLinkLibrary)

export:: $(stubdblib)
	@$(ExportLibrary)

clean::
	$(RM) $(stubdblib)

veryclean::
	$(RM) $(stubdblib)

endif
