# Microsoft Developer Studio Project File - Name="hotkeyp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=HOTKEYP - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hotkeyp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hotkeyp.mak" CFG="HOTKEYP - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hotkeyp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "hotkeyp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "hotkeyp - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\Release"
# PROP Intermediate_Dir "obj\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /Yu"hdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"./HotkeyP98.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeypdbg.exe -send window command 203 C:\_Petr\CW\hotkeyp\hotkeyp.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj\Debug"
# PROP Intermediate_Dir "obj\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 oleaut32.lib htmlhelp.lib version.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./hotkeypDBG.exe" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -send window command 203 PlasHotKey
# End Special Build Tool

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "hotkeyp___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "hotkeyp___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\Release_Unicode"
# PROP Intermediate_Dir "obj\Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /Yu"hdr.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /Yu"hdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"./HotkeyP.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"./HotkeyP.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeypdbg.exe -send window command 203 C:\_Petr\CW\hotkeyp\hotkeyp.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "hotkeyp - Win32 Release"
# Name "hotkeyp - Win32 Debug"
# Name "hotkeyp - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\abortable.cpp
# End Source File
# Begin Source File

SOURCE=.\source\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\source\encrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\source\help.cpp
# End Source File
# Begin Source File

SOURCE=.\source\HOTKEYP.cpp
# End Source File
# Begin Source File

SOURCE=.\source\hotkeyp.rc
# End Source File
# Begin Source File

SOURCE=.\source\joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\source\keys.cpp
# End Source File
# Begin Source File

SOURCE=.\source\lang.cpp
# End Source File
# Begin Source File

SOURCE=.\source\rmdrive.cpp
# End Source File
# Begin Source File

SOURCE=.\source\trayicon.cpp
# ADD CPP /Yc"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\utl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\volume.cpp
# End Source File
# Begin Source File

SOURCE=.\source\winlirc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\hdr.h
# End Source File
# Begin Source File

SOURCE=.\source\hotkeyp.h
# End Source File
# Begin Source File

SOURCE=.\source\lang.h
# End Source File
# Begin Source File

SOURCE=.\source\trayicon.h
# End Source File
# Begin Source File

SOURCE=.\source\vistavol.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\source\ascdesc.bmp
# End Source File
# Begin Source File

SOURCE=.\source\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\source\down.bmp
# End Source File
# Begin Source File

SOURCE=.\source\down.ico
# End Source File
# Begin Source File

SOURCE=.\source\ico5.ico
# End Source File
# Begin Source File

SOURCE=.\source\ico6.ico
# End Source File
# Begin Source File

SOURCE=.\source\ico7.ico
# End Source File
# Begin Source File

SOURCE=.\source\ico8.ico
# End Source File
# Begin Source File

SOURCE=.\source\ico9.ico
# End Source File
# Begin Source File

SOURCE=.\source\keys.ico
# End Source File
# Begin Source File

SOURCE=.\source\keys2.ico
# End Source File
# Begin Source File

SOURCE=.\source\up.bmp
# End Source File
# Begin Source File

SOURCE=.\source\up.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\HotkeyP.exe.manifest
# End Source File
# End Target
# End Project
