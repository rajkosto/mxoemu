// *************************************************************************************************
// --------------------------------------
// Copyright (C) 2006-2010 Rajko Stojadinovic
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

///////////////////////////////////////////////////////////////////////////////
//
// loader.cpp
//

#include <windows.h>

#include "..\detours\include\detours.h"

#include "loader.h"


///////////////////////////////////////////////////////////////////////////////
//
// main()
//
int main (int argc, char *argv[])
{
    STARTUPINFO sui;
    PROCESS_INFORMATION pi;

    sui.cb = sizeof (sui);
    sui.lpReserved = NULL;
    sui.lpDesktop = NULL;
    sui.lpTitle = NULL;
    sui.cbReserved2 = 0;
    sui.lpReserved2 = NULL;

    pi.hProcess = NULL;
    pi.hThread = NULL;
    pi.dwProcessId = 0;
    pi.dwThreadId = 0;

    BOOL bProcessStarted =
        DetourCreateProcessWithDll (MATRIX_EXE_NAME, "-nopatch -clone", NULL, NULL, FALSE,
                                    0, NULL, NULL, &sui, &pi,
                                    DETOURS_DLL_NAME, DLL_NAME, NULL);

    if (FALSE == bProcessStarted)
    {
        TCHAR msgBuffer[0x100];
        wsprintf (msgBuffer, "Failed to start %s", MATRIX_EXE_NAME);
        MessageBox (NULL, msgBuffer, EXE_NAME, MB_OK);
        return (-1);
    }

    return (0);
}

//
///////////////////////////////////////////////////////////////////////////////