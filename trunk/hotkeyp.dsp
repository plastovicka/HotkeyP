# Microsoft Developer Studio Project File - Name="hotkeyp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=hotkeyp - Win32 Debug no hook
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hotkeyp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hotkeyp.mak" CFG="hotkeyp - Win32 Debug no hook"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hotkeyp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "hotkeyp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "hotkeyp - Win32 Debug no hook" (based on "Win32 (x86) Application")
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
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /O1 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x405 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 htmlhelp.lib version.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:"./HotkeyP.exe"
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
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
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

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "hotkeyp___Win32_Debug_no_hook"
# PROP BASE Intermediate_Dir "hotkeyp___Win32_Debug_no_hook"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "hotkeyp___Win32_Debug_no_hook"
# PROP Intermediate_Dir "hotkeyp___Win32_Debug_no_hook"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /D "NOHOOK" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x405 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 version.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./hotkeypDBG.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 oleaut32.lib htmlhelp.lib version.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./hotkeypDBG_NHK.exe" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Cmds=c:\_Petr\cw\hotkeyp\hotkeyp.exe -send window command 203 PlasHotKey
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "hotkeyp - Win32 Release"
# Name "hotkeyp - Win32 Debug"
# Name "hotkeyp - Win32 Debug no hook"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\commands.cpp

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# ADD CPP /MD /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

# ADD CPP /Yu"hdr.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\encrypt.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\help.cpp

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\HOTKEYP.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\hotkeyp.rc
# End Source File
# Begin Source File

SOURCE=.\source\joystick.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\keys.cpp

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\lang.cpp
# ADD CPP /Yu"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\rmdrive.cpp

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\trayicon.cpp
# ADD CPP /Yc"hdr.h"
# End Source File
# Begin Source File

SOURCE=.\source\volume.cpp

!IF  "$(CFG)" == "hotkeyp - Win32 Release"

# ADD CPP /Yu"hdr.h"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug"

!ELSEIF  "$(CFG)" == "hotkeyp - Win32 Debug no hook"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\source\winlirc.cpp
# ADD CPP /Yu"hdr.h"
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
# End Target
# End Project
