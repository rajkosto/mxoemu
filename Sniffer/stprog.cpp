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

#include <pcap.h>

#define LINE_LEN 16
#include "Util.h"
#include "Internets.h"

CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptGTC;
CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *TFDecryptCTG;

DWORD WINAPI InstanceThread(LPVOID lpvParam) 
{
	HANDLE hPipe = CreateNamedPipe( 
		  "\\\\.\\pipe\\MxoTwofishKey",             // pipe name 
          PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,       // read/write access 
          PIPE_TYPE_BYTE |       // byte type pipe 
          PIPE_READMODE_BYTE |   // byte-read mode 
          PIPE_WAIT,                // blocking mode 
          1,					// max. instances  
          0x10,                  // output buffer size 
          0x10,                  // input buffer size 
          NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
          NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE) 
	{
		cout << "CreatePipe failed" << endl;
		exit(0);
	}

	for (;;)
	{
	
		BOOL bResult = ConnectNamedPipe (hPipe, NULL);

		if ( (FALSE == bResult) && ( ERROR_PIPE_CONNECTED != GetLastError() ) )
		{
			cout << "HAXXXX CANT CONNECT TO PIPEZ" << endl;
		}

		byte buffar[16];
		DWORD hax;

		BOOL ok = ReadFile( 
				 hPipe,        // handle to pipe 
				 buffar,    // buffer to receive data 
				 sizeof(buffar), // size of buffer 
				 &hax, // number of bytes read 
				 NULL);        // not overlapped I/O 

		if (!ok || hax != 0x10)
		{
			cout << "HAX !! GET AWAY ZE BOMB IS GONNA BLOWZ ! " << hax << endl; 
			goto end;
		}
		else
		{
			std::string data= std::string((char*)buffar,sizeof(buffar));
			std::string password;
			ConvertBytesintoHex((byte*)data.c_str(),&password,sizeof(buffar));
			cout << "TwoFish passkey: " << password << endl;
			byte hax[16];
			memset(hax,0,sizeof(hax));

			if (TFDecryptGTC != NULL)
				delete TFDecryptGTC;
			if (TFDecryptCTG != NULL)
				delete TFDecryptCTG;

			TFDecryptGTC=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(buffar, CryptoPP::Twofish::DEFAULT_KEYLENGTH, hax);
			TFDecryptCTG=new CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption(buffar, CryptoPP::Twofish::DEFAULT_KEYLENGTH, hax);
		}
end:
		FlushFileBuffers(hPipe); 
		DisconnectNamedPipe(hPipe); 
	}
	CloseHandle(hPipe); 
	return 0;
}

typedef struct ServerSeq
{ 
	unsigned server_sequence: 12;
	unsigned client_sequence: 12;
	unsigned flags: 8;
} ServerSequence;

std::string IP2String(ip_address ip)
{
	std::ostringstream hax;

	hax << (int)ip.byte1 << "." << (int)ip.byte2 << "." << (int)ip.byte3 << "." << (int)ip.byte4;
	hax.flush();
	return hax.str();
}

#include "Config.h"

void ProcessPacket(u_char type,ip_address sip,ip_address dip,u_short sport,u_short dport,u_char *data,unsigned int lenght)
{
	if (!lenght)
		return;

	if (type==IPPROTO_TCP && (sport != MARGIN_PORT && dport != MARGIN_PORT))
		return;
	else if (type==IPPROTO_UDP && (sport != WORLD_PORT && dport != WORLD_PORT))
		return;

	std::string sourceIP = IP2String(sip);
	std::string destIP = IP2String(dip);

	std::string FileName="Unknown";

	if (sourceIP == RECURSION || destIP == RECURSION)
		FileName="Recursion";
	else if (sourceIP == SYNTAX || destIP == SYNTAX)
		FileName="Syntax";
	else if (sourceIP == VECTOR || destIP == VECTOR)
		FileName="Vector";
	else if (sourceIP == LOCALHOST || destIP == LOCALHOST)
		FileName="LocalHost";

	FileName+=".log";

	char dateStr [9];
	char timeStr [9];
	_strdate_s(dateStr,9);
	_strtime_s(timeStr,9);

	std::string location = "error";

	bool encrypted = false;
	unsigned char flags = 0;
	unsigned short client_sequence = 0;
	unsigned short server_sequence = 0;
	CryptoPP::CBC_Mode<CryptoPP::Twofish>::Decryption *Decryptor;
	Decryptor = NULL;

	std::string display;

	if (type==IPPROTO_TCP)
	{
		if (sport==MARGIN_PORT)
			location="MRGN->Client";
		else if (dport==MARGIN_PORT)
			location="Client->MRGN";

		display=std::string((const char*)data,lenght);
	}
	else if (type==IPPROTO_UDP)
	{
		if (data[0] == 0x01)
			encrypted = true;

		if (sport==WORLD_PORT)
		{
			if (encrypted)
				Decryptor = TFDecryptGTC;

			location="GAME->Client";
		}
		else if (dport==WORLD_PORT)
		{
			if (encrypted)
				Decryptor = TFDecryptCTG;

			location="Client->GAME";
		}

		if (encrypted && Decryptor == NULL)
			return;
		
		if (encrypted)
		{
			std::string vector((const char*)data+1,16);
			Decryptor->Resynchronize((const byte *)vector.c_str());
			std::string input((const char*)data+17,lenght-17);
			std::string output;
			CryptoPP::StringSource(input, true, new CryptoPP::StreamTransformationFilter(*Decryptor, new CryptoPP::StringSink(output)));

			memcpy(&flags,output.c_str()+10,sizeof(flags));

			if (sport==WORLD_PORT)
			{
				memcpy(&client_sequence,output.c_str()+11,sizeof(client_sequence));
				memcpy(&server_sequence,output.c_str()+12,sizeof(server_sequence));
				client_sequence = (client_sequence >> 4);
				client_sequence = (client_sequence & 0xFF0F);
				client_sequence = ntohs(client_sequence);
				server_sequence = (server_sequence & 0xFF0F);
				server_sequence = ntohs(server_sequence);
			}
			else if (dport==WORLD_PORT)
			{
				memcpy(&server_sequence,output.c_str()+11,sizeof(server_sequence));
				memcpy(&client_sequence,output.c_str()+12,sizeof(client_sequence));
				server_sequence = (server_sequence >> 4);
				server_sequence = (server_sequence & 0xFF0F);
				server_sequence = ntohs(server_sequence);
				client_sequence = (client_sequence & 0xFF0F);
				client_sequence = ntohs(client_sequence);
			}

			display = std::string(output.c_str()+14,output.size()-14); // remove first 14 bytes
		}
		else
			display=std::string((const char*)data,lenght);
	}

	std::ofstream File;
	File.open(FileName.c_str(),std::ios::app);
	File << location << " " << "[" << timeStr << " " << dateStr  << "]" << " " << "packet size: " << display.size();
	if (encrypted)
		File << " CRYPTED, Flags: " << std::hex << (int)flags << " ServerSeq: " << std::dec << server_sequence << " ClientSeq: " << client_sequence;
	File << endl;
	string text;
	ConvertBytesintoHex((byte *)display.c_str(),&text,(unsigned int)display.size());
	File << text << endl;
	int j = 0;
		for (unsigned int i = 0;i < display.size();i++)
		{
			j++;
			if (j == 97)
			{
				File << endl;
				j = 1;
			}
			if (display.c_str()[i] > 31 && display.c_str()[i] < 127)
				File << display.c_str()[i];
			else
				File << ".";
		}
	File << endl << endl;
	File.close();
}

int main()
{
	srand ( (unsigned int)time(NULL) );
	TFDecryptGTC=NULL;
	TFDecryptCTG=NULL;

	DWORD dwThreadId; 

	CreateThread( 
            NULL,              // no security attribute 
            0,                 // default stack size 
            InstanceThread,    // thread proc
            (LPVOID) NULL,    // thread parameter 
            0,                 // not suspended 
            &dwThreadId);      // returns thread ID 

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	int res;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr *header;
	const u_char *pkt_data;

	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		cout << "Error in pcap_findalldevs: "<< errbuf << endl;
		exit(1);
	}

	for(d=alldevs; d; d=d->next)
	{
		cout << ++i << ". ";

		if (d->description)
			cout << d->description << endl;
		else
			cout << "No description available" << endl;
	}
	        
	if (!i)
	{
		cout << endl << "No interfaces found! Make sure WinPcap is installed." << endl;
		return -1;
	}

	cout << "Enter the interface number (1-"<<i<<"):";
	cin >> inum;
	        
	if (inum < 1 || inum > i)
	{
		cout << endl << "Interface number out of range." << endl;

		pcap_freealldevs(alldevs);
		return -1;
	}
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
    
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		cout << endl << "Unable to open the adapter. " << d->description << " is not supported by WinPcap"<<endl;
		pcap_freealldevs(alldevs);
		return -1;
	}
    
    printf("\nlistening on %s...\n", d->description);
	
    /* At this point, we don't need any more the device list. Free it */
    pcap_freealldevs(alldevs);
	
	/* Retrieve the packets */
	while((res = pcap_next_ex( adhandle, &header, &pkt_data)) >= 0)
	{		
		if(res == 0)
			/* Timeout elapsed */
			continue;

		ip_header *ip;
		u_int size_ip;

		ip = (ip_header*)(pkt_data + SIZE_ETHERNET);
		size_ip = (ip->ver_ihl & 0xf) * 4; //Gets length of IP header with options
		if (size_ip < 20) 
			continue;

		if (ip->proto == IPPROTO_TCP)
		{
			tcp_header *tcp;
			tcp = (tcp_header*)(pkt_data + SIZE_ETHERNET + size_ip); //TCP header
			u_int size_tcp = tcp->th_off/4;
			if (size_tcp < 20) 
			{
				cout << "Invalid TCP header length: " << size_tcp << " bytes" << endl;
				continue;
			}

			ProcessPacket(IPPROTO_TCP,ip->saddr,ip->daddr,htons(tcp->sport),htons(tcp->dport),
						 (u_char *)(pkt_data + SIZE_ETHERNET + size_ip + size_tcp),
						 (header->caplen - SIZE_ETHERNET - size_ip - size_tcp));
		}
		else if (ip->proto == IPPROTO_UDP)
		{
			udp_header *udp;
			udp = (udp_header*)(pkt_data + SIZE_ETHERNET + size_ip); //UDP header

			ProcessPacket(IPPROTO_UDP,ip->saddr,ip->daddr,htons(udp->sport),htons(udp->dport),
						 (u_char *)(pkt_data + SIZE_ETHERNET + size_ip + SIZE_UDP),(htons(udp->len)-SIZE_UDP));
		}
	}
	
	if(res == -1)
	{
		cout << "Error reading the packets: " << pcap_geterr(adhandle) << endl;
		return -1;
	}
	
	pcap_close(adhandle);

	if (TFDecryptGTC != NULL)
		delete TFDecryptGTC;
	if (TFDecryptCTG != NULL)
		delete TFDecryptCTG;

   return 0;
}