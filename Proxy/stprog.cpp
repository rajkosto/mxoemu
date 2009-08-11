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

#include <stdio.h>
#include "ProxyHandler.h"
#include "LocalSocket.h"
#include "RemoteSocket.h"
#include <windows.h>
#include "Util.h"
#include <iostream>
#include "Logging.h"

#define AUTHPORT_LOCAL 11000
#define GAMEPORT_LOCAL 10000

#ifdef LOCALHOST_CAPTURE
#define AUTHPORT_REMOTE 13000
#define GAMEPORT_REMOTE 12000
#else
#define AUTHPORT_REMOTE 11000
#define GAMEPORT_REMOTE 10000
#endif

ProxyHandler h;
LocalSocket *worldLocal;
RemoteSocket *worldRemote;
u_short GlobalPort;
std::string GlobalIP;

void PerformReplay( const char*p, size_t l, bool server);

#include <Wincrypt.h>

void ProcessCommand(string processMe,bool server)
{
	if (processMe.size() < 1)
		return;

	if (processMe == "ROTATE")
	{
		cout << "ROTATING" << endl;
		if (server == false)
		{
			unsigned char rawData[10] =
			{
				0x02, 0x03, 0x02, 0x00, 0x01, 0x06, 0x00, 0x5A, 0x00, 0x00, 
			};

			rawData[7] = rand()%255;

			PerformReplay((const char *)&rawData[0],sizeof(rawData),false);
		}
		else
		{
			unsigned char rawData[9] =
			{
				0x02, 0x03, 0x02, 0x00, 0x01, 0x04, 0x21, 0x00, 0x00, 
			};

			rawData[6] = rand()%255;

			PerformReplay((const char *)&rawData[0],sizeof(rawData),true);
		}

		return;
	}

	string strippedCommand;

	for (string::size_type i=0;i<processMe.size();i++)
	{
		char currChar = tolower(processMe[i]);
		if ( (currChar >= '0' && currChar <= '9') || (currChar >= 'a' && currChar <= 'f') )
		{
			strippedCommand += string(&currChar,1);
		}
	}

	if (strippedCommand.size() < 1)
		return;

	cout << "Replaying ";
	if (server == true)
	{
		cout << "server |";
	}
	else
	{
		cout << "client |";
	}

	cout << strippedCommand << "|" << endl;

	vector <unsigned char> rawBytes;
	rawBytes.resize(strippedCommand.size()); // should require much less than this
	DWORD sizeOfRawBytes = rawBytes.size();

	bool success = CryptStringToBinary(strippedCommand.c_str(),strippedCommand.length(),CRYPT_STRING_HEX,&rawBytes[0],&sizeOfRawBytes,NULL,NULL);

	if (success == false)
	{
		cout << "WTF converting to binary failed hard for some reason" << endl;
		return;
	}

	rawBytes.resize(sizeOfRawBytes);

	PerformReplay((const char *)&rawBytes[0],rawBytes.size(),server);
}

#include "Config.h"

DWORD WINAPI ConsoleInThread(LPVOID lpvParam)
{
	string commandBuffer;

	for (;;)
	{
		string::size_type startpos;
		if ((startpos=commandBuffer.find_first_of('[')) != string::npos)
		{
			//trim anything before [
			if (startpos != 0)
			{
				commandBuffer = commandBuffer.substr(startpos);
			}

			//do we have a complete command ?
			string::size_type endpos = commandBuffer.find_first_of(']');
			if (endpos != string::npos)
			{
				string command = commandBuffer.substr(1,endpos-1);
				ProcessCommand(command,false);
				commandBuffer = commandBuffer.substr(endpos+1);

				continue;
			}
		}
		else if ((startpos=commandBuffer.find_first_of('{')) != string::npos)
		{
			//trim anything before {
			if (startpos != 0)
			{
				commandBuffer = commandBuffer.substr(startpos);
			}

			//do we have a complete command ?
			string::size_type endpos = commandBuffer.find_first_of('}');
			if (endpos != string::npos)
			{
				string command = commandBuffer.substr(1,endpos-1);
				ProcessCommand(command,true);
				commandBuffer = commandBuffer.substr(endpos+1);

				continue;
			}
		}
		else
		{
			//no command in buffer, so its just garbage we can clear out
			commandBuffer = "";
		}

		string currLine;
		getline(cin,currLine);

		commandBuffer+= currLine;
	}
}

typedef enum
{
	SERVER_AUTH,
	SERVER_MARGIN
} ServerType;

#include <Windows.h>
#include <Winsock2.h>

typedef struct  
{
	ServerType sType;
	SOCKET localSocket;
	SOCKET remoteSocket;
} ServerParams;

DWORD WINAPI TcpProxyRemoteThread(LPVOID lpvParam)
{
	ServerParams theParams;
	memcpy(&theParams,lpvParam,sizeof(theParams));
	free(lpvParam);

	ServerType theServerType = theParams.sType;
	SOCKET localClientSocket = theParams.localSocket;
	SOCKET remoteClientSocket = theParams.remoteSocket;

	//now listen for packets
	for (;;)
	{
		byte sizeBytes[2];
		memset(&sizeBytes,0,sizeof(sizeBytes));
		if (recv(remoteClientSocket,(char*)&sizeBytes[0],1,0) < 1)
		{
			//connection closed
			break;
		}
		//we got our one byte
		int numSizeBytes = 1;
		if (sizeBytes[0] > 0x7F)
		{
			numSizeBytes = 2;
			sizeBytes[0] -= 0x80;
		}
		if (numSizeBytes == 2)
		{
			if (recv(remoteClientSocket,(char*)&sizeBytes[1],1,0) < 1)
			{
				//connection closed
				break;
			}
		}
		uint16 packetSize = 0;
		//now we got both bytes there
		if (numSizeBytes == 1)
		{
			packetSize = sizeBytes[0];
		}
		else
		{
			memcpy(&packetSize,sizeBytes,sizeof(packetSize));
			packetSize = ntohs(packetSize);
		}

		ByteBuffer ourWholePacket;
		if (numSizeBytes == 1)
		{
			ourWholePacket << uint8(packetSize);
		}
		else if (numSizeBytes == 2)
		{
			ourWholePacket << uint16(swap16(packetSize | 0x8000));
		}

		//now we wait for those bytes
		while (ourWholePacket.size() < (numSizeBytes + packetSize))
		{
			byte tempByte = 0;
			if (recv(remoteClientSocket,(char*)&tempByte,sizeof(tempByte),0) < 1)
			{
				//connection aborted
				break;
			}
			//we received a byte, append it to our packet
			ourWholePacket << uint8(tempByte);
		}

		//if they arent equal, means receiving failed somewhere, and socket was closed
		if (ourWholePacket.size() != (numSizeBytes + packetSize))
		{
			break;
		}
		//process data from server
		string packetToSend;
		if (theServerType == SERVER_AUTH)
		{
			packetToSend = AuthToClient(ourWholePacket.contents(),ourWholePacket.size());
		} 
		else if (theServerType == SERVER_MARGIN)
		{
			packetToSend = MarginToClient(ourWholePacket.contents(),ourWholePacket.size());
		}
		//send it to real client
		send(localClientSocket,packetToSend.data(),packetToSend.size(),0);
	}		
	closesocket(remoteClientSocket);

	return 0;
}

DWORD WINAPI TcpProxyLocalThread(LPVOID lpvParam)
{	
	DWORD serverTypeInt = (DWORD)lpvParam;
	ServerType theServerType = (ServerType)(serverTypeInt);
	SOCKET tcpServerListeningSock = socket(AF_INET,SOCK_STREAM,0);
	string serverTypeStr;

	if (theServerType == SERVER_AUTH)
	{
		serverTypeStr = "Auth";
	}
	else if (theServerType == SERVER_MARGIN)
	{
		serverTypeStr = "Margin";
	}

	if(tcpServerListeningSock==INVALID_SOCKET)
	{
		cout << "Unable to create " << serverTypeStr << " Server Local SOCKET" << endl;
		return 0;
	}

	sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;



	if (theServerType == SERVER_AUTH)
	{
		local.sin_port=htons(AUTHPORT_LOCAL);
	}
	else if (theServerType == SERVER_MARGIN)
	{
		local.sin_port=htons(GAMEPORT_LOCAL);
	}

	if(bind(tcpServerListeningSock,(sockaddr*)&local,sizeof(local))!=0)
	{
		cout << "Unable to BIND " << serverTypeStr << " Server Local Socket" << endl;
		return 0;
	}

	if(listen(tcpServerListeningSock,10)!=0)
	{
		cout << "Unable to LISTEN on " << serverTypeStr << "Server Local Socket" << endl;
		return 0;
	}

	for(;;)
	{
		sockaddr_in from;
		int fromlen=sizeof(from);
		SOCKET localClientSocket=accept(tcpServerListeningSock,(struct sockaddr*)&from,&fromlen);
		if (localClientSocket == INVALID_SOCKET)
		{
			cout << "Invalid " << serverTypeStr << " client connected" << endl;
			continue;
		}

		SOCKET remoteClientSocket = socket(AF_INET,SOCK_STREAM,0);
		if(remoteClientSocket==INVALID_SOCKET)
		{
			cout << "Unable to create " << serverTypeStr << " Server Remote SOCKET" << endl;
			closesocket(localClientSocket);
			continue;
		}
		sockaddr_in remote;
		remote.sin_family=AF_INET;
		if (theServerType == SERVER_AUTH)
		{
#ifdef LOCALHOST_CAPTURE
			remote.sin_addr.s_addr=inet_addr("127.0.0.1");
#else
			remote.sin_addr.s_addr=inet_addr("64.37.156.17");
#endif
			remote.sin_port=htons(AUTHPORT_REMOTE);
		}
		else if (theServerType == SERVER_MARGIN)
		{
#ifdef LOCALHOST_CAPTURE
			remote.sin_addr.s_addr=inet_addr("127.0.0.1");
#else
			remote.sin_addr.s_addr=inet_addr(SERVER_IP.c_str());
#endif
			remote.sin_port=htons(GAMEPORT_REMOTE);
		}

		if (connect(remoteClientSocket,(sockaddr*)&remote,sizeof(remote)) == SOCKET_ERROR)
		{
			cout << "Unable to connect " << serverTypeStr << " Server Remote SOCKET" << endl;
			closesocket(remoteClientSocket);
			closesocket(localClientSocket);
			continue;
		}

		ServerParams *newServerParams = (ServerParams*)malloc(sizeof(ServerParams));
		newServerParams->localSocket = localClientSocket;
		newServerParams->remoteSocket = remoteClientSocket;
		newServerParams->sType = theServerType;

		//create remote end thread
		CreateThread( 
			NULL,              // no security attribute 
			0,                 // default stack size 
			TcpProxyRemoteThread,   // thread proc
			(LPVOID)newServerParams,    // thread parameter 
			0,                 // not suspended 
			NULL);      // returns thread ID 
		
		//now listen for packets
		for (;;)
		{
			byte sizeBytes[2];
			memset(&sizeBytes,0,sizeof(sizeBytes));
			if (recv(localClientSocket,(char*)&sizeBytes[0],1,0) < 1)
			{
				//connection closed
				cout << serverTypeStr << " Local Server: Client closed connection 1st recv" << endl;
				break;
			}
			//we got our one byte
			int numSizeBytes = 1;
			if (sizeBytes[0] > 0x7F)
			{
				numSizeBytes = 2;
				sizeBytes[0] -= 0x80;
			}
			if (numSizeBytes == 2)
			{
				if (recv(localClientSocket,(char*)&sizeBytes[1],1,0) < 1)
				{
					//connection closed
					cout << serverTypeStr << " Local Server: Client closed connection 2nd recv" << endl;
					break;
				}
			}
			uint16 packetSize = 0;
			//now we got both bytes there
			if (numSizeBytes == 1)
			{
				packetSize = sizeBytes[0];
			}
			else
			{
				memcpy(&packetSize,sizeBytes,sizeof(packetSize));
				packetSize = ntohs(packetSize);
			}

			ByteBuffer ourWholePacket;
			if (numSizeBytes == 1)
			{
				ourWholePacket << uint8(packetSize);
			}
			else if (numSizeBytes == 2)
			{
				ourWholePacket << uint16(swap16(packetSize | 0x8000));
			}

			//now we wait for those bytes
			while (ourWholePacket.size() < (numSizeBytes + packetSize))
			{
				byte tempByte = 0;
				if (recv(localClientSocket,(char*)&tempByte,sizeof(tempByte),0) < 1)
				{
					//connection aborted
					cout << serverTypeStr << " Local Server: Client closed connection 3nd recv" << endl;
					break;
				}
				//we received a byte, append it to our packet
				ourWholePacket << uint8(tempByte);
			}

			//if they arent equal, means receiving failed somewhere, and socket was closed
			if (ourWholePacket.size() != (numSizeBytes + packetSize))
			{
				cout << "Local Server: ourWholePacket not equal" << endl;
				break;
			}
			//process data from client
			string packetToSend;
			if (theServerType == SERVER_AUTH)
			{
				packetToSend = ClientToAuth(ourWholePacket.contents(),ourWholePacket.size());
			}
			else if (theServerType == SERVER_MARGIN)
			{
				packetToSend = ClientToMargin(ourWholePacket.contents(),ourWholePacket.size());
			}
			//send it to real server
			send(remoteClientSocket,packetToSend.data(),packetToSend.size(),0);
		}		
		closesocket(localClientSocket);
	}
	closesocket(tcpServerListeningSock);

	return 0;
}

int main()
{
	WSADATA wsaData;
	int wsaret=WSAStartup(MAKEWORD(2, 2),&wsaData);

	if(wsaret!=0)
	{
		cout << "Unable to init WINSOCK" << endl;
		return 0;
	}

	srand ( (unsigned int)time(NULL) );

	worldLocal = new LocalSocket(h);
	worldRemote = new RemoteSocket(h);

	port_t worldPort = 10000;

	if (worldLocal->Bind( worldPort ) != 0)
		exit(-1);

	h.Add(worldLocal);

	string serverToOpen;
	uint16 portToOpen;

#ifdef LOCALHOST_CAPTURE
	serverToOpen = "127.0.0.1";
	portToOpen = 12345;
#else
	serverToOpen = SERVER_IP;
	portToOpen = 10000;
#endif

	if (worldRemote->Open(serverToOpen, portToOpen) == false)
		exit(-1);

	h.Add(worldRemote);

	//make auth server thread
	CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		TcpProxyLocalThread,   // thread proc
		(LPVOID) SERVER_AUTH,    // thread parameter 
		0,                 // not suspended 
		NULL);      // returns thread ID 

	//make margin server thread
	CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		TcpProxyLocalThread,   // thread proc
		(LPVOID) SERVER_MARGIN,    // thread parameter 
		0,                 // not suspended 
		NULL);      // returns thread ID 

	CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		ConsoleInThread,    // thread proc
		(LPVOID) NULL,    // thread parameter 
		0,                 // not suspended 
		NULL);      // returns thread ID 

	//i will leave udp to alhem, FOR NOW
	while (1)
	{
		h.Select(1,0);
	}

	WSACleanup();

	return 0;
}