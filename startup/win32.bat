@ECHO OFF
REM *************************************************************************
REM  Copyright (c) 2002 The University of Chicago, as Operator of Argonne
REM      National Laboratory.
REM  Copyright (c) 2002 The Regents of the University of California, as
REM      Operator of Los Alamos National Laboratory.
REM  EPICS BASE Versions 3.13.7
REM  and higher are distributed subject to a Software License Agreement found
REM  in file LICENSE that is included with this distribution.
REM *************************************************************************
REM  Revision-Id: jba@aps.anl.gov-20110419162804-n948b5t79d7ctygo
REM
REM  Site-specific EPICS environment settings
REM 
REM  sites should modify these definitions

REM ======================================================
REM    ====== REQUIRED ENVIRONMENT VARIABLES FOLLOW ======
REM ======================================================

REM ======================================================
REM   ---------------- WINDOWS ---------------------------
REM ======================================================
REM ----- WIN95 -----
REM set PATH=C:\WINDOWS;C:\WINDOWS\COMMAND
REM ----- WINNT, WIN2000  -----
REM set PATH=C:\WINNT;C:\WINNT\SYSTEM32
REM ----- WINXP, Vista, Windows 7 -----

REM leave base path alone!
REM set PATH=C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\SYSTEM32\Wbem

REM ======================================================
REM   ---------------- make and perl ---------------------
REM ======================================================

REM   --------------- ActiveState perl -------------------
set PATH=C:\Perl\bin;%PATH%

REM    --------------- mingw make ------------------------
REM set PATH=C:\mingw-make\bin;%PATH%
REM set PATH=C:\mingw-make82-3\bin;%PATH%

REM   --------------- gnuwin32 make ----------------------
set PATH=C:\gnuwin32\bin;%PATH%

REM ======================================================
REM ---------------- cygwin tools ------------------------
REM ======================================================
REM    (make & perl if above perl and make are REMs)
REM    Dont use cygwin GNU make and Perl!  
REM    cygwin contains tk/tcl, vim, perl, and many unix tools
REM    need grep from here NOT from cvs directory
REM set PATH=%PATH%;.;..
REM set PATH=%PATH%;c:\cygwin\bin

REM ======================================================
REM --------------- EPICS 1--------------------------------
REM ======================================================
if "%EPICS_HOST_ARCH%" == "" (
    set EPICS_HOST_ARCH=windows-x64
REM set EPICS_HOST_ARCH=windows-x64-debug
REM set EPICS_HOST_ARCH=win32-x86
REM set EPICS_HOST_ARCH=win32-x86-debug
)

REM ======================================================
REM   --------------- Visual c++ -------------------------
REM ======================================================

REM    ------ Microsoft Visual Studio 2005 ------
REM call "C:\Program files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86_amd64
REM set PATH=%PATH%;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin
REM set INCLUDE=%INCLUDE%;C:\Program Files\Microsoft SDKs\Windows\v6.0A\include
REM REM set LIBPATH=%LIBPATH%;C:\Program Files\Microsoft SDKs\Windows\v6.0A\lib
REM set LIB=%LIB%;C:\Program Files\Microsoft SDKs\Windows\v6.0A\lib

REM    ------ Microsoft Visual Studio 2008 ------
REM call "C:\Program files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat"
REM call "C:\Program files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
REM set    PATH=C:\Program Files\Microsoft SDKs\Windows\v7.0\bin;%PATH%
REM set INCLUDE=C:\Program Files\Microsoft SDKs\Windows\v7.0\include;%INCLUDE%
REM set LIBPATH=C:\Program Files\Microsoft SDKs\Windows\v7.0\lib;%LIBPATH%
REM set     LIB=C:\Program Files\Microsoft SDKs\Windows\v7.0\lib;%LIB%

REM    ----- Prefer Visual Studio 2010, otherwise most recent version -----
if exist "C:\Program files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" set VCVERSION=11.0
if exist "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" set VCVERSION=12.0
if exist "C:\Program files (x86)\Microsoft Visual Studio 13.0\VC\vcvarsall.bat" set VCVERSION=13.0
if exist "C:\Program files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" set VCVERSION=14.0
if exist "C:\Program files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" set VCVERSION=10.0

if exist "C:\Program files (x86)\Microsoft Visual Studio %VCVERSION%\VC\vcvarsall.bat" (
  if "%EPICS_HOST_ARCH:~0,11%" == "windows-x64" (
REM -- express 2012 provides a 32->64 cross compiler, the full visual studio has both a cross and native compiler
    if exist "C:\Program files (x86)\Microsoft Visual Studio %VCVERSION%\VC\bin\amd64" (
	    @echo Using Visual Studio %VCVERSION% x64 native compiler
        call "C:\Program files (x86)\Microsoft Visual Studio %VCVERSION%\VC\vcvarsall.bat" x64
	) else (
	    @echo Using Visual Studio %VCVERSION% x64 cross compiler
        call "C:\Program files (x86)\Microsoft Visual Studio %VCVERSION%\VC\vcvarsall.bat" x86_amd64
	)
  )
  if "%EPICS_HOST_ARCH:~0,9%" == "win32-x86" (
	 @echo Using Visual Studio %VCVERSION% x86 native compiler
     call "C:\Program files (x86)\Microsoft Visual Studio %VCVERSION%\VC\vcvarsall.bat" x86
  )
)

REM ======================================================
REM --------------- EPICS 2--------------------------------
REM ======================================================
REM set PATH=%PATH%;G:\epics\base\bin\%EPICS_HOST_ARCH%
REM set PATH=%PATH%;G:\epics\extensions\bin\%EPICS_HOST_ARCH%

REM ======================================================
REM ------- OPTIONAL ENVIRONMENT VARIABLES FOLLOW --------
REM ======================================================

REM ======================================================
REM ----------------- remote CVS -------------------------
REM ======================================================
REM set CVS_RSH=c:/cygwin/bin/ssh.exe
REM set CVSROOT=:ext:jba@aps.anl.gov:/usr/local/epicsmgr/cvsroot
REM set HOME=c:/users/%USERNAME%
REM set HOME=c:/users/jba

REM ======================================================
REM ------------------- Bazaar ---------------------------
REM ======================================================
set PATH=%PATH%;C:\Program files\Bazaar

REM ======================================================
REM ----------------- GNU make flags ---------------------
REM ======================================================
set MAKEFLAGS=-w

REM ======================================================
REM -------------- vim (use cygwin vim ) -----------------
REM ======================================================
REM HOME needed by vim to write .viminfo file.
REM VIM needed by vim to find _vimrc file.
REM set VIM=c:\cygwin

REM ======================================================
REM --------------- Epics Channel Access -----------------
REM    Modify and uncomment the following lines
REM    to override the base/configure/CONFIG_ENV defaults
REM ======================================================
REM set EPICS_CA_ADDR_LIST=n.n.n.n  n.n.n.n
REM set EPICS_CA_AUTO_ADDR_LIST=YES

REM set EPICS_CA_CONN_TMO=30.0
REM set EPICS_CA_BEACON_PERIOD=15.0
REM set EPICS_CA_REPEATER_PORT=5065
REM set EPICS_CA_SERVER_PORT=5064
REM set EPICS_TS_MIN_WEST=420

REM ======================================================
REM --------------- JAVA ---------------------------------
REM ======================================================
REM    Needed for java extensions
REM set CLASSPATH=G:\epics\extensions\javalib
REM set PATH=%PATH%;C:\j2sdk1.4.1_01\bin
REM set CLASSPATH=%CLASSPATH%;C:\j2sdk1.4.1_01\lib\tools.jar

REM ======================================================
REM --------------- Exceed -------------------------------
REM    Needed for X11 extensions
REM ======================================================
REM set EX_VER=7.10
REM set EX_VER=12.00
REM set EX_VER=14.00
REM set PATH=%PATH%;C:\Exceed%EX_VER%\XDK\
REM set PATH=%PATH%;C:\Program Files\Hummingbird\Connectivity\%EX_VER%\Exceed\

