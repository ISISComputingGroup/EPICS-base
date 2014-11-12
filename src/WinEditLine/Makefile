TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += WinEditLine

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install WinEditLine.dbd into <top>/dbd
INC += editline/readline.h

# specify all source files to be compiled and added to the library
WinEditLine_SRCS += editline.c fn_complete.c history.c

USR_CFLAGS += -I$(TOP)/src/libCom/misc

WinEditLine_SYS_LIBS_WIN32 += user32
#WinEditLine_LDFLAGS_WIN32 += /def:$(TOP)/src/WinEditLine/libedit.def

PROD_IOC += libedit_test
libedit_test_SRCS += libedit_test.c
libedit_test_LIBS += WinEditLine
libedit_test_SYS_LIBS_WIN32 += user32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

