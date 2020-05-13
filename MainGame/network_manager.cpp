#include "stdafx.h"
#include "network_manager.h"

network_manager* network_manager::Inst = NULL;

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

	/*m_my_info.Transform._11 = 1.f, m_my_info.Transform._12 = 0.f, m_my_info.Transform._13 = 0.f, m_my_info.Transform._14 = 0.f;
	m_my_info.Transform._21 = 0.f, m_my_info.Transform._22 = 1.f, m_my_info.Transform._23 = 0.f, m_my_info.Transform._24 = 0.f;
	m_my_info.Transform._31 = 0.f, m_my_info.Transform._32 = 0.f, m_my_info.Transform._33 = 1.f, m_my_info.Transform._34 = 0.f;
	m_my_info.Transform._41 = 0.f, m_my_info.Transform._42 = 0.f, m_my_info.Transform._43 = 0.f, m_my_info.Transform._44 = 1.f;*/

	m_OtherInfo.is_connect = false;
}

void network_manager::rq_connect_server(const char * server_ip)
{
	memset(&m_serverAddr, 0, sizeof(SOCKADDR_IN));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, server_ip, &m_serverAddr.sin_addr);// ipv4에서 ipv6로 변환	
	connect(m_serverSocket, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr));
	//return m_serverSocket;
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
		cout << "id받기 확인" << m_my_info.id << endl;
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *pos_packet = reinterpret_cast<sc_packet_pos*>(buf);
		//cout << "id: " << pos_packet->id << endl;
		if (pos_packet->mover_id == m_my_info.id) { // 자기자신 위치
			m_my_info.Transform = pos_packet->world_pos;
			cout << "내 위치 받기 확인" << endl;
		}
		else { // 다른 플레이어 위치
			m_OtherInfo.Transform = pos_packet->world_pos;
			m_OtherInfo.AnimateState = pos_packet->animation_state;
			cout << "다른 플레이어 위치 받기 확인" << endl;
		}
		break;
	}
	case SC_PUT_PLAYER: {
		sc_packet_put_player *put_player_packet = reinterpret_cast<sc_packet_put_player*>(buf);
		int new_id = put_player_packet->new_player_id;
		if (new_id != m_my_info.id) {
			m_OtherInfo.id = new_id;
			m_OtherInfo.is_connect = true;
			cout << "다른 플레이어 풋 확인" << endl;
		}
		break;
	}
	case SC_REMOVE_PLAYER: {
		sc_packet_remove_player *remove_player_packet = reinterpret_cast<sc_packet_remove_player*>(buf);
		if (m_OtherInfo.id == remove_player_packet->leave_player_id) {
			m_OtherInfo.is_connect = false;
			m_OtherInfo.id = -1;
		}

		break;
	}

	}
}

void network_manager::send_change_state_packet(char state)
{
	cs_packet_client_state_change packet;
	packet.type = CS_CLIENT_STATE_CHANGE;
	packet.id = m_my_info.id;
	packet.change_state = state;
	packet.size = sizeof(packet);
	send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
}

void network_manager::send_my_world_pos_packet(DirectX::XMFLOAT4X4 world_pos, short animation_state)
{
	cs_packet_pos packet;
	packet.type = CS_POS;
	packet.id = m_my_info.id;
	packet.player_world_pos = world_pos;
	packet.animation_state = animation_state;
	packet.size = sizeof(packet);
	send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
}

void network_manager::send_make_room_packet()
{
	cs_packet_make_room packet;
	packet.type = CS_MAKE_ROOM;
	packet.id = m_my_info.id;
	packet.size = sizeof(packet);
	send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
}

void network_manager::send_request_join_room(short room_number)
{
	cs_packet_request_join_room packet;
	packet.type = CS_REQUEST_JOIN_ROOM;
	packet.joiner_id = m_my_info.id;
	packet.room_number = room_number;
	packet.size = sizeof(packet);
	send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
}
