#pragma once
//#include "globals.h"
#include "stdafx.h"
#include <algorithm>
#include <thread>
#define IPADDR = "127.0.0.1"

enum EVENT_TYPE {
	EV_RECV, EV_SEND
};

struct PLAYER_INFO
{
	int id;
	volatile bool is_connect;
	char player_state;
	short room_number;
	short AnimateState;

	short hp;
	short gold;
	XMFLOAT4X4 Transform;
};

struct GAME_ROOM_C {
	short room_number;
	char room_state;
	short wave_count;
	short stage_number;
	short portalLife;
	int players_id[4];
};

struct TRAPINFO_C {
	volatile bool enable;
	short id;
	char trap_type;
	DirectX::XMFLOAT3 trap_pos;
	DirectX::XMFLOAT4X4 trap4x4pos;
};

struct OVER_EX {
	WSAOVERLAPPED over;
	WSABUF wsabuf[1];
	char net_buf[MAX_BUFFER];
	EVENT_TYPE event_type;
};

class network_manager
{
public:
	network_manager();
	~network_manager();
private:
	static network_manager* Inst;

public:
	void init_data();
	void init_socket();
	void init_pool();
	void init_mon_pool();
	void rq_connect_server(const char * server_ip);
	void test_connect(HWND& hwnd);
	void ReadBuffer(SOCKET sock);
	void PacketProccess(char* buf);
	void recvThread();

	void send_packet(void *buf);
	void send_change_state_packet(const char& state, const short& StageNum);
	void send_my_world_pos_packet(const DirectX::XMFLOAT4X4& world_pos, const short& animation_state);
	void send_make_room_packet();
	void send_request_join_room(const short& room_number);
	void send_install_trap(char trap_type, DirectX::XMFLOAT4X4 trap_pos);
	void send_shoot(short monster_id, bool headShot);
	void send_request_login(char name[]);
	void send_leaveRoom();

	void socket_err_display(const char * msg, int err_no);


public:
	bool wsa = false;

	int orc_id = 0, sorc_id = 0, wolf_id = 0;

	WSADATA m_WSAData;
	SOCKET m_serverSocket;
	sockaddr_in m_serverAddr;

	WSABUF m_recv_buf;	// 최초에 recv한 데이터 받아올 버퍼
	WSABUF send_wsabuf;
	char m_buffer[MAX_BUFFER];
	char send_buffer[MAX_BUFFER];

	PLAYER_INFO m_my_info;
	PLAYER_INFO m_OtherInfo;
	GAME_ROOM_C m_myRoomInfo;
	bool m_nameLogin = false;

	vector<GAME_ROOM_C*> m_vec_gameRoom;
	vector<TRAPINFO_C*> m_vec_trapPool;
	MONSTER m_monster_pool[MAX_MONSTER];
	TRAPINFO_C m_trap_pool[MAX_TRAP];

	MONSTER m_orcPool[MAX_MONSTER];
	MONSTER m_strongorcPool[MAX_MONSTER];
	MONSTER m_riderPool[MAX_MONSTER];

	HWND async_handle;
	bool first_recv;


public:
	bool IsConnect() { return m_OtherInfo.is_connect; }

public:
	static network_manager* GetInst() {
		if (Inst == NULL)
			Inst = new network_manager;

		return Inst;
	}

	static void DestroyInst() {
		if (Inst) {
			delete Inst;
			Inst = nullptr;
		}
	}
};

