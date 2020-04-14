#pragma once
//#include "globals.h"
#include "stdafx.h"


struct PLAYER_INFO
{
	int id;
	bool is_connect;
	XMFLOAT4X4 Transform;
};

struct OVER_EX {
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char net_buf[MAX_BUFFER];
	//EVENT_TYPE event_type;
};

class network_manager
{
public:
	network_manager();
	~network_manager();

	void init_socket();
	SOCKET rq_connect_server(const char * server_ip);
	void ReadBuffer(SOCKET sock);

	void PacketProccess(void * buf);


public:
	WSADATA m_WSAData;
	SOCKET m_serverSocket;
	sockaddr_in m_serverAddr;

	WSABUF m_recv_buf;	// 최초에 recv한 데이터 받아올 버퍼
	char m_buffer[MAX_BUFFER];

	PLAYER_INFO m_my_info;
	PLAYER_INFO m_OtherInfo;

public:
	bool IsConnect() { return m_OtherInfo.is_connect; }
};

