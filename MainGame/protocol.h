#pragma once
#include <DirectXMath.h>


struct MONSTER {
	short id;
	volatile bool isLive;
	char type;
	short animation_state;
	short hp;
	DirectX::XMFLOAT4X4 world_pos;
};

#define TYPE_DEFAULT	5

constexpr short lastWAVE = 10;
#define	WM_SOCKET			WM_USER + 1
#define MAX_BUFFER			65536	//16384
#define SERVER_PORT			3500

#define MAX_ROOMPLAYER		2
constexpr int PLAYER_NONE = -1;

#define MONSTER_ID_START	100
#define MAX_MONSTER			70
#define MAX_TRAP			100

// user_state
#define PLAYER_STATE_default		0
#define PLAYER_STATE_playing_game	1
#define PLAYER_STATE_in_lobby		2
#define PLAYER_STATE_in_room		3

// player
#define PLAYER_ATT	70

// room state
#define R_STATE_in_room				0
#define R_STATE_gameStart			1
#define R_STATE_wave_start			2
#define R_STATE_wave_end			3

// monster_type
#define TYPE_ORC		1
#define TYPE_SHAMAN		2
#define TYPE_STRONGORC	3
#define TYPE_RIDER		4

// monster_health
#define ORC_HP			200
#define SHAMAN_HP		150
#define STRONGORC_HP	250
#define RIDER_HP		250

// monster_att
#define ORC_ATT			10
#define SHAMAN_ATT		10
#define STRONGORC_ATT	10
#define RIDER_ATT		10

// monster att range
#define ORC_ATT_RANGE		70.f
#define SHAMAN_ATT_RANGE	70.f
#define STRONGORC_ATT_RANGE	70.f
#define RIDER_ATT_RANGE		70.f

// monster animation num
#define M_ANIM_IDLE		0
#define	M_ANIM_WALK		1
#define M_ANIM_RUN		2
#define M_ANIM_ATT		3
#define M_ANIM_DAMAGE	5
#define M_ANIM_DEATH	6
// monster buff type
#define TRAP_BUFF_NONE		0
#define TRAP_BUFF_SLOW		1

// trap type
#define TRAP_NEEDLE		0
#define TRAP_FIRE		1
#define TRAP_SLOW		2
#define TRAP_ARROW		3

#define TRAP_COST		30

// trap ATT
#define TRAP_NEEDLE_ATT		80
#define TRAP_FIRE_ATT		80
#define TRAP_SLOW_ATT		10
#define TRAP_ARROW_ATT		80

// trap range
#define TRAP_NEEDLE_RANGE		50.f
#define TRAP_FIRE_RANGE			300.f
#define TRAP_FIRE_WIDTHRANGE	30
#define TRAP_SLOW_RANGE			60.f
#define TRAP_ARROW_RANGE		350.f
#define TRAP_ARROW_WIDTHRANGE	30

#define SC_SEND_ID			1
#define SC_POS				2
#define SC_SEND_ROOM_LIST	3	
#define SC_PUT_PLAYER		4
#define SC_REMOVE_PLAYER	5
#define SC_TRAP_INFO		6
#define SC_JOIN_ROOM_OK		7
#define SC_MAKE_ROOM_OK		8
#define SC_MONSTER_POS		9
#define SC_GAME_END			10
#define SC_PLAYER_STAT_CHANGE	11
#define SC_GAME_INFO_UPDATE		12
#define SC_GAME_START			13
#define SC_NAMELOGIN_RESULT		14
#define		NAMELOGIN_SUC	0
#define		NAMELOGIN_FAIL	1
#define SC_LEAVE_ROOM_OK		15
#define SC_REMOVE_ROOM			16
#define SC_WAVE_END				17
#define SC_MONSTER_POS_TEST		18
#define SC_WALLTRAP_ON			19


#define CS_LEFT					1
#define CS_RIGHT				2
#define CS_UP					3
#define CS_DOWN					4
#define CS_MAKE_ROOM			5
#define CS_REQUEST_JOIN_ROOM	6
#define CS_POS					7
#define CS_TEST					8
#define CS_INSTALL_TRAP			9
#define CS_CLIENT_STATE_CHANGE	10
#define CS_GAME_START			11
#define CS_REQUEST_NAMELOGIN	12
#define CS_SHOOT				13
#define CS_LEAVE_ROOM			14

#pragma pack(push ,1)

// server to client
struct sc_packet_send_id {
	unsigned short size;
	char type;
	int id;
};

struct sc_packet_put_player {
	unsigned short size;
	char type;
	int new_player_id;
	short animation_state;
	DirectX::XMFLOAT4X4 world_pos;
};

struct sc_packet_pos {
	unsigned short size;
	char type;
	int mover_id;
	short x, y;
	short animation_state;
	DirectX::XMFLOAT4X4 world_pos;
};

struct sc_packet_remove_player {
	unsigned short size;
	char type;
	int leave_player_id;
};

struct sc_packet_room_info {
	unsigned short size;
	char type;
	short room_number;
	bool room_enable;
	char room_state;
	short stage_number;
	int players_id[4];
};

struct sc_packet_trap_info {
	unsigned short size;
	char type;
	unsigned short trap_index;
	unsigned short trap_local_id;
	char trap_type;
	DirectX::XMFLOAT4X4 trap_pos;
};

struct sc_packet_join_room_ok {
	unsigned short size;
	char type;
	int id;
	short room_number;
	int players_id[4];
};

struct sc_packet_make_room_ok {
	unsigned short size;
	char type;
	int id;
	short room_number;
};

struct sc_packet_monster_pos {
	unsigned short size;
	char type;
	MONSTER monsterArr[100];
};

struct sc_packet_game_end {
	unsigned short size;
	char type;
	bool clear;
};

struct sc_packet_stat_change {
	unsigned short size;
	char type;
	int id;
	short hp;
	short gold;
};

struct sc_packet_game_info_update {
	unsigned short size;
	char type;
	int id;
	short wave;
	short portalLife;
};

struct sc_packet_game_start {
	unsigned short size;
	char type;
	short stage_number;
	short wave;
	short portalLife;
};

struct sc_packet_nameLogin_result {
	unsigned short size;
	char type;
	int id;
	char result;
};

struct sc_packet_leaveRoom_ok {
	unsigned short size;
	char type;
	int id;
};

struct sc_packet_remove_room {
	unsigned short size;
	char type;
	short room_number;
};

struct sc_packet_wave_end {
	unsigned short size;
	char type;
	int id;
};

struct sc_packet_monster_pos_test {
	unsigned short size;
	char type;
	unsigned short monster_id;
	char monster_type;
	unsigned short animation_state;
	DirectX::XMFLOAT4X4 world_pos;
};

struct sc_packet_wallTrapOn {
	unsigned short size;
	char type;
	unsigned short trap_index;
};


// client to server

struct cs_packet_test {
	char size;
	char type;
	int id;
};

struct cs_packet_left {
	char size;
	char type;
};

struct cs_packet_right {
	char size;
	char type;
};

struct cs_packet_up {
	char size;
	char type;
};

struct cs_packet_down {
	char size;
	char type;
};

struct cs_packet_make_room {
	char size;
	char type;
	int id;
};

struct cs_packet_request_join_room {
	char size;
	char type;
	int joiner_id;
	short room_number;
};

struct cs_packet_pos {
	char size;
	char type;
	int id;
	short animation_state;
	DirectX::XMFLOAT4X4 player_world_pos;
};

struct cs_packet_install_trap {
	char size;
	char type;
	int id;
	unsigned short trap_local_id;
	char trap_type;
	DirectX::XMFLOAT4X4 trap_pos;
};

struct cs_packet_client_state_change {
	char size;
	char type;
	int id;
	char change_state;
	short stage_number;
};

struct cs_packet_game_start {
	char size;
	char type;
	int id;
	short stage_number;
};

struct cs_packet_namelogin {
	char size;
	char type;
	int id;
	char name[11];
};

struct cs_packet_shoot {
	char size;
	char type;
	int id;
	short monster_id;
	bool headShot;
};

struct cs_packet_leaveRoom {
	char size;
	char type;
	int id;
};


#pragma pack (pop) 