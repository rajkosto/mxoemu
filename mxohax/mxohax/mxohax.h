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
// mxohax.h
//

#ifndef MXOHAX_MXOHAX_H
#define MXOHAX_MXOHAX_H

#define SEND_KEY_PIPE_NAME  "\\\\.\\pipe\\MxoTwofishKey"
#define DLL_NAME            "mxohax.dll"
#define TWOFISH_KEY_LENGTH  0x10

typedef void* (__stdcall *GetKey_) ();
typedef void (__stdcall *CBCEncryption_ProcessBlocks_) ();
typedef void (__stdcall *CBCDecryption_ProcessBlocks_) ();
typedef bool (__stdcall *VerifyMessage_) ();
typedef void (__stdcall *HashUpdate_) ();
typedef void* (__stdcall *GetMaterial_) ();
typedef void* (__stdcall *PK_Decryptor_Decrypt_) ();

#endif // MXOHAX_MXOHAX_H

//
///////////////////////////////////////////////////////////////////////////////