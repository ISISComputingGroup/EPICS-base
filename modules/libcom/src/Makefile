#*************************************************************************
# Copyright (c) 2010 UChicago Argonne LLC, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE is distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution.
#*************************************************************************
LIBCOM := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
TOP = ../../..

include $(TOP)/configure/CONFIG

# Uncomment this to remove the (benign) valgrind helper stubs
#USR_CFLAGS += -DNVALGRIND

INC += valgrind/valgrind.h

INC += libComVersion.h
INC += libComVersionNum.h

include $(LIBCOM)/as/Makefile
include $(LIBCOM)/bucketLib/Makefile
include $(LIBCOM)/calc/Makefile
include $(LIBCOM)/cvtFast/Makefile
include $(LIBCOM)/cppStd/Makefile
include $(LIBCOM)/cxxTemplates/Makefile
include $(LIBCOM)/dbmf/Makefile
include $(LIBCOM)/ellLib/Makefile
include $(LIBCOM)/env/Makefile
include $(LIBCOM)/error/Makefile
include $(LIBCOM)/fdmgr/Makefile
include $(LIBCOM)/flex/Makefile
include $(LIBCOM)/freeList/Makefile
include $(LIBCOM)/gpHash/Makefile
include $(LIBCOM)/iocsh/Makefile
include $(LIBCOM)/log/Makefile
include $(LIBCOM)/macLib/Makefile
include $(LIBCOM)/misc/Makefile
include $(LIBCOM)/osi/Makefile
include $(LIBCOM)/pool/Makefile
include $(LIBCOM)/ring/Makefile
include $(LIBCOM)/taskwd/Makefile
include $(LIBCOM)/timer/Makefile
include $(LIBCOM)/yacc/Makefile
include $(LIBCOM)/yajl/Makefile

# Library to build:
LIBRARY=Com

ifeq ($(COMMANDLINE_LIBRARY),WINEDITLINE)
Com_LIBS_WIN32 = WinEditLine
endif

Com_SYS_LIBS_WIN32 = ws2_32 advapi32 user32 dbghelp

Com_RCS = Com.rc

ifeq ($(T_A),$(EPICS_HOST_ARCH))
  # Antelope & flex are needed to finish libCom
  DELAY_INSTALL_LIBS = YES
endif

EXPANDVARS += EPICS_LIBCOM_MAJOR_VERSION
EXPANDVARS += EPICS_LIBCOM_MINOR_VERSION
EXPANDVARS += EPICS_LIBCOM_MAINTENANCE_VERSION
EXPANDVARS += EPICS_LIBCOM_DEVELOPMENT_FLAG

EXPANDFLAGS += $(foreach var,$(EXPANDVARS),-D$(var)="$(strip $($(var)))")

# shared library ABI version.
SHRLIB_VERSION = $(EPICS_LIBCOM_MAJOR_VERSION).$(EPICS_LIBCOM_MINOR_VERSION).$(EPICS_LIBCOM_MAINTENANCE_VERSION)

#   libs needed for PROD and TESTPRODUCT
#PROD_LIBS = Com
#ifeq ($(COMMANDLINE_LIBRARY),WINEDITLINE)
#PROD_LIBS_WIN32 = WinEditLine
#endif
#PROD_SYS_LIBS_WIN32 = ws2_32 advapi32

include $(TOP)/configure/RULES

include $(LIBCOM)/as/RULES
include $(LIBCOM)/env/RULES
include $(LIBCOM)/error/RULES
include $(LIBCOM)/flex/RULES
include $(LIBCOM)/misc/RULES
include $(LIBCOM)/osi/RULES
include $(LIBCOM)/yajl/RULES

# Can't use EXPAND as generated headers must appear
# in O.Common, but EXPAND emits rules for O.$(T_A)
../O.Common/libComVersionNum.h: ../libComVersionNum.h@
	$(EXPAND_TOOL) $(EXPANDFLAGS) $($@_EXPANDFLAGS) $< $@
