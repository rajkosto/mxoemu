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

#include "Common.h"
#include "Config.h"
#include "DotConfPP/dotconfpp.h"

createFileSingleton(Config);

Config::Config() : mConf(0)
{
}


Config::~Config()
{
    if (mConf)
        delete mConf;
}


bool Config::SetSource(const char *file, bool ignorecase)
{
    mConf = new DOTCONFDocument(ignorecase ?
        DOTCONFDocument::CASEINSENSETIVE :
    DOTCONFDocument::CASESENSETIVE);

    if (mConf->setContent(file) == -1)
    {
        delete mConf;
        mConf = 0;
        return false;
    }

    return true;
}


bool Config::GetString(const char* name, std::string *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    *value = node->getValue();

    return true;
}


std::string Config::GetStringDefault(const char* name, const char* def)
{
    if(!mConf)
		return std::string(def);

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
		return std::string(def);

	return std::string(node->getValue());
}


bool Config::GetBool(const char* name, bool *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    const char* str = node->getValue();
    if(strcmp(str, "true") == 0 || strcmp(str, "TRUE") == 0 ||
        strcmp(str, "yes") == 0 || strcmp(str, "YES") == 0 ||
        strcmp(str, "1") == 0)
    {
        *value = true;
    }
    else
        *value = false;

    return true;
}


bool Config::GetBoolDefault(const char* name, const bool def)
{
    bool val;
    return GetBool(name, &val) ? val : def;
}


bool Config::GetInt(const char* name, int *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    *value = atoi(node->getValue());

    return true;
}


bool Config::GetFloat(const char* name, float *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    *value = (float)atof(node->getValue());

    return true;
}


int Config::GetIntDefault(const char* name, const int def)
{
    int val;
    return GetInt(name, &val) ? val : def;
}


float Config::GetFloatDefault(const char* name, const float def)
{
    float val;
    return (GetFloat(name, &val) ? val : def);
}
