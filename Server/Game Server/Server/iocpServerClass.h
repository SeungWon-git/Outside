#pragma once

#include <concurrent_queue.h>
#include <atomic>

#include "Common.h"

#include "ZombieController.h"
#include "ItemController.h"

#include "Player.h"

#include "ZombieBT.h"



// 전방 선언 -> 순환 포함 문제를 피하기 위해 (ex: static 변수 BT_INTERVAL 을 MoveTo 클래스에서 사용하기 위해)  [+] 해당 클래스 변수 여기서 사용하려면 포인터로 선언해서 사용
class Zombie;

class ZombiePathFinder;

class ZombieController;
class ItemController;

// BT
class Task;
class Selector;
class Sequence;

class Sel_Detect;
class Sel_CanSeePlayer;
class Seq_HasShouting;
class Seq_HasFootSound;
class Seq_HordeAction;
class Seq_HasInvestigated;
class Seq_NotHasLastKnownPlayerLocation;
class Seq_CanAttack;
class Seq_CanNotAttack;
class T_Attack;
class T_MoveTo;


#define ENABLE_BT_LOG	// 전처리기 디렉티브 (좀비 BT 관련 로그 cout 찍기 플래그) ==> "로그가 많아 끊김 현상이 심하니 필요할 때만 키도록;;"
#undef ENABLE_BT_LOG	// 여기 주석처리 하면 -> 활성화 / 주석 있으면 -> 비활성화

#define ENABLE_BT_NODE_LOG	// 전처리기 디렉티브 (좀비 BT Node 관련 로그 cout 찍기 플래그)
#undef ENABLE_BT_NODE_LOG	// 여기 주석처리 하면 -> 활성화 / 주석 있으면 -> 비활성화

#define ENABLE_BT_DETECT_RANDOMCHANCE_LOG	// 전처리기 디렉티브 (좀비 BT Detect RandomChance 관련 로그 cout 찍기 플래그)
#undef ENABLE_BT_DETECT_RANDOMCHANCE_LOG	// 여기 주석처리 하면 -> 활성화 / 주석 있으면 -> 비활성화

#define ENABLE_PACKET_LOG		// 전처리기 디렉티브 활성화 (패킷 송수신 관련 로그 찍기 플래그)
#undef ENABLE_PACKET_LOG		// 여기 주석처리 하면 -> 활성화 / 주석 있으면 -> 비활성화

#define ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG	// 전처리기 디렉티브 활성화 (발소리 탐지 랜덤 위치 관련 로그 찍기 플래그)
#undef ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG	// 여기 주석처리 하면 -> 활성화 / 주석 있으면 -> 비활성화

#define ENABLE_BT_FLEE_LOG	// 전처리기 디렉티브 (좀비 BT 도망치기 관련 로그 cout 찍기 플래그) 
#undef ENABLE_BT_FLEE_LOG	// 전처리기 디렉티브 (좀비 BT 도망치기 관련 로그 cout 찍기 플래그) 


using OVLP_EX = struct Overlap_ex {
	OVERLAPPED original_overlap;
	int operation;
	WSABUF wsabuf;
	char iocp_buffer[MAX_BUF_SIZE];
};

using PLAYER_INFO = struct Client_INFO {
	SOCKET s;
	unsigned int id;
	bool connected;
	OVLP_EX recv_overlap;
	int packet_size;
	int previous_size;
	char packet_buff[MAX_BUF_SIZE];
	bool isInGame;
	int pingcnt = 0;
	bool send_zombie = false;
	bool send_item = false;
	bool send_car = false;
	bool send_game_clear = false;
	int roomid = 0;
	
	// 전송 대기열 추가
	Concurrency::concurrent_queue<std::string> sendQueue;  // 전송할 데이터를 담는 대기열
	Concurrency::concurrent_queue<std::string> recvQueue;  // 수신한 데이터를 담는 대기열
	std::mutex sendMutex;  // 큐 접근 제어용 락
	atomic<bool> isSending = false;  // 현재 전송 중인지 여부를 나타내는 플래그
};

struct GameSession {
	unsigned int sessionID;                       // 세션 ID
	std::vector<PLAYER_INFO*> players;            // 세션에 포함된 플레이어
	std::vector<Zombie*> zombies;                 // 세션에 포함된 좀비
	std::mutex sessionMutex;                      // 동기화를 위한 뮤텍스
	bool isActive = true;                         // 세션 활성화 여부
};

struct Vector3D {
	float x, y;
	FLOOR floor;
};

struct RoomState {
	int roofkey_cnt{};
	std::string Root_Open_Player = "None";
	Vector3D Escape_Location;
	bool b_IsEscaping = false;
	int Escape_Root{};
};


extern std::unordered_map<int, ZombieBT> zombie_bt_map; 
extern std::unordered_map<int, RoomState> room_states; // 방 ID -> RoomState


extern std::unordered_map<unsigned int, PLAYER_INFO*> g_players;// room_players 와 비슷한데 room에 따라 저장을 안하고 그냥 쭉 연속해서 저장함 & 가장 먼저 생성함 (+ id배정은 1에서 부터 쭉 연속해서, 중간에 삭제되어도 이전 마지막 숫자에서 이어서 할당)
extern std::unordered_map<int, std::unordered_map<int, PLAYER_INFO*>> room_players; // room에 따라 플레이어 정보 저장 (패킷통신에 필요한 정보들) (+ id배정은 g_players와 같이 1부터 쭉 연속해서)
extern std::unordered_map<int, std::unordered_map<int, Player>> playerDB;  // room에 따라서 플레이어 정보 저장 (인게임에서 필요한 정보들) (+ id배정은 g_players와 같이 1부터 쭉 연속해서)
extern std::unordered_map<int, std::unordered_map<int, Player>> playerDB_BT;	// 송수신 쓰레드와 BT 쓰레드간 데이터 레이스 방지용으로 사용
extern std::unordered_map<int, ZombieController*> zombieControllers;
extern std::unordered_map<int, ItemController*> itemControllers;

struct TupleHash {
	size_t operator()(const tuple<float, float, float>& t) const {
		size_t h1 = hash<float>{}(get<0>(t));
		size_t h2 = hash<float>{}(get<1>(t));
		size_t h3 = hash<float>{}(get<2>(t));
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

struct TupleEqual {
	bool operator()(const tuple<float, float, float>& t1, const tuple<float, float, float>& t2) const {
		return t1 == t2;
	}
};


extern std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapB2;
extern std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapB1;
extern std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapF1;
extern std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapF2;

extern std::vector<tuple<float, float, float>> g_valispositionsB2;
extern std::vector<tuple<float, float, float>> g_valispositionsB1;
extern std::vector<tuple<float, float, float>> g_valispositionsF1;
extern std::vector<tuple<float, float, float>> g_valispositionsF2;

// ostream 연산자 오버로딩 선언
std::ostream& operator<<(std::ostream& os, FLOOR floor);


class IOCP_CORE
{
public:
	IOCP_CORE();
	~IOCP_CORE();

	void IOCP_GetServerIpAddress();
	void IOCP_Initialize();
	void CheckThisCPUcoreCount();

	void IOCP_MakeWorkerThreads();
	void IOCP_WorkerThread();
	void IOCP_AcceptThread();

	void DisconnectClient(unsigned int clientId);

	bool IOCP_ProcessPacket(int id, const std::string& packet);
	void Send_GameEnd(int alive_cnt, int dead_cnt, int bestkill_cnt, std::string bestkill_player, int roomid);

	void AddPlayerToRoom(int roomId, PLAYER_INFO* clientInfo);

	void IOCP_SendNextPacket(PLAYER_INFO* user);
	void IOCP_SendPacket(unsigned int id, const char* serializedData, size_t dataSize);

	void IOCP_ErrorDisplay(const char *msg, int err_no, int line);
	void IOCP_ErrorQuit(const wchar_t *msg, int err_no);

	bool LoadEdgesMap(const string& filePath,
		vector<tuple<float, float, float>>& positions,
		unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash>& EdgesMap);

	bool UpdateEdgesMap(const string& originalFilePath, const string& copiedFilePath);

	void SendPingToClients();

	unsigned int CreateSession();
	void DeleteSession(unsigned int sessionID);

	void Zombie_BT_Thread(int roomid);

	void GameTimerEndCheck(int roomid, float& GameTime, std::chrono::steady_clock::time_point currentTime, std::chrono::steady_clock::time_point& lastGTTime, std::chrono::steady_clock::time_point& lastGameClearSendTime);

	void ServerOn();

	//서버가 먼저 켜지고 좀비 BT가 실행되도록 하기위해 사용 
	bool bServerOn;

	//======Zombie_BT ======
	
	//좀비 벡터
	std::unordered_map<int, vector<Zombie*>> zombieDB;
	std::unordered_map<int, vector<Zombie*>> zombieDB_BT;

	static float BT_INTERVAL;		// BT 작동 인터벌 설정
	static float GAME_TIMER_INTERVAL;	// 게임 타이머 시간 누적 인터벌 설정

	static std::chrono::duration<float> BT_deltaTime;
	static std::chrono::duration<float> GT_deltaTime;

	//======================



private:
	HANDLE g_hIocp;	
	int cpuCore;

	vector<thread*> worker_threads;

	vector<thread> zombie_threads;

	bool ServerShutdown{ false };

	unsigned int playerIndex{ UINT_MAX };

	bool b_Timer = false;

	std::mutex g_sessionsMutex;

	//int player_loading_cnt = 0;
	//bool all_player_loading = false;
};