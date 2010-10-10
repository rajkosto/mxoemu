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
#include "Log.h"
#include "ConsoleThread.h"
#include "Util.h"
#include "Master.h"
#include "Crypto.h"
#include "GameServer.h"
#include "AuthServer.h"

#include <boost/algorithm/string.hpp>
using boost::iequals;

bool ConsoleThread::run()
{
	SetThreadName("Console Thread");

	for (;;) 
	{
		string command;
		cin >> command;

		if (iequals(command, "exit"))
		{
			Master::m_stopEvent = true;
			INFO_LOG("Got exit command. Shutting down...");
			break;
		}
		else if (iequals(command,"register"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string username,password;
			lineParser >> username;
			lineParser >> password;

			if (username.length() < 1 || password.length() < 1)
			{
				WARNING_LOG("Invalid username or password");
			}
			else
			{
				bool accountCreated = sAuth.CreateAccount(username,password);
				if (accountCreated)
					INFO_LOG(format("Created account with username %1% password %2%") % username % password );
			}
		}
		else if (iequals(command,"changePassword"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string username,newPassword;
			lineParser >> username;
			lineParser >> newPassword;

			bool passwordChanged = sAuth.ChangePassword(username,newPassword);
			if (passwordChanged)
				INFO_LOG(format("Account %1% now has password %2%") % username % newPassword );
		}
		else if (iequals(command,"createWorld"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string worldName;
			lineParser >> worldName;

			bool worldCreated = sAuth.CreateWorld(worldName);
			if (worldCreated)
				INFO_LOG(format("Created world named %1%") % worldName );
		}
		else if (iequals(command,"createCharacter"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string worldName,userName,charHandle,firstName,lastName;
			lineParser >> worldName;
			lineParser >> userName;
			lineParser >> charHandle;
			lineParser >> firstName;
			lineParser >> lastName;

			bool characterCreated = sAuth.CreateCharacter(worldName,userName,charHandle,firstName,lastName);
			if (characterCreated)
				INFO_LOG(format("Inserted character %1% into world %2% for user %3% with name %4% %5%")
				% charHandle % worldName % userName % firstName % lastName );

		}
		else if (iequals(command, "broadcastMsg") || iequals(command, "modalMsg"))
		{
			string theAnnouncement;
			getline(cin,theAnnouncement);
			while (theAnnouncement.c_str()[0] == ' ' || theAnnouncement.c_str()[0] == '\n')
			{
				theAnnouncement = theAnnouncement.substr(1);
			}

			if (theAnnouncement.size() > 0)
			{
				if (iequals(command, "broadcastMsg"))
					sGame.AnnounceCommand(NULL,make_shared<BroadcastMsg>(theAnnouncement));
				else if (iequals(command, "modalMsg"))
					sGame.AnnounceCommand(NULL,make_shared<ModalMsg>(theAnnouncement));

				cout << "OK" << std::endl;
			}
			else
			{
				cout << "EMPTY MESSAGE" << std::endl;
			}
		}
		else if (iequals(command, "send") || iequals(command, "sendCmd") )
		{
			stringstream hexStream;
			for (;;)
			{
				string word;
				cin >> word;

				string::size_type semicolonPos = word.find_first_of(";");
				if (semicolonPos != string::npos)
				{
					word = word.substr(0,semicolonPos);
					if (word.length() > 0)
					{
						hexStream << word;
					}
					break;
				}
				else
				{
					hexStream << word;
				}
			}

			string binaryOutput;
			try
			{
				CryptoPP::HexDecoder hexDecoder(new CryptoPP::StringSink(binaryOutput));
				hexDecoder.Put((const byte*)hexStream.str().data(),hexStream.str().size(),true);
				hexDecoder.MessageEnd();
			}
			catch (...)
			{
				cout << "Invalid hex string" << std::endl;		
				continue;
			}

			bool rpcCmd=false;
			if (iequals(command, "sendCmd"))
				rpcCmd=true;

			sGame.Broadcast(ByteBuffer(binaryOutput),rpcCmd);
			cout << "OK" << std::endl;
		}
	}

	return true;
}