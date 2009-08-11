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

#ifndef MXOSIM_CONFIG_H
#define MXOSIM_CONFIG_H

#include "Singleton.h"
#include <string>

class Config : public Singleton<Config>
{
    public:
        Config();
        ~Config();

        bool SetSource(const char *file, bool ignorecase = true);

		bool GetString(const char* name, std::string *value);
		std::string GetStringDefault(const char* name, const char* def);

        bool GetBool(const char* name, bool *value);
        bool GetBoolDefault(const char* name, const bool def = false);

        bool GetInt(const char* name, int *value);
        int GetIntDefault(const char* name, const int def);

        bool GetFloat(const char* name, float *value);
        float GetFloatDefault(const char* name, const float def);

    private:
        class DOTCONFDocument *mConf;
};

#define sConfig Config::getSingleton()
#endif
