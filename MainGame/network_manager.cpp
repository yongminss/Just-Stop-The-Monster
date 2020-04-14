#include "stdafx.h"
#include "network_manager.h"


network_manager::network_manager()
{
}


network_manager::~network_manager()
{
}

void network_manager::init_socket()
{
	WSAStartup(MAKEWORD(2, 0), &m_WSAData);	//  네트워크 기능을 사용하기 위함, 인터넷 표준을 사용하기 위해
	m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);

	m_recv_buf.len = MAX_BUFFER;
	m_recv_buf.buf = m_buffer;
}

SOCKET network_manager::rq_connect_server(const char * server_ip)
{
	memset(&m_serverAddr, 0, sizeof(SOCKADDR_IN));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, server_ip, &m_serverAddr.sin_addr);// ipv4에서 ipv6로 변환	
	connect(m_serverSocket, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr));
	return m_serverSocket;
}

void network_manager::ReadBuffer(SOCKET sock)
{
	int in_packet_size = 0;
	int saved_packet_size = 0;

	DWORD iobyte, ioflag = 0;
	WSARecv(sock, &m_recv_buf, 1, &iobyte, &ioflag, NULL, NULL);

	char * temp = reinterpret_cast<char*>(m_buffer);

	while (iobyte != 0)
	{
		if (in_packet_size == 0)
		{
			in_packet_size = temp[0];
		}
		if (iobyte + saved_packet_size >= in_packet_size)
		{
			memcpy(m_buffer + saved_packet_size, temp, in_packet_size - saved_packet_size);
			PacketProccess(m_buffer);
			temp += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else
		{
			memcpy(m_buffer + saved_packet_size, temp, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void network_manager::PacketProccess(void * buf)
{
	char* temp = reinterpret_cast<char*>(buf);

	switch (temp[1]) {
	case SC_SEND_ID:
	{
		sc_packet_send_id *send_id_packet = reinterpret_cast<sc_packet_send_id*>(buf);
		m_my_info.id = send_id_packet->id;
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *pos_packet = reinterpret_cast<sc_packet_pos*>(buf);
		m_my_info.x = pos_packet->x;
		m_my_info.y = pos_packet->y;
		break;
	}
	}
}
