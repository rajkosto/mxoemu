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

#ifndef INTERNETS_H
#define INTERNETS_H

// 4 bytes IP address 
typedef struct ip_address
{ 
    u_char byte1; 
    u_char byte2; 
    u_char byte3; 
    u_char byte4; 
}ip_address; 

#define SIZE_ETHERNET 14

// 20 bytes IP Header 
typedef struct ip_header
{ 
    u_char ver_ihl; // Version (4 bits) + Internet header length (4 bits) 
    u_char tos; // Type of service 
    u_short tlen; // Total length 
    u_short identification; // Identification 
    u_short flags_fo; // Flags (3 bits) + Fragment offset (13 bits) 
    u_char ttl; // Time to live 
    u_char proto; // Protocol 
    u_short crc; // Header checksum 
    ip_address saddr; // Source address 
    ip_address daddr; // Destination address 
	u_int op_pad; // Option + Padding -- NOT NEEDED! 
}ip_header; 

//"Simple" struct for TCP
typedef struct tcp_header 
{ 
	u_short sport; // Source port 
	u_short dport; // Destination port 
	u_int seqnum; // Sequence Number 
	u_int acknum; // Acknowledgement number 
	u_char th_off; // Header length 
	u_char flags; // packet flags 
	u_short win; // Window size 
	u_short crc; // Header Checksum 
	u_short urgptr; // Urgent pointer...still don't know what this is...

}tcp_header; 

//"Simple" struct for UDP
typedef struct udp_header
{
       u_short sport;                        // Source port
       u_short dport;                        // Destination port
       u_short len;                        // Datagram length
       u_short crc;                        // Checksum
}udp_header;

#define SIZE_UDP sizeof(udp_header)

#endif 