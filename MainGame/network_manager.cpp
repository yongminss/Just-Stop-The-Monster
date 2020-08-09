#include "stdafx.h"
#include "network_manager.h"

network_manager* network_manager::Inst = NULL;

network_manager::network_manager()
{
	//ZeroMemory(m_myRoomInfo, sizeof(m_myRoomInfo));
	wsa = false;
	m_OtherInfo.is_connect = false;
	init_data();
}


network_manager::~network_manager()
{
}

void network_manager::init_data()
{
	m_my_info.hp = 0;
	m_my_info.gold = 200;

	m_myRoomInfo.room_number = 0;
	m_myRoomInfo.portalLife = 20;
	m_myRoomInfo.wave_count = 0;
	m_myRoomInfo.stage_number = 0;

	m_nameLogin = false;

	ZeroMemory(&m_OtherInfo, sizeof(m_OtherInfo));
	ZeroMemory(&m_trap_pool, sizeof(m_trap_pool));
	ZeroMemory(&m_orcPool, sizeof(m_orcPool));
	ZeroMemory(&m_strongorcPool, sizeof(m_strongorcPool));
	ZeroMemory(&m_riderPool, sizeof(m_riderPool));

	m_vec_gameRoom.reserve(20);
	m_vec_trapPool.reserve(50);

	init_pool();
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

void network_manager::init_pool()
{

	for (short i = 0; i < MAX_TRAP; ++i) {
		m_trap_pool[i].enable = false;
		m_trap_pool[i].wallTrapOn = false;
	}
	this->init_mon_pool();
}

void network_manager::init_mon_pool()
{
	for (short i = 0; i < MAX_MONSTER; ++i) {
		//m_monster_pool[i].isLive = false;
		m_orcPool[i].isLive = false;
		m_orcPool[i].type = TYPE_ORC;
		m_strongorcPool[i].isLive = false;
		m_strongorcPool[i].type = TYPE_STRONGORC;
		m_riderPool[i].isLive = false;
		m_riderPool[i].type = TYPE_RIDER;
	}
}

void network_manager::rq_connect_server(const char * server_ip)
{
	memset(&m_serverAddr, 0, sizeof(SOCKADDR_IN));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, server_ip, &m_serverAddr.sin_addr);// ipv4에서 ipv6로 변환	
	connect(m_serverSocket, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr));
}

void network_manager::test_connect(HWND & hwnd)
{
	WSAStartup(MAKEWORD(2, 0), &m_WSAData);	//  네트워크 기능을 사용하기 위함, 인터넷 표준을 사용하기 위해
	m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	int opt_val = TRUE;
	setsockopt(m_serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val, sizeof(opt_val));

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int retval = WSAConnect(m_serverSocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) {
		printf("소켓 연결 안됨\n");
	}

	async_handle = hwnd;
	WSAAsyncSelect(m_serverSocket, async_handle, WM_SOCKET, FD_READ || FD_CLOSE);

	m_recv_buf.len = MAX_BUFFER;
	m_recv_buf.buf = m_buffer;

	send_wsabuf.len = MAX_BUFFER;
	send_wsabuf.buf = send_buffer;
}

void network_manager::ReadBuffer(SOCKET sock)
{

	DWORD iobyte, ioflag = 0;
	int ret = WSARecv(sock, &m_recv_buf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			socket_err_display("WSASend Error :", err_no);
	}
	//cout << "iobyte: " << iobyte << endl;

	unsigned short size = 0;
	char* ptr = reinterpret_cast<char*>(m_buffer);

	while (iobyte != 0)
	{
		memcpy(&size, ptr, sizeof(unsigned short));
		if (in_packet_size == 0) in_packet_size = size;
		if (iobyte + saved_packet_size >= in_packet_size)
		{
			//cout << size << ", " << (int)ptr[2] << endl;
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			PacketProccess(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else
		{
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}


	/*unsigned short * temp_size = reinterpret_cast<unsigned short*>(m_buffer);
	char * temp = reinterpret_cast<char*>(m_buffer);

	while (iobyte != 0)
	{
	if (in_packet_size == 0)
	{
	in_packet_size = temp_size[0];
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
	}*/
}

void network_manager::PacketProccess(char* buf)
{
	//char* temp = reinterpret_cast<char*>(buf);

	switch (buf[2]) {
	case SC_SEND_ID:
	{
		sc_packet_send_id *send_id_packet = reinterpret_cast<sc_packet_send_id*>(buf);
		if (send_id_packet->id >= 0 && send_id_packet->id < 50) {
			m_my_info.id = send_id_packet->id;
			cout << "id받기 확인" << m_my_info.id << endl;
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *pos_packet = reinterpret_cast<sc_packet_pos*>(buf);
		//cout << "id: " << pos_packet->id << endl;
		if (pos_packet->mover_id == m_my_info.id) { // 자기자신 위치
													//m_my_info.Transform = pos_packet->world_pos;
			cout << "내 위치 받기 확인" << endl;
		}
		else if (pos_packet->mover_id == m_OtherInfo.id) { // 다른 플레이어 위치
			m_OtherInfo.Transform = pos_packet->world_pos;
			m_OtherInfo.AnimateState = pos_packet->animation_state;
			m_OtherInfo.is_connect = true;
			//cout << "다른 플레이어 위치 받기 확인" << endl;
		}
		break;
	}
	case SC_PUT_PLAYER: {
		sc_packet_put_player *put_player_packet = reinterpret_cast<sc_packet_put_player*>(buf);
		int new_id = put_player_packet->new_player_id;
		if (new_id != m_my_info.id) {
			m_OtherInfo.id = new_id;
			//m_OtherInfo.is_connect = true;
			cout << "다른 플레이어 풋 확인" << new_id << endl;
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
	case SC_SEND_ROOM_LIST: {
		sc_packet_room_info *room_info_packet = reinterpret_cast<sc_packet_room_info*>(buf);

		if (m_myRoomInfo.room_number == room_info_packet->room_number) { // 내 방 업데이트
			for (int i = 0; i < 4; ++i) {
				m_myRoomInfo.players_id[i] = room_info_packet->players_id[i];
			}
			cout << "내방 정보 업데이트 \n";
			for (int i = 0; i < 4; ++i) {
				m_myRoomInfo.players_id[i] = room_info_packet->players_id[i];
				cout << m_myRoomInfo.players_id[i] << endl;
			}
		}



		auto findret = find_if(m_vec_gameRoom.begin(), m_vec_gameRoom.end(), [room_info_packet](GAME_ROOM_C* gr) {
			return gr->room_number == room_info_packet->room_number;
		});
		if (findret != m_vec_gameRoom.end()) { // 원래 벡터에 있던 방 정보 업데이트
			if (room_info_packet->room_enable == false) { // 삭제해야 할 방
				m_vec_gameRoom.erase(findret);
				return;
			}
			else {	// 원래 벡터에 있던 방 정보 업데이트
				for (auto &rw : m_vec_gameRoom) {
					if (rw->room_number == room_info_packet->room_number) {
						for (int i = 0; i < 4; ++i) {
							rw->players_id[i] = room_info_packet->players_id[i];
						}
						return;
					}
				}
			}
		}


		GAME_ROOM_C *new_room = new GAME_ROOM_C;
		new_room->room_number = room_info_packet->room_number;
		for (int i = 0; i < 4; ++i) {
			new_room->players_id[i] = room_info_packet->players_id[i];
		}
		m_vec_gameRoom.emplace_back(new_room);
		cout << "new room" << new_room->room_number << endl;
		break;
	}
	case SC_MAKE_ROOM_OK: {
		sc_packet_make_room_ok *make_room_ok_packet = reinterpret_cast<sc_packet_make_room_ok*>(buf);
		if (m_my_info.id == make_room_ok_packet->id) {
			m_my_info.room_number = make_room_ok_packet->room_number;
			m_my_info.player_state = PLAYER_STATE_in_room;
			m_myRoomInfo.room_number = make_room_ok_packet->room_number;
		}
		break;
	}
	case SC_GAME_START: {
		sc_packet_game_start *game_start_packet = reinterpret_cast<sc_packet_game_start*>(buf);
		cout << "game start packet \n";
		m_my_info.player_state = PLAYER_STATE_playing_game;
		m_my_info.hp = 200;
		m_my_info.gold = 500;
		m_myRoomInfo.portalLife = 20;
		m_myRoomInfo.wave_count = 1;
		init_pool();
	}
	case SC_MONSTER_POS: {
		sc_packet_monster_pos *monster_pos_packet = reinterpret_cast<sc_packet_monster_pos*>(buf);
		//memcpy_s(m_monster_pool, sizeof(m_monster_pool), monster_pos_packet->monsterArr, sizeof(monster_pos_packet->monsterArr));
		if (monster_pos_packet->monsterArr == NULL) { break; }
		for (short i = 0; i < MAX_MONSTER; ++i) {
			if (monster_pos_packet->monsterArr[i].isLive == false || monster_pos_packet->monsterArr[i].isLive == true) {
				if (monster_pos_packet->monsterArr[i].type == TYPE_ORC) {
					int monster_id = orc_id++;
					m_orcPool[monster_id].isLive = monster_pos_packet->monsterArr[i].isLive;
					m_orcPool[monster_id].id = monster_pos_packet->monsterArr[i].id;
					m_orcPool[monster_id].animation_state = monster_pos_packet->monsterArr[i].animation_state;
					m_orcPool[monster_id].world_pos = monster_pos_packet->monsterArr[i].world_pos;
				}
				else if (monster_pos_packet->monsterArr[i].type == TYPE_STRONGORC) {
					int monster_id = sorc_id++;
					m_strongorcPool[monster_id].isLive = monster_pos_packet->monsterArr[i].isLive;
					m_strongorcPool[monster_id].id = monster_pos_packet->monsterArr[i].id;
					m_strongorcPool[monster_id].animation_state = monster_pos_packet->monsterArr[i].animation_state;
					m_strongorcPool[monster_id].world_pos = monster_pos_packet->monsterArr[i].world_pos;
				}
				else if (monster_pos_packet->monsterArr[i].type == TYPE_RIDER) {
					int monster_id = wolf_id++;
					m_riderPool[monster_id].isLive = monster_pos_packet->monsterArr[i].isLive;
					m_riderPool[monster_id].id = monster_pos_packet->monsterArr[i].id;
					m_riderPool[monster_id].animation_state = monster_pos_packet->monsterArr[i].animation_state;
					m_riderPool[monster_id].world_pos = monster_pos_packet->monsterArr[i].world_pos;
				}
			}
		}
		orc_id = 0, sorc_id = 0, wolf_id = 0;
		break;
	}
	case SC_TRAP_INFO: {
		sc_packet_trap_info *trap_info_packet = reinterpret_cast<sc_packet_trap_info*>(buf);
		if (trap_info_packet->trap_index >= 0 && trap_info_packet->trap_index < MAX_TRAP) {
			cout << "new trap" << trap_info_packet->trap_index << endl;
			m_trap_pool[trap_info_packet->trap_index].enable = true;
			m_trap_pool[trap_info_packet->trap_index].id = trap_info_packet->trap_local_id;
			m_trap_pool[trap_info_packet->trap_index].trap_type = trap_info_packet->trap_type;
			m_trap_pool[trap_info_packet->trap_index].trap4x4pos = trap_info_packet->trap_pos;

			cout << "Server에서 받은 Trap ID : " << trap_info_packet->trap_index << endl;
		}
		break;
	}
	case SC_WALLTRAP_ON: {
		sc_packet_wallTrapOn *walltrap_on_packet = reinterpret_cast<sc_packet_wallTrapOn*>(buf);
		if (walltrap_on_packet->trap_index >= 0 && walltrap_on_packet->trap_index < MAX_TRAP) {
			m_trap_pool[walltrap_on_packet->trap_index].wallTrapOn = true;
			cout << walltrap_on_packet->trap_index << "번 벽함정 on \n";
		}
		break;
	}
	case SC_JOIN_ROOM_OK: {
		sc_packet_join_room_ok *join_room_ok_packet = reinterpret_cast<sc_packet_join_room_ok*>(buf);
		m_myRoomInfo.room_number = join_room_ok_packet->room_number;
		m_my_info.player_state = PLAYER_STATE_in_room;
		for (short i = 0; i < 4; ++i) {
			m_myRoomInfo.players_id[i] = join_room_ok_packet->players_id[i];
		}
		cout << "join " << join_room_ok_packet->room_number << " room \n";
		cout << "내방 정보 업데이트 \n";
		for (int i = 0; i < 4; ++i) {
			cout << m_myRoomInfo.players_id[i] << endl;
		}
		break;
	}
	case SC_PLAYER_STAT_CHANGE: {
		sc_packet_stat_change *stat_change_packet = reinterpret_cast<sc_packet_stat_change*>(buf);
		if (stat_change_packet->hp == -1000) { // gold 업데이트
			m_my_info.gold = stat_change_packet->gold;
		}
		else if (stat_change_packet->gold == -1000) { // hp업데이트
			m_my_info.hp = stat_change_packet->hp;
		}
		else { // hp업데이트
			m_my_info.hp = stat_change_packet->hp;
			m_my_info.gold = stat_change_packet->gold;
		}
		break;
	}
	case SC_GAME_INFO_UPDATE: {
		sc_packet_game_info_update *game_info_update_packet = reinterpret_cast<sc_packet_game_info_update*>(buf);
		if (game_info_update_packet->portalLife == -1000) { // wave 업데이트
			m_myRoomInfo.wave_count = game_info_update_packet->wave;
			cout << "wave update " << m_myRoomInfo.wave_count << endl;
			for (short i = 0; i < MAX_MONSTER; ++i) {
				//m_monster_pool[i].isLive = false;
			}
		}
		else if (game_info_update_packet->wave == -1000) { // portalLife 업데이트
			m_myRoomInfo.portalLife = game_info_update_packet->portalLife;
		}
		else {
			m_myRoomInfo.wave_count = game_info_update_packet->wave;
			m_myRoomInfo.portalLife = game_info_update_packet->portalLife;
		}
		break;
	}
	case SC_GAME_END: {
		// 게임종료
		sc_packet_game_end *game_end_packet = reinterpret_cast<sc_packet_game_end*>(buf);
		if (game_end_packet->clear == true) { // 클리어
			cout << "stage clear\n";
			game_end = true;
		}
		else {// 실패
			cout << "stage fail\n";
		}

		m_my_info.player_state = PLAYER_STATE_in_lobby;
		m_OtherInfo.is_connect = false;
		m_OtherInfo.id = -1;

		break;
	}
	case SC_NAMELOGIN_RESULT: {
		sc_packet_nameLogin_result *nameLogin_result_packet = reinterpret_cast<sc_packet_nameLogin_result*>(buf);
		if (nameLogin_result_packet->type == NAMELOGIN_SUC) {
			m_nameLogin = true;
		}
		else if (nameLogin_result_packet->type == NAMELOGIN_FAIL) {

		}
		break;
	}
	case SC_LEAVE_ROOM_OK: {
		sc_packet_leaveRoom_ok *leaveRoom_ok_packet = reinterpret_cast<sc_packet_leaveRoom_ok*>(buf);
		if (leaveRoom_ok_packet->id == m_my_info.id) {
			m_my_info.player_state = PLAYER_STATE_in_lobby;
		}
		break;
	}
	case SC_WAVE_END: {
		sc_packet_wave_end *wave_end_packet = reinterpret_cast<sc_packet_wave_end*>(buf);
		cout << "wave end \n";
		init_mon_pool();
		break;
	}


	}
}

void network_manager::recvThread()
{
	while (true) {
		int in_packet_size = 0;
		int saved_packet_size = 0;

		DWORD iobyte, ioflag = 0;
		int ret = WSARecv(m_serverSocket, &m_recv_buf, 1, &iobyte, &ioflag, NULL, NULL);
		if (ret != 0) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				socket_err_display("WSASend Error :", err_no);
		}
		//cout << "iobyte: " << iobyte << endl;

		unsigned short * temp_size = reinterpret_cast<unsigned short*>(m_buffer);
		char * temp = reinterpret_cast<char*>(m_buffer);

		while (iobyte != 0)
		{
			if (in_packet_size == 0)
			{
				in_packet_size = temp_size[0];
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
}

void network_manager::send_packet(void * buf)
{
	char* packet = reinterpret_cast<char*>(buf);
	int packet_size = packet[0];
	/*OVER_EX *send_over = new OVER_EX;
	memset(send_over, 0x00, sizeof(OVER_EX));
	send_over->event_type = EV_SEND;
	memcpy(send_over->net_buf, packet, packet_size);
	send_over->wsabuf[0].buf = send_over->net_buf;
	send_over->wsabuf[0].len = packet_size;

	int ret = WSASend(m_serverSocket, send_over->wsabuf, 1, 0, 0, &send_over->over, 0);
	if (0 != ret) {
	int err_no = WSAGetLastError();
	if (WSA_IO_PENDING != err_no)
	socket_err_display("WSASend Error :", err_no);
	}*/

	DWORD iobytes;
	memcpy(send_buffer, packet, packet_size);
	send_wsabuf.len = packet_size;
	send_wsabuf.buf = send_buffer;
	int ret = WSASend(m_serverSocket, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			socket_err_display("WSASend Error :", err_no);
	}
}

void network_manager::send_change_state_packet(const char& state, const short& StageNum = 1)
{
	cs_packet_client_state_change packet;
	packet.type = CS_CLIENT_STATE_CHANGE;
	packet.id = m_my_info.id;
	packet.change_state = state;
	packet.stage_number = StageNum;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_my_world_pos_packet(const DirectX::XMFLOAT4X4& world_pos, const short& animation_state)
{
	cs_packet_pos packet;
	packet.type = CS_POS;
	packet.id = m_my_info.id;
	packet.player_world_pos = world_pos;
	packet.animation_state = animation_state;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_make_room_packet()
{
	cs_packet_make_room packet;
	packet.type = CS_MAKE_ROOM;
	packet.id = m_my_info.id;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_request_join_room(const short& room_number)
{
	cs_packet_request_join_room packet;
	packet.type = CS_REQUEST_JOIN_ROOM;
	packet.joiner_id = m_my_info.id;
	packet.room_number = room_number;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_install_trap(unsigned short trap_local_id, char trap_type, DirectX::XMFLOAT4X4 trap_pos)
{
	cs_packet_install_trap packet;
	packet.type = CS_INSTALL_TRAP;
	packet.id = m_my_info.id;
	packet.trap_local_id = trap_local_id;
	packet.trap_type = trap_type;
	packet.trap_pos = trap_pos;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_shoot(short monster_id, bool headShot)
{
	cs_packet_shoot packet;
	packet.type = CS_SHOOT;
	packet.id = m_my_info.id;
	packet.monster_id = monster_id;
	packet.headShot = headShot;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_request_login(char name[])
{
	cs_packet_namelogin packet;
	packet.type = CS_REQUEST_NAMELOGIN;
	packet.id = m_my_info.id;
	strcpy_s(packet.name, name);
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}

void network_manager::send_leaveRoom()
{
	cs_packet_leaveRoom packet;
	packet.type = CS_LEAVE_ROOM;
	packet.id = m_my_info.id;
	packet.size = sizeof(packet);

	if (wsa == false) {
		send(m_serverSocket, (char*)&packet, sizeof(packet), 0);
	}
	else {
		send_packet(&packet);
	}
}



void network_manager::socket_err_display(const char * msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::cout << L"에러: " << lpMsgBuf << std::endl;

	while (true);
	LocalFree(lpMsgBuf);
}
