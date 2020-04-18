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
	WSAStartup(MAKEWORD(2, 0), &m_WSAData);	//  ��Ʈ��ũ ����� ����ϱ� ����, ���ͳ� ǥ���� ����ϱ� ����
	m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);

	m_recv_buf.len = MAX_BUFFER;
	m_recv_buf.buf = m_buffer;

	/*m_my_info.Transform._11 = 1.f, m_my_info.Transform._12 = 0.f, m_my_info.Transform._13 = 0.f, m_my_info.Transform._14 = 0.f;
	m_my_info.Transform._21 = 0.f, m_my_info.Transform._22 = 1.f, m_my_info.Transform._23 = 0.f, m_my_info.Transform._24 = 0.f;
	m_my_info.Transform._31 = 0.f, m_my_info.Transform._32 = 0.f, m_my_info.Transform._33 = 1.f, m_my_info.Transform._34 = 0.f;
	m_my_info.Transform._41 = 0.f, m_my_info.Transform._42 = 0.f, m_my_info.Transform._43 = 0.f, m_my_info.Transform._44 = 1.f;*/

	m_OtherInfo.is_connect = false;
}

SOCKET network_manager::rq_connect_server(const char * server_ip)
{
	memset(&m_serverAddr, 0, sizeof(SOCKADDR_IN));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, server_ip, &m_serverAddr.sin_addr);// ipv4���� ipv6�� ��ȯ	
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
		cout << "id�ޱ� Ȯ��"<< m_my_info.id << endl;
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *pos_packet = reinterpret_cast<sc_packet_pos*>(buf);
		cout << "id: " << pos_packet->id << endl;
		if (pos_packet->mover_id == m_my_info.id) { // �ڱ��ڽ� ��ġ
			m_my_info.Transform = pos_packet->world_pos;
			cout << "�� ��ġ �ޱ� Ȯ��" << endl;
		}
		else { // �ٸ� �÷��̾� ��ġ
			m_OtherInfo.Transform = pos_packet->world_pos;
			cout << "�ٸ� �÷��̾� ��ġ �ޱ� Ȯ��" << endl;
		}
		break;
	}
	case SC_PUT_PLAYER: {
		sc_packet_put_player *put_player_packet = reinterpret_cast<sc_packet_put_player*>(buf);
		int new_id = put_player_packet->new_player_id;
		if (new_id != m_my_info.id) {
			m_OtherInfo.is_connect = true;
			cout << "�ٸ� �÷��̾� ǲ Ȯ��" << endl;
		}
		break;
	}
	case SC_REMOVE_PLAYER: {
		sc_packet_remove_player *remove_player_packet = reinterpret_cast<sc_packet_remove_player*>(buf);
		if (remove_player_packet->leave_player_id != m_my_info.id) {
			m_OtherInfo.is_connect = false;
		}

		break;
	}
		
	}
}