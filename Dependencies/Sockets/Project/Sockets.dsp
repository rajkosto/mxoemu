# Microsoft Developer Studio Project File - Name="Sockets" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Sockets - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sockets.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sockets.mak" CFG="Sockets - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sockets - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Sockets - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sockets - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\R"
# PROP Intermediate_Dir "R"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "c:\openssl\include;c:\libxml2\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy ..\*.h Include /I /Y
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Sockets - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\D"
# PROP Intermediate_Dir "D"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "c:\openssl\include;c:\libxml2\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy ..\*.h Include /I /Y
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Sockets - Win32 Release"
# Name "Sockets - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\socket_include.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Exception.h
# End Source File
# Begin Source File

SOURCE=..\socket_include.h
# End Source File
# Begin Source File

SOURCE="..\sockets-config.h"
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Base64.cpp
# End Source File
# Begin Source File

SOURCE=..\Base64.h
# End Source File
# Begin Source File

SOURCE=..\Debug.cpp
# End Source File
# Begin Source File

SOURCE=..\Debug.h
# End Source File
# Begin Source File

SOURCE=..\Parse.cpp
# End Source File
# Begin Source File

SOURCE=..\Parse.h
# End Source File
# Begin Source File

SOURCE=..\Utility.cpp
# End Source File
# Begin Source File

SOURCE=..\Utility.h
# End Source File
# End Group
# Begin Group "Internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SSLInitializer.cpp
# End Source File
# Begin Source File

SOURCE=..\SSLInitializer.h
# End Source File
# End Group
# Begin Group "Webserver framework"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ajp13.h
# End Source File
# Begin Source File

SOURCE=..\Ajp13Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\Ajp13Socket.h
# End Source File
# Begin Source File

SOURCE=..\AjpBaseSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\AjpBaseSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpBaseSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpBaseSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpdCookies.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpdCookies.h
# End Source File
# Begin Source File

SOURCE=..\HttpdForm.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpdForm.h
# End Source File
# Begin Source File

SOURCE=..\HttpdSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpdSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpRequest.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpRequest.h
# End Source File
# Begin Source File

SOURCE=..\HttpResponse.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpResponse.h
# End Source File
# Begin Source File

SOURCE=..\HttpTransaction.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpTransaction.h
# End Source File
# Begin Source File

SOURCE=..\IHttpServer.h
# End Source File
# End Group
# Begin Group "HTTP Sockets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\HttpClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpClientSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpDebugSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpDebugSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpGetSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpGetSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpPostSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpPostSocket.h
# End Source File
# Begin Source File

SOURCE=..\HttpPutSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HttpPutSocket.h
# End Source File
# Begin Source File

SOURCE=..\HTTPSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\HTTPSocket.h
# End Source File
# End Group
# Begin Group "File handling"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\File.cpp
# End Source File
# Begin Source File

SOURCE=..\File.h
# End Source File
# Begin Source File

SOURCE=..\IFile.h
# End Source File
# Begin Source File

SOURCE=..\MemFile.cpp
# End Source File
# Begin Source File

SOURCE=..\MemFile.h
# End Source File
# End Group
# Begin Group "Asynchronous DNS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ResolvServer.cpp
# End Source File
# Begin Source File

SOURCE=..\ResolvServer.h
# End Source File
# Begin Source File

SOURCE=..\ResolvSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\ResolvSocket.h
# End Source File
# End Group
# Begin Group "Basic Sockets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ipv4Address.cpp
# End Source File
# Begin Source File

SOURCE=..\Ipv4Address.h
# End Source File
# Begin Source File

SOURCE=..\Ipv6Address.cpp
# End Source File
# Begin Source File

SOURCE=..\Ipv6Address.h
# End Source File
# Begin Source File

SOURCE=..\ISocketHandler.h
# End Source File
# Begin Source File

SOURCE=..\ListenSocket.h
# End Source File
# Begin Source File

SOURCE=..\SctpSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\SctpSocket.h
# End Source File
# Begin Source File

SOURCE=..\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\Socket.h
# End Source File
# Begin Source File

SOURCE=..\SocketAddress.h
# End Source File
# Begin Source File

SOURCE=..\SocketHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\SocketHandler.h
# End Source File
# Begin Source File

SOURCE=..\StreamSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\StreamSocket.h
# End Source File
# Begin Source File

SOURCE=..\TcpSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\TcpSocket.h
# End Source File
# Begin Source File

SOURCE=..\UdpSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\UdpSocket.h
# End Source File
# End Group
# Begin Group "Log help classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\StdLog.h
# End Source File
# Begin Source File

SOURCE=..\StdoutLog.cpp
# End Source File
# Begin Source File

SOURCE=..\StdoutLog.h
# End Source File
# End Group
# Begin Group "Threading"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Lock.cpp
# End Source File
# Begin Source File

SOURCE=..\Lock.h
# End Source File
# Begin Source File

SOURCE=..\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\Thread.h
# End Source File
# End Group
# Begin Group "Timer Events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Event.cpp
# End Source File
# Begin Source File

SOURCE=..\Event.h
# End Source File
# Begin Source File

SOURCE=..\EventHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\EventHandler.h
# End Source File
# Begin Source File

SOURCE=..\EventTime.cpp
# End Source File
# Begin Source File

SOURCE=..\EventTime.h
# End Source File
# Begin Source File

SOURCE=..\IEventHandler.h
# End Source File
# Begin Source File

SOURCE=..\IEventOwner.cpp
# End Source File
# Begin Source File

SOURCE=..\IEventOwner.h
# End Source File
# End Group
# Begin Group "SMTP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SmtpdSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\SmtpdSocket.h
# End Source File
# End Group
# Begin Group "Xml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\XmlDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\XmlDocument.h
# End Source File
# Begin Source File

SOURCE=..\XmlException.cpp
# End Source File
# Begin Source File

SOURCE=..\XmlException.h
# End Source File
# Begin Source File

SOURCE=..\XmlNode.cpp
# End Source File
# Begin Source File

SOURCE=..\XmlNode.h
# End Source File
# End Group
# End Target
# End Project
