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

#ifndef MXOSIM_MARGINHANDLER_H
#define MXOSIM_MARGINHANDLER_H

#include <Sockets/SocketHandler.h>
#include "Common.h"

class MarginHandler : public SocketHandler
{
public:
	MarginHandler();
	~MarginHandler();

	vector<class MarginSocket*> FindByCharacterUID(uint64 charUID);
	class MarginSocket *FindBySessionId(uint32 sessionId);
};

#endif // _MARGINHANDLER_H
