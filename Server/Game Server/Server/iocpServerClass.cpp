#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

#include "iocpServerClass.h"

#include "ZombiePathfinder.h"

//BT
#include "Task.h"
#include "Selector.h"
#include "Sequence.h"

#include "Detect.h"
#include "CanSeePlayer.h"
#include "HasShouting.h"
#include "HasFootSound.h"
#include "HordeAction.h"
#include "HasInvestigated.h"
#include "NotHasLastKnownPlayerLocation.h"
#include "CanAttack.h"
#include "CanNotAttack.h"
#include "Attack.h"
#include "MoveTo.h"


std::unordered_map<int, ZombieBT> zombie_bt_map;
std::unordered_map<int, RoomState> room_states;
std::unordered_map<unsigned int, PLAYER_INFO*> g_players;	
std::unordered_map<int, std::unordered_map<int, PLAYER_INFO*>> room_players;	
std::unordered_map<int, std::unordered_map<int, Player>> playerDB;	
std::unordered_map<int, std::unordered_map<int, Player>> playerDB_BT;

std::unordered_map<int, ZombieController*> zombieControllers;
std::unordered_map<int, ItemController*> itemControllers;

std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapB2;
std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapB1;
std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapF1;
std::unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash> g_EdgesMapF2;

std::vector<tuple<float, float, float>> g_valispositionsB2;
std::vector<tuple<float, float, float>> g_valispositionsB1;
std::vector<tuple<float, float, float>> g_valispositionsF1;
std::vector<tuple<float, float, float>> g_valispositionsF2;


float IOCP_CORE::BT_INTERVAL = 0.05f;	// BT 작동 인터벌 설정 (0.05초)
float IOCP_CORE::GAME_TIMER_INTERVAL = 0.005f;	// 게임 타이머 시간 누적 인터벌 설정 (0.005초 - 5ms)

std::chrono::duration<float> IOCP_CORE::BT_deltaTime;	// BT 작동 인터벌 체크 & MoveTo에서 계산용으로 사용
std::chrono::duration<float> IOCP_CORE::GT_deltaTime;	// 게임 타이머 누적용 deltaTime

bool UPDATEMAP = false;		// 맵 txt 업데이트 유무


IOCP_CORE::IOCP_CORE()
{	
	playerIndex = 0;

	bServerOn = false;

	string filePath;
	string unrealFilePath = "../../../unreal/Project/";

	filePath = "EdgesB2.txt";
	UpdateEdgesMap(unrealFilePath + filePath, filePath);
	LoadEdgesMap(filePath, g_valispositionsB2, g_EdgesMapB2);

	filePath = "EdgesB1.txt";
	UpdateEdgesMap(unrealFilePath + filePath, filePath);
	LoadEdgesMap(filePath, g_valispositionsB1, g_EdgesMapB1);

	filePath = "EdgesF1.txt";
	UpdateEdgesMap(unrealFilePath + filePath, filePath);
	LoadEdgesMap(filePath, g_valispositionsF1, g_EdgesMapF1);

	filePath = "EdgesF2.txt";
	UpdateEdgesMap(unrealFilePath + filePath, filePath);
	LoadEdgesMap(filePath, g_valispositionsF2, g_EdgesMapF2);

	IOCP_GetServerIpAddress();
	CheckThisCPUcoreCount();

	IOCP_Initialize();
	
	IOCP_MakeWorkerThreads();
}

IOCP_CORE::~IOCP_CORE()
{
	WSACleanup();
}

void IOCP_CORE::IOCP_GetServerIpAddress()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	PHOSTENT hostinfo;
	char hostname[50];
	char ipaddr[50];
	memset(hostname, 0, sizeof(hostname));
	memset(ipaddr, 0, sizeof(ipaddr));

	int nError = gethostname(hostname, sizeof(hostname));
	if (nError == 0)
	{
		hostinfo = gethostbyname(hostname);
		strcpy(ipaddr, inet_ntoa(*reinterpret_cast<struct in_addr*>(hostinfo->h_addr_list[0])));
	}
	WSACleanup();
	printf("*/GAME SERVER/*\n");
	printf("This Server's IP address : %s\n", ipaddr);
}

void IOCP_CORE::IOCP_Initialize()
{

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (g_hIocp == NULL) {
		int err_no = WSAGetLastError();
		IOCP_ErrorQuit(L"IOCP_CORE::IOCP_Initialize", err_no);
	}
}

void IOCP_CORE::CheckThisCPUcoreCount()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	cpuCore = static_cast<int>(si.dwNumberOfProcessors);
	printf("CPU Core Count = %d, threads = %d\n", cpuCore, cpuCore);


}

void IOCP_CORE::IOCP_MakeWorkerThreads()
{
	worker_threads.reserve(cpuCore);

	for (int i = 0; i < cpuCore; ++i)
	{
		worker_threads.push_back(new thread{ &IOCP_CORE::IOCP_WorkerThread, this });
	}

	ServerOn();

	thread acceptThread{ &IOCP_CORE::IOCP_AcceptThread, this };
	while (ServerShutdown) { Sleep(1000); }

	for (auto thread : worker_threads) {
		thread->join();
		delete thread;
	}

	acceptThread.join();

	//zombie_thread.join();
}

void IOCP_CORE::IOCP_WorkerThread() {
	while (TRUE == (!ServerShutdown)) {
		DWORD key;
		DWORD iosize;
		OVLP_EX* my_overlap;

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, (PULONG_PTR)&key, reinterpret_cast<LPOVERLAPPED*>(&my_overlap), INFINITE);

		auto client_it = g_players.find(key);
		if (client_it == g_players.end()) {
			// 유효하지 않은 클라이언트 키에 대한 처리, 예: 로그 출력, 계속하기
			continue;
		}
		PLAYER_INFO* user = client_it->second;

		// 클라이언트가 접속을 끊었을 경우
		if (FALSE == result || 0 == iosize) {
			if (FALSE == result) {
				int err_no = WSAGetLastError();
				IOCP_ErrorDisplay("WorkerThreadStart::GetQueuedCompletionStatus", err_no, __LINE__);
			}

			if (user->connected) {
				closesocket(user->s);
				user->connected = false;
				DisconnectClient(user->id);
				printf("[ No. %3u ] Disconnected\n", key);
			}

			continue;
		}
		else if (OP_SERVER_RECV == my_overlap->operation) {
			// 클라이언트로부터 데이터를 받았을 경우
			char* buf_ptr = reinterpret_cast<char*>(user->recv_overlap.iocp_buffer);
			int remained = iosize;

			// 남은 데이터를 처리하는 로직
			while (0 < remained) {
				// 버퍼에 데이터를 추가
				int copySize = min(remained, MAX_PACKET_SIZE - user->previous_size);
				memcpy(user->packet_buff + user->previous_size, buf_ptr, copySize);
				user->previous_size += copySize;
				buf_ptr += copySize;
				remained -= copySize;
				
				// 더 이상 처리할 패킷이 없을 때까지 반복
				while (true) {
					// 최소 4바이트(길이 헤더) 이상 있어야 패킷 크기를 알 수 있음
					if (user->previous_size < 4) {
						break;
					}

					// 패킷 길이 읽기 (4바이트)
					uint32_t packetSize = 0;
					memcpy(&packetSize, user->packet_buff, sizeof(packetSize));

					// 유효성 검사
					// 1) packetSize가 너무 크면 에러
					if (packetSize > MAX_PACKET_SIZE - 4) {
						// 비정상 패킷 처리. 연결 종료 등
						printf("Invalid packetSize: %u\n", packetSize);
						user->previous_size = 0; // 데이터 초기화
						break;
					}

					// 2) 아직 전체 패킷(4 + packetSize)을 다 못 받았으면 다음 수신 대기
					if (user->previous_size < (4 + packetSize)) {
						break;
					}

					// [4 + packetSize] 만큼이 완성된 패킷
					// 패킷 데이터 추출
					std::string fullPacket((char*)user->packet_buff, 4 + packetSize);

					// fullPacket에서 4바이트 헤더 제외, 실제 페이로드
					std::string payload = fullPacket.substr(4);

					// 패킷 처리
					IOCP_ProcessPacket(user->id, payload);

					// leftover(남은 데이터) 이동
					int leftover = user->previous_size - (4 + packetSize);
					if (leftover > 0) {
						memmove(user->packet_buff, user->packet_buff + (4 + packetSize), leftover);
					}
					user->previous_size = leftover;
				}
			}

			// 다음 데이터를 받기 위한 WSARecv 호출
			DWORD flags = 0;
			int retval = WSARecv(user->s, &user->recv_overlap.wsabuf, 1, NULL, &flags, &user->recv_overlap.original_overlap, NULL);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) {
					IOCP_ErrorDisplay("WorkerThreadStart::WSARecv", err_no, __LINE__);
				}
				continue;
			}
		}
		else if (OP_SERVER_SEND == my_overlap->operation) {
			// 서버에서 메세지를 보냈으면, 메모리를 해제해 준다.

			IOCP_SendNextPacket(user);

			delete my_overlap;
		}
		else {
			cout << "Unknown IOCP event !!\n";
			exit(-1);
		}
	}
}


void IOCP_CORE::IOCP_AcceptThread()
{
	int retval{ 0 };

	// socket() - IPv4 ( AF_INET )
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) {
		int err_no = WSAGetLastError();
		IOCP_ErrorQuit(L"socket()", err_no);
	};

	// bind()
	struct sockaddr_in serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = ::bind(listen_sock, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		IOCP_ErrorQuit(L"socket()", err_no);
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		IOCP_ErrorQuit(L"socket()", err_no);
	}

	while (TRUE == (!ServerShutdown)) {
		// accept()
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		if (INVALID_SOCKET == client_sock) {
			int err_no = WSAGetLastError();
			IOCP_ErrorDisplay("Accept::WSAAccept", err_no, __LINE__);
			while (true);
		}

		playerIndex += 1;
		printf("\n[ No. %3u ] Client IP = %s, Port = %d is Connected\n", playerIndex, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		BOOL bNoDelay = TRUE;
		retval = setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
		if (retval == SOCKET_ERROR) {
			int err_no = WSAGetLastError();
			IOCP_ErrorDisplay("setsockopt::TCP_NODELAY", err_no, __LINE__);
		}

		uint32_t packetSize = sizeof(playerIndex); // playerIndex = 4바이트
		uint32_t totalSize = 4 + packetSize;        // 길이(4바이트) + playerIndex(4바이트)

		char sendBuf[128] = { 0 }; // 임시 송신 버퍼
		// a) 4바이트 길이
		memcpy(sendBuf, &packetSize, sizeof(packetSize));
		// b) playerIndex(4바이트)
		memcpy(sendBuf + 4, &playerIndex, sizeof(playerIndex));

		WSABUF wsabuf;
		wsabuf.buf = sendBuf;
		wsabuf.len = totalSize; // 4 + 4 = 8바이트

		DWORD sent = 0;
		OVERLAPPED sendOverlap;
		ZeroMemory(&sendOverlap, sizeof(sendOverlap));

		retval = WSASend(client_sock, &wsabuf, 1, &sent, 0, &sendOverlap, NULL);
		if (retval == SOCKET_ERROR) {
			int err_no = WSAGetLastError();
			IOCP_ErrorQuit(L"id send()", err_no);
		}

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_hIocp, playerIndex, 0);

		PLAYER_INFO* user = new PLAYER_INFO;

		user->s = client_sock;
		user->connected = true;
		user->id = playerIndex;
		user->packet_size = 0;
		user->previous_size = 0;
		memset(&user->recv_overlap.original_overlap, 0, sizeof(user->recv_overlap.original_overlap));
		user->recv_overlap.operation = OP_SERVER_RECV;
		user->recv_overlap.wsabuf.buf = reinterpret_cast<char*>(&user->recv_overlap.iocp_buffer);
		user->recv_overlap.wsabuf.len = sizeof(user->recv_overlap.iocp_buffer);
		user->isInGame = false;

		//* g_players 새로 생성
		g_players[user->id] = user;

		/* 주변 클라이언트에 대해 뿌릴 정보 뿌리고, 시야 리스트나 처리해야 할 정보들도 함께 넣는다. */

		// 클라이언트에서 응답오길 기다리기
		DWORD flags{ 0 };
		retval = WSARecv(client_sock, &user->recv_overlap.wsabuf, 1, NULL, &flags, &user->recv_overlap.original_overlap, NULL);
		if (SOCKET_ERROR == retval) {
			int err_no = WSAGetLastError();
			if (ERROR_IO_PENDING != err_no) {
				IOCP_ErrorDisplay("Accept::WSARecv", err_no, __LINE__);
			}
		}
	}
}

void IOCP_CORE::DisconnectClient(unsigned int id) {
    if (g_players.find(id) != g_players.end()) {
        PLAYER_INFO *user = g_players[id];

		room_players[user->roomid].erase(id);
		playerDB[user->roomid].erase(id);

        delete user;
		g_players.erase(id);
    }
}


void IOCP_CORE::IOCP_SendPacket(unsigned int id, const char* serializedData, size_t dataSize)
{
	auto it = g_players.find(id);
	if (it == g_players.end() || !(it->second->connected)) {
		return;
	}

	PLAYER_INFO* user = it->second;

	// Lock-free queue에 데이터 삽입
	user->sendQueue.push(std::string(serializedData, dataSize));

	// 현재 전송 중이 아닌 경우에만 전송 시작
	bool expected = false;
	if (user->isSending.compare_exchange_strong(expected, true)) {
		// 전송 시작 플래그를 true로 설정 후 전송 작업 시작
		IOCP_SendNextPacket(user);
	}
}

void IOCP_CORE::IOCP_SendNextPacket(PLAYER_INFO* user)
{
	std::string data;
	if (!user->sendQueue.try_pop(data)) {
		user->isSending = false;
		return;
	}

	// 1) 패킷 크기(4바이트) + 실제 데이터 -> 전체 길이
	uint32_t packetSize = static_cast<uint32_t>(data.size());
	uint32_t totalSize = 4 + packetSize;

	// 2) Overlapped 구조체 세팅
	OVLP_EX* over = new OVLP_EX;
	memset(over, 0, sizeof(OVLP_EX));
	over->operation = OP_SERVER_SEND;

	// 3) iocp_buffer에 [4바이트 길이 + 데이터]를 복사
	//    iocp_buffer가 totalSize보다 크다고 가정 (안전성 체크 필요)
	memcpy(over->iocp_buffer, &packetSize, sizeof(packetSize));          // 길이 4바이트
	memcpy(over->iocp_buffer + 4, data.data(), data.size());            // 실제 데이터

	over->wsabuf.buf = reinterpret_cast<char*>(over->iocp_buffer);
	over->wsabuf.len = totalSize;  // 4 + 데이터 길이

	// 4) 비동기 송신
	DWORD flags = 0;
	int retval = WSASend(user->s, &over->wsabuf, 1, NULL, flags, &over->original_overlap, NULL);
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING) {
			IOCP_ErrorDisplay("SendPacket::WSASend", err_no, __LINE__);
		}
	}
}


void IOCP_CORE::IOCP_ErrorDisplay(const char *msg, int err_no, int line)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[ %s - %d ]", msg, line);
	wprintf(L"에러 %s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void IOCP_CORE::IOCP_ErrorQuit(const wchar_t *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// ostream 연산자 오버로딩 - FLOOR cout 출력용
std::ostream& operator<<(std::ostream& os, FLOOR floor) 
{
	switch (floor) {
	case FLOOR::FLOOR_B2:       os << "B2"; break;
	case FLOOR::FLOOR_B1:       os << "B1"; break;
	case FLOOR::FLOOR_F1:       os << "F1"; break;
	case FLOOR::FLOOR_F2:       os << "F2"; break;
	case FLOOR::FLOOR_F3:       os << "F3"; break;
	default:                    os << "Unknown FLOOR"; break;
	}
	return os;
}

void IOCP_CORE::ServerOn()
{
	cout << endl;

	cout << std::setfill(' ') << std::showpoint << std::fixed << std::setprecision(2);		// 출력 칸 맞추기 (소수점 2자리까지)


#ifdef	ENABLE_BT_LOG
	cout << "<< 좀비 BT 관련 로그 출력 ON >>" << endl;
#endif

#ifdef ENABLE_BT_NODE_LOG
	cout << "<< 좀비 BT Node 관련 로그 출력 ON >>" << endl;
#endif

#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
	cout << "<< 좀비 BT Detect RandomChance 관련 로그 출력 ON >>" << endl;
#endif

#ifdef ENABLE_PACKET_LOG
	cout << "<< Packet 송수신 관련 로그 출력 ON >>" << endl;
#endif

#ifdef 	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
	cout << "<< 발소리 탐지 랜덤 위치 관련 로그 츨력 ON >>" << endl;
#endif

#ifdef 	ENABLE_BT_FLEE_LOG
	cout << "<< 좀비 BT 도망치기 관련 로그 츨력 ON >>" << endl; 
#endif

	//for (const auto player : playerDB) {
	//	float p_x = player.second.x;
	//	float p_y = player.second.y;
	//	float p_z = player.second.z;
	//
	//	//이거 플레이어는 서버 초기화 후에 추가되서 하나도 안 찍힘 (따로 찍고 싶으면 플레그 값 설정해서 나중에 플레이어 다 받으면 찍어야 할 듯)
	//	cout << "플레이어 \'#" << player.first << "\' 의 시작 위치: ( "
	//		<< std::setw(8) << p_x << ", " << std::setw(8) << p_y << ", " << std::setw(8) << p_z << " )" << endl;
	//}

	cout << endl;

	/*for (const auto zom : zombieDB]) {
		float z_x = zom->ZombieData.x;
		float z_y = zom->ZombieData.y;
		float z_z = zom->ZombieData.z;

		cout << "좀비 \'#" << zom->ZombieData.zombieID << "\' 의 시작 위치: ( "
			<< std::setw(8) << z_x << ", " << std::setw(8) << z_y << ", " << std::setw(8) << z_z << " ) , HP: " << zom->GetHP() << " , floor: "  << zom->z_floor << endl;
	}*/

	cout << endl;

	bServerOn = true;
}

void IOCP_CORE::GameTimerEndCheck(int roomid, float& GameTime, std::chrono::steady_clock::time_point currentTime, std::chrono::steady_clock::time_point& lastGTTime)
{
	GT_deltaTime = currentTime - lastGTTime;

	// 게임 타이머 계산 
	if (GT_deltaTime.count() > GAME_TIMER_INTERVAL) {	// 5ms 이상 경과 시만 시간 누적 => 이 설정 없으면 시간이 훨어얼씬 더 빨리 측정됨
		// deltaTime을 누적하여 GameTime에 더함
		GameTime += GT_deltaTime.count();  // 초 단위 (* -> 그냥 이렇게 부동소수점 누적하면, 나중에 시간 지날 수록 정확도 떨어짐)
		lastGTTime = currentTime;
	}

	//cout << "게임 경과시간: " << GameTime << "초" << endl;

	 // 게임시간이 10분을 넘으면 게임오버 엔딩 점수판 띄우게하기
	if (GameTime >= 10 * 60.f) {
		// 점수판 계산
		int alive_cnt = 0;
		int dead_cnt = 0;
		int disconnected = 0;
		int bestkill_cnt = 0;
		std::string bestkill_player = "None";

		for (const auto player : playerDB[roomid]) {
			if (g_players.find(player.first) == g_players.end()) { // 연결이 끊긴 플레이어라면  
				disconnected++;
				if (bestkill_cnt < player.second.killcount) {   // 연결이 끊겼어도 젤 마니 좀비를 죽였을 수도 있으니
					bestkill_cnt = player.second.killcount;
					bestkill_player = player.second.username;
				}
				continue;
			}

			// 게임 시간 초과 엔딩은 그냥 모든 플레이어가 실패라고 띄워야해서
			dead_cnt++;

			if (bestkill_cnt < player.second.killcount) {
				bestkill_cnt = player.second.killcount;
				bestkill_player = player.second.username;
			}
		}

		room_states[roomid].Escape_Root = 0;    // 탈출방법 0(실패)으로 초기화 => 이전에 문을 연적이 있으면 해당 변수 갱신되서, 게임오버에서 탈출방법 실패가 안뜸;;

		// 전송작업
		// 세션 나누면서 마지막 인자 추가해야할듯.. 지금 상태에서는 서버에 접속한 전부에게 전송
		Send_GameEnd(alive_cnt, dead_cnt, bestkill_cnt, bestkill_player, roomid);
	}
}

void IOCP_CORE::Zombie_BT_Thread(int roomid)
{
	//========작업 실행==========

	string result = "Initial";
	
	float GameTime = 0.f;
	std::chrono::steady_clock::time_point currentTime;
	std::chrono::steady_clock::time_point lastBTTime = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point lastGTTime = std::chrono::high_resolution_clock::now();

	
	while (true) {
		// BT 작동 여부 판단
		{
			// 서버가 먼저 켜지고 좀비 BT가 실행되도록
			if (bServerOn == false)
				continue;
			// (아무) 플레이어가 서버로 접속하면 b_Timer => true (로딩 중도 포함임)
			if (b_Timer == false) 
				continue;

			// 연결된 플레이어가 있는지 검사
			if (playerDB[roomid].size() == 0) {
				//cout << "연결된 플레이어가 없습니다... => (playerDB_BT.size() == 0)" << endl;
				//cout << endl;
				result = "NO PLAYER"; 
			}
			else {
#ifdef	ENABLE_BT_LOG
				//for (auto player : playerDB_BT) {
				//	float p_x = player.second.x;					float p_y = player.second.y;					float p_z = player.second.z;
				//	cout << "플레이어 \'#" << player.first << "\' 의 현재 위치: ( " << p_x << ", " << p_y << ", " << p_z << " )" << endl;
				//	//cout << endl;
				//}
				//cout << endl;
#endif
				result = "HAS PLAYER";

				// 접속이 끊긴 것도 검사
				if (room_players[roomid].size() == 0) {
					cout << "방-" << roomid << " 에 더이상 연결된 플레이어가 없습니다... => (room_players[roomid].size() == 0)" << endl;
					cout << endl;

					result = "NO PLAYER";
					break;			// 완전히 BT 쓰레드 종료시킴
				}
				else {
					result = "HAS PLAYER";
				}
			}

		}

		currentTime = std::chrono::high_resolution_clock::now();
		// 게임 10분 타이머 계산
		GameTimerEndCheck(roomid, GameTime, currentTime, lastGTTime);

		// BT 작동 인터벌 설정
		BT_deltaTime = currentTime - lastBTTime;
		if (BT_deltaTime.count() < BT_INTERVAL) {
			continue;
		}
		lastBTTime = currentTime;
		
		// BT-송수신 쓰레드간의 데이터 레이스 방지를 위해
		zombieDB_BT[roomid] = zombieDB[roomid];
		playerDB_BT[roomid] = playerDB[roomid];


		// 좀비가 있을때 BT 실행
		for (auto& zom : zombieDB_BT[roomid]) {

			// 플레이어가 있을때 BT 실행
			if (result == "NO PLAYER")
				break;

#ifdef	ENABLE_BT_LOG
			if (zom->printLog == true) {
				cout << endl;
				cout << "//========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 실행==========" << endl;
				cout << "게임 경과시간: " << GameTime << "초" << endl;
				cout << endl;
			}
#endif

			// 좀비가 사망시 BT 중지
			if (zom->zombieHP <= 0.f) {
				// 부활 타이머 계산
				auto deadAfterTime = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> deltaTime = deadAfterTime - zom->resurrectionStartTime;

#ifdef ENABLE_BT_LOG
				cout << "### [Resurrect] 호출" << endl;
#endif

				if (deltaTime.count() >= zom->resurrectionTimer) {
					zom->Resurrect();
					continue;
				}
				else {
#ifdef ENABLE_BT_LOG
					cout << "좀비 '#" << zom->ZombieData.zombieID << "' 부활중" << endl;
					cout << "부활 남은 시간: " << zom->resurrectionTimer - deltaTime.count() << "s" << endl;
#endif
				}


#ifdef	ENABLE_BT_LOG
				if (zom->printLog == true) {
					cout << "좀비 \'#" << zom->ZombieData.zombieID << "\' 사망함." << endl << endl;
					cout << "==========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 종료========//" << endl;
					cout << endl;

					zom->printLog = false;
				}
#endif
				continue;
			}

			// 좀비가 대기상태라면 해당 좀비 BT 잠시 대기
			if (zom->HaveToWait == true) {
				zom->Wait();
#ifdef	ENABLE_BT_LOG
				cout << "==========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 종료========//" << endl;
				cout << endl;
#endif
				continue;
			}

			// 좀비 도망가기 상태
			if (zom->IsRunaway == true) {
				zom->Flee();
#ifdef	ENABLE_BT_FLEE_LOG
				cout << endl;
#endif
#ifdef	ENABLE_BT_LOG
				cout << "==========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 종료========//" << endl;
				cout << endl;
#endif
				continue;
			}

			// 좀비가 플레이어들이 없는 층에 있다면 좀비 BT 실행 멈추고 있기
			bool same_floor = false;
			//cout << "좀비 #" << zom->ZombieData.zombieID << " 의 z_floor: " << zom->z_floor << endl;
			//cout << "좀비 #" << zom->ZombieData.zombieID << " 의 pathfinder.floor: " << zom->pathfinder.floor << endl;
			for (auto player : playerDB_BT[roomid]) {
				//cout << "플레이어 #" << player.first << " 의 floor: " << player.second.floor << endl;
				if (zom->z_floor == player.second.floor && g_players.find(player.first) != g_players.end()) {	// 좀비 있는 층에 플레이어 있고, 해당 플레이어 서버랑 연결되어 있는 상태라면
					same_floor = true;
					//cout << endl;
					break;
				}
			}
			//cout << endl;

			if (same_floor == false) {
#ifdef	ENABLE_BT_LOG
				if (zom->printLog == true) {
					cout << "좀비 \'#" << zom->ZombieData.zombieID << "\' 플레이어들이 없는 층에 존재. -> BT 실행 잠시 중지" << endl << endl;
					cout << "==========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 종료========//" << endl;
					cout << endl;

					zom->printLog = false;
				}
#endif
				continue;	// 같은 층에 플레이어가 아무도 없으니 BT 스킵
			}
#ifdef	ENABLE_BT_LOG
			else if (same_floor == true) {
				if (zom->printLog == false) {
					cout << endl;
					cout << "//========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 실행==========" << endl;
					cout << endl;

					zom->printLog = true;
				}
			}
#endif


#ifdef	ENABLE_BT_LOG
			float z_x = zom->ZombieData.x;					float z_y = zom->ZombieData.y;					float z_z = zom->ZombieData.z;
			cout << "좀비 \'#" << zom->ZombieData.zombieID << "\' 의 현재 위치: ( " << z_x << ", " << z_y << ", " << z_z << " )" << endl;
			cout << endl;
#endif


			//==== 실제 BT 실행 ====//
			
			zombie_bt_map[roomid].t_moveto->d_result = false;	// moveto task (실행여부)결과값 초기화 (나중에 moveto 우선순위 구현하면 지워야함)
			zombie_bt_map[roomid].sel_detect->Detect(*zom);

			//==== 실제 BT 종료 ====//


#ifdef	ENABLE_BT_LOG
			//z_x = zom->ZombieData.x;					z_y = zom->ZombieData.y;					z_z = zom->ZombieData.z;
			//cout << "좀비 \'#" << zom->ZombieData.zombieID << "\' 의 새로운 위치: ( " << z_x << ", " << z_y << ", " << z_z << " )" << endl;
			//cout << endl;
#endif

#ifdef	ENABLE_BT_LOG
			cout << "==========좀비 \'#" << zom->ZombieData.zombieID << "\' BT 종료========//" << endl;
			cout << endl;
#endif
		}


		// send_path 통신작업
		for (const auto player : playerDB_BT[roomid]) {
			if (g_players.find(player.first) == g_players.end()) { // 연결이 끊긴 플레이어라면
				continue;	// 보내지 X
			}
			else if (g_players[player.first]->connected == false) { // 연결이 끊긴 플레이어라면
				continue;	// 보내지 X
			}
			else if (g_players[player.first]->send_zombie == false || g_players[player.first]->send_item == false || g_players[player.first]->send_car == false) {	// 아직 월드 생성 중이라면
				continue;	// 아직 보내지 X
			}

			Protocol::ZombiePathList zPathList;
			zPathList.set_packet_type(10);

			for (const auto zom : zombieDB_BT[roomid]) {
				if (player.second.floor == zom->z_floor) {	// 플레이어 같은 층에 있는 좀비의 path만 받음
					//cout << "<<방 #" << roomid << " - 좀비 #" << zom->ZombieData.zombieID << ">>" << endl;

					// path 보낼 필요 없는 좀비들 예외처리 (최적화)
					if (
						zom->GetHP() <= 0.f	// 죽은 좀비
						|| zom->path.empty() // path가 설정 안된 좀비
						|| (zom->ZombiePathIndex >= zom->path.size() || (zom->ZombieData.x == zom->TargetLocation[0][0][0] && zom->ZombieData.y == zom->TargetLocation[0][0][1]))	 // 이미 도착지점에 있는 좀비
						|| (zom->IsStandingStill == true && zom->targetType == zom->PATROL)	// 숨고르기 할때'만' 패킷 보내지 X -> 제자리 걸음 방지 & 숨고르기에서 피격 당하고 블랙보드 클리어 할때는 보내야 하니까
						|| (zom->IsStandingStill == true && zom->targetType == zom->RUNAWAY)	// 도망치기 이후 숨어서 숨고르기 하는 중인 좀비
						//|| zom->HaveToWait == true	/* 이러면, 다른 층에서 있던 플레이어 애니메이션 재생 중이던 좀비 위치를 갱신 못 받아서 이상함 */
						) {
						if (zom->targetType != zom->BLACKBOARDCLEARED	// 다만 블랙보드 클리어는 보내야 하니 제외
							&& zom->targetType != zom->LOOKINGAROUND)	// 다만 주위 둘러보기는 보내야 하니 제외
						{	
							continue;
						}
					}

					if (zom->HaveToWait == true) {
						zom->targetType = zom->TARGET::WAIT;	// 현재 애니메이션 재생 중임을 알림
					}

					Protocol::ZombiePath* zPath = zPathList.add_zombiepaths();
					zPath->set_zombieid(zom->ZombieData.zombieID);
					zPath->set_packet_type(10);
					zPath->set_target_type(zom->targetType);

					// Path1 설정
					Protocol::Vector3* Destination1 = zPath->mutable_path1();
					Destination1->set_x(get<0>(zom->path[zom->ZombiePathIndex]));
					Destination1->set_y(get<1>(zom->path[zom->ZombiePathIndex]));
					Destination1->set_z(get<2>(zom->path[zom->ZombiePathIndex]));

					// Path2 설정 (다음 경로가 존재하는 경우)
					if ((zom->ZombiePathIndex + 1) < zom->path.size()) {
						Protocol::Vector3* Destination2 = zPath->mutable_path2();
						Destination2->set_x(get<0>(zom->path[zom->ZombiePathIndex + 1]));
						Destination2->set_y(get<1>(zom->path[zom->ZombiePathIndex + 1]));
						Destination2->set_z(get<2>(zom->path[zom->ZombiePathIndex + 1]));
					}

					// 현재 위치 설정
					Protocol::Vector3* currentLocation = zPath->mutable_location();
					currentLocation->set_x(zom->ZombieData.x);
					currentLocation->set_y(zom->ZombieData.y);
					currentLocation->set_z(zom->ZombieData.z);

					//cout << "<<방 #" << roomid << " SendPath 전송 완료 - 좀비 #" << zom->ZombieData.zombieID << ">>" << endl;

#ifdef	ENABLE_BT_LOG
					cout << "<<플레이어 #" << player.first << " 에게 SendPath 전송 완료 - 좀비 #" << zom->ZombieData.zombieID << ">>" << endl;
					cout << endl;
#endif
				}
			}

			std::string serializedData;
			zPathList.SerializeToString(&serializedData);

			IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
			//printf("send zombiedatalist\n");

		}

	}

	//==========================

	cout << "방-" << roomid << " 좀비 BT 종료" << endl;

	//std::cout << "Press Enter to close this room's zombieBT...";
	//std::cin.get(); // 사용자가 Enter를 입력할 때까지 대기 (콘솔 애플리케이션이 실행 후 바로 종료되면서 콘솔 창이 닫히는거 방지) 
	//-> 이제 필요 없음 (로비서버처럼 모든 플레이어가 접속을 끊어도 계속 돌아감)
	//-> 근데 갑자기 바로 꺼질때 도 있음;;; => 밑에 메모리 해제 때문이였음

	//========할당한 메모리 해제========
	//zombie_bt_map[roomid].~ZombieBT();		// ************************* 왜인지는 모르겠는데 이거 키면 겜서버 같이 꺼짐...;;

}


bool IOCP_CORE::UpdateEdgesMap(const string& originalFilePath, const string& copiedFilePath)
{
	if (UPDATEMAP == false)
		return false;

	ifstream file(originalFilePath);
	if (!file.is_open()) {
		cerr << "Cannot open file: " << originalFilePath << endl;
		return false;
	}
	
	bool result = CopyFileA(originalFilePath.c_str(), copiedFilePath.c_str(), false);

	if (result == true) {
		cout << "Update " << copiedFilePath << " complete!" << endl;
	}
	else {
		cout << "Update " << copiedFilePath << " fail!" << endl;
	}

	return result;
}

bool IOCP_CORE::LoadEdgesMap(const string& filePath, vector<tuple<float, float, float>>& positions, unordered_map<tuple<float, float, float>, vector<pair<tuple<float, float, float>, float>>, TupleHash>& EdgesMap)
{
	ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filePath << endl;
        return false;
    }

    string line;
    tuple<float, float, float> currentNode;
    tuple<float, float, float> neighborNode;
    while (getline(file, line)) {
        stringstream ss(line);

        // "Node:" 줄 처리
        if (line.find("Node:") != string::npos) {
            // Node 정보 파싱
            float x, y, z;
            char comma;
            ss.ignore(5); // "Node: " 무시
            if (ss >> x >> comma >> y >> comma >> z) {
                currentNode = make_tuple(x, y, z);
                EdgesMap[currentNode] = {};  // 새로운 노드 추가

                // validPositions에 currentNode 추가
                positions.push_back(currentNode);
            }
        }
        // "Neighbor:" 줄 처리
        else if (line.find("Neighbor:") != string::npos) {
            // Neighbor 정보 파싱
            float nx, ny, nz;
            char comma;
            ss.ignore(9); // "Neighbor: " 무시

            // x, y, z 좌표 파싱
            if (ss >> nx >> comma >> ny >> comma >> nz) {
                neighborNode = make_tuple(nx, ny, nz);
            }

            // 다음 줄에서 "Weight:" 정보를 가져오기
            if (getline(file, line)) {
                stringstream weightStream(line);
                string temp;
                float weight;

                // "Weight: " 무시하고 가중치 값 추출
                if (weightStream >> temp >> weight && temp == "Weight:") {
                    // 이웃 노드와 가중치 추가
                    EdgesMap[currentNode].emplace_back(neighborNode, weight);
                }
            }
        }
    }

    file.close();
    return true;
}

void IOCP_CORE::SendPingToClients()
{
	Protocol::ping pingpacket;

	pingpacket.set_packet_type(11);

	std::string serializedData;
	pingpacket.SerializeToString(&serializedData);

	for (auto it = g_players.begin(); it != g_players.end();)
	{
		// 유효성 검사
		if (it->second == nullptr)
			continue;

		PLAYER_INFO* player = it->second;

		if (player->connected)
		{
			IOCP_SendPacket(player->id, serializedData.data(), serializedData.size());
			player->pingcnt++;

			if (player->pingcnt >= 3)
			{
				printf("Client #%u did not respond\n", player->id);

				// 클라이언트 연결 해제
				player->connected = false;

				// 클라이언트 삭제 후, 다음 반복자를 얻음
				it = g_players.erase(it);  // erase()는 삭제된 요소의 다음 반복자를 반환
				DisconnectClient(player->id);
				continue;  // 삭제한 후, 다음 클라이언트로 이동
			}
		}
		++it;  // 삭제되지 않은 경우에만 반복자를 증가시킴
	}
}