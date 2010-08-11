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

#ifndef MXOSIM_MASTER_H
#define MXOSIM_MASTER_H

#include "Singleton.h"

class Master : public Singleton<Master>
{
    public:
        Master();
        ~Master();
        bool Run();

        static volatile bool m_stopEvent;
    private:
		bool _StartDB();
		void _StopDB();

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
};

#endif