// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

#ifndef MXOSIM_SOCKETS_H
#define MXOSIM_SOCKETS_H

#if PLATFORM == PLATFORM_WIN32
    #include <winsock2.h>
	#ifndef EWOULDBLOCK
    #define EWOULDBLOCK WSAEWOULDBLOCK
	#endif
#else // Berkley
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/ioctl.h>
    #include <sys/types.h>
    #include <string.h>
    #include <sys/time.h>
	#include <errno.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
//    #define FD_SET fd_set
#endif

#endif
