# Microsoft Developer Studio Project File - Name="tests" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tests - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tests.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tests.mak" CFG="tests - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tests - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tests - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tests - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../c-sdk-wdeps/include" /I "../CUnit/CUnit/Headers" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QINIU_EXPORTS" /FR /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../c-sdk-wdeps/lib"

!ELSEIF  "$(CFG)" == "tests - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../c-sdk-wdeps/include" /I "../CUnit/CUnit/Headers" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QINIU_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../c-sdk-wdeps/lib"

!ENDIF 

# Begin Target

# Name "tests - Win32 Release"
# Name "tests - Win32 Debug"
# Begin Group "qiniu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\qiniu\auth_mac.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\base.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\base.h
# End Source File
# Begin Source File

SOURCE=..\qiniu\base_io.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\conf.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\conf.h
# End Source File
# Begin Source File

SOURCE=..\qiniu\http.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\http.h
# End Source File
# Begin Source File

SOURCE=..\qiniu\io.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\io.h
# End Source File
# Begin Source File

SOURCE=..\qiniu\resumable_io.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\resumable_io.h
# End Source File
# Begin Source File

SOURCE=..\qiniu\rs.c
# End Source File
# Begin Source File

SOURCE=..\qiniu\rs.h
# End Source File
# End Group
# Begin Group "deps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\b64\b64.h
# End Source File
# Begin Source File

SOURCE=..\cJSON\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\cJSON\cJSON.h
# End Source File
# Begin Source File

SOURCE="..\..\c-sdk-wdeps\emu-posix\emu_posix.c"
# End Source File
# Begin Source File

SOURCE="..\..\c-sdk-wdeps\emu-posix\emu_posix.h"
# End Source File
# Begin Source File

SOURCE=..\b64\urlsafe_b64.c
# End Source File
# End Group
# Begin Group "cunit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Basic\Basic.c
# End Source File
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Framework\CUError.c
# End Source File
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Framework\MyMem.c
# End Source File
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Framework\TestDB.c
# End Source File
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Framework\TestRun.c
# End Source File
# Begin Source File

SOURCE=..\CUnit\CUnit\Sources\Framework\Util.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\equal.c
# End Source File
# Begin Source File

SOURCE=.\seq.c
# End Source File
# Begin Source File

SOURCE=.\test.c
# End Source File
# Begin Source File

SOURCE=.\test.h
# End Source File
# Begin Source File

SOURCE=.\test_base_io.c
# End Source File
# Begin Source File

SOURCE=.\test_fmt.c
# End Source File
# Begin Source File

SOURCE=.\test_io_put.c
# End Source File
# Begin Source File

SOURCE=.\test_resumable_io.c
# End Source File
# Begin Source File

SOURCE=.\test_rs_ops.c
# End Source File
# End Target
# End Project
