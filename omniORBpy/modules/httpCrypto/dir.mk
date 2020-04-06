include $(BASE_OMNI_TREE)/mk/python.mk

DIR_CPPFLAGS += -DOMNIPY_MAJOR=$(OMNIPY_MAJOR) -DOMNIPY_MINOR=$(OMNIPY_MINOR)
DIR_CPPFLAGS += -DOMNIORB_VERSION_STRING=\"$(OMNIORB_VERSION)\"

CXXSRCS = pyhttpCrypto.cc

OBJS =    pyhttpCrypto.o

DIR_CPPFLAGS += $(patsubst %,-I%/include,$(OMNIORB_ROOT))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(OMNIORB_ROOT))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += -I../../include
DIR_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)


#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################

ifdef UnixPlatform
#CXXDEBUGFLAGS = -g

DIR_CPPFLAGS += $(CORBA_CPPFLAGS)
endif

ifdef Cygwin
extralibs += -lomniORB4 -lomnithread -lpthread
endif

#############################################################################
#   Make rules for Autoconf builds                                          #
#############################################################################

ifeq ($(platform),autoconf)

namespec := _omnihttpCrypto$(PY_MODULE_SUFFIX) _ $(OMNIPY_MAJOR) $(OMNIPY_MINOR)

ifdef PythonSHAREDLIB_SUFFIX
SHAREDLIB_SUFFIX = $(PythonSHAREDLIB_SUFFIX)
endif

SharedLibraryFullNameTemplate = $$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   = $$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  = $$1$$2.$(SHAREDLIB_SUFFIX)

ifdef PythonLibraryPlatformLinkFlagsTemplate
SharedLibraryPlatformLinkFlagsTemplate = $(PythonLibraryPlatformLinkFlagsTemplate)
endif

shlib := $(shell $(SharedLibraryFullName) $(namespec))

DIR_CPPFLAGS += $(SHAREDLIB_CPPFLAGS)

$(shlib): $(OBJS)
	@(namespec="$(namespec)"; extralibs="$(OMNIORB_HTTP_CRYPTO_LIB) $(extralibs)";\
          $(MakeCXXSharedLibrary))

all:: $(shlib)

export:: $(shlib)
	@(namespec="$(namespec)"; $(ExportSharedLibrary))

ifdef INSTALLTARGET
install:: $(shlib)
	@(dir="$(INSTALLPYEXECDIR)"; namespec="$(namespec)"; \
          $(ExportSharedLibraryToDir))
endif

clean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

veryclean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

else

#############################################################################
#   Make rules for Windows                                                  #
#############################################################################

ifdef Win32Platform

PYPREFIX1 := "$(shell $(PYTHON) -c 'import sys,string; sys.stdout.write(sys.prefix.lower())')"
PYPREFIX  := $(subst program files,progra~1,$(subst \,/,$(PYPREFIX1)))
PYVERSION := $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.version[:3])')
PYINCDIR  := $(PYPREFIX)/include
PYLIBDIR  := $(PYPREFIX)/libs
PYLIB     := python$(subst .,,$(PYVERSION)).lib

DIR_CPPFLAGS += -I$(PYINCDIR) -I$(PYINCDIR)/python$(PYVERSION) \
                -DPYTHON_INCLUDE="<Python.h>" -DPYTHON_THREAD_INC="<pythread.h>"

PYLIBPATH = $(patsubst %,-libpath:%,$(PYLIBDIR))

implib = _omnihttpCrypto.lib
lib = $(patsubst %.lib,%.pyd,$(implib))

all:: $(lib)

$(lib): $(OBJS)
	(set -x; \
	 $(RM) $@; \
	 libs="$(OMNIORB_HTTP_CRYPTO_LIB) $(OMNIORB_LIB_NODYN) $(PYLIB)"; \
	 $(CXXLINK) -out:$@ -DLL $(CXXLINKOPTIONS) $(IMPORT_LIBRARY_FLAGS) $(PYLIBPATH) $(OBJS) $$libs; \
         $(MANIFESTTOOL) /outputresource:"$@;#2" /manifest $@.manifest; \
	)

export:: $(lib)
	@$(ExportLibrary)


endif

endif
