#include <iostream>
#include <cmath>
#include <random>

#include "GStruct.pb.h"

#include "Zombie.h"
//#include "ZombiePathfinder.h"
#include "ShoutingZombie.h"

using std::cout;
using std::endl;

constexpr double PI = 3.14159265358979323846;


Zombie::Zombie()
	: pathfinder(-1,-1,0,0,0,1,1,1)
{
	iocpServer = nullptr;

	SetHP(0);

	SetSpeed(0);

	path = vector<tuple<float, float, float>>{};

	beforepath = vector<tuple<float, float, float>>{};

	ZombieData = Zombie_Data();

	DistanceTo_PlayerInsight = {};

	DistanceTo_FootSound = {};

	TargetLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };

	LastKnownTargetLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };

	HordeLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };

	PlayerInSight = false;

	KnewPlayerLocation = false;

	HeardShouting = false;

	HeardFootSound = false;

	HeardHordeSound = false;

	RandPatrolSet = false;

	IsAttacking = false;

	IsBeingAttacked = false;

	IsShouting = false;

	IsStandingStill = false;

	HaveToWait = false;

	DetermineFloor(ZombieData.z);

	//speed = 0.f;

	targetType = Zombie::TARGET::NULL_TARGET;

	ClosestPlayerID = 0;

	roomid = 0;

	SetZombieType(ZOMBIE_TYPE::NULL_TYPE);
}

Zombie::Zombie(IOCP_CORE* mainServer, Zombie_Data z_d)
	: pathfinder(z_d.roomID, z_d.zombieID, 0, 0, 0, 1, 1, 1)
{
	iocpServer = mainServer;

	SetHP(0);

	SetSpeed(0);

	path = vector<tuple<float, float, float>>{};

	beforepath = vector<tuple<float, float, float>>{};

	ZombieData = z_d;

	DistanceTo_PlayerInsight = {};

	DistanceTo_FootSound = {};

	TargetLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };

	LastKnownTargetLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };
	
	HordeLocation = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };

	PlayerInSight = false;

	KnewPlayerLocation = false;

	HeardShouting = false;

	HeardFootSound = false;

	HeardHordeSound = false;

	RandPatrolSet = false;

	IsAttacking = false;

	IsBeingAttacked = false;

	IsShouting = false;

	IsStandingStill = false;

	HaveToWait = false;

	DetermineFloor(ZombieData.z);

	//speed = 0.f;

	targetType = Zombie::TARGET::NULL_TARGET;

	ClosestPlayerID = 0;

	roomid = z_d.roomID;

	SetZombieType(ZOMBIE_TYPE::NULL_TYPE);

	makeHordeSound_StartTime = std::chrono::steady_clock::now();
}

Zombie::~Zombie()
{
	//여기 cout 해서 출력보면 왜 zombie 클래스가 생성도 되기 전인데 여러번 미리 불림?! 
	// -> 그래서 여기서 만약 new 할당된 메모리 delete 하려하면 할당된 메모리도 없는데 지울려 해서 에러 (지금은 다 없애서 없긴하지만... 미리 소멸자가 여러번 불리는 건 마찬가지)
	// -> 이유는 모르겠음 그냥
	// + 혹시 전방 선언해서?? / 복사 생성되서?? / 암시적 삭제??

	cout << "++++++++++++++++++++++ ~Zombie() 호출!!! ++++++++++++++++++++++" << endl;	// 이거 전혀 안 불리는데...
}

void Zombie::DetermineFloor(float startZ)
{
	//cout << "좀비 시작 위치 - z 좌표: " << startZ << endl;

	if (startZ < 800.f) {
		z_floor = FLOOR::FLOOR_B2;
		//cout << "floor 할당: B2" << endl;
	}
	else if (startZ < 1800.f) {
		z_floor = FLOOR::FLOOR_B1;
		//cout << "floor 할당: B1" << endl;
	}
	else if (startZ < 2500.f) {
		z_floor = FLOOR::FLOOR_F1;
		//cout << "floor 할당: F1" << endl;
	}
	else if (startZ < 3600.f) {
		z_floor = FLOOR::FLOOR_F2;
		//cout << "floor 할당: F2" << endl;
	}
	else {
		z_floor = FLOOR::FLOOR_F3;
		//cout << "floor 할당: F3" << endl;
	}
}

// distanceType = 1: Detect / 2: FootSound
float Zombie::SetDistance(int playerid, int distanceType)
{
	map<int, float>* setMap = {};

	bool spacing = false;

	if (distanceType == 1)
		setMap = &DistanceTo_PlayerInsight;
	else if (distanceType == 2)
		setMap = &DistanceTo_FootSound;

	vector<vector<vector<float>>> zl = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };
	vector<vector<vector<float>>> pl = vector<vector<vector<float>>>{ {{playerDB_BT[roomid][playerid].x, playerDB_BT[roomid][playerid].y, playerDB_BT[roomid][playerid].z}} };

	float dist = sqrt(powf(zl[0][0][0] - pl[0][0][0], 2) + powf(zl[0][0][1] - pl[0][0][1], 2) + powf(zl[0][0][2] - pl[0][0][2], 2));


	if (setMap->find(playerid) == setMap->end()) {		// map에 playerid가 없으면 -> 새로 생성
		setMap->emplace(playerid, dist);

#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		if (distanceType == 1) {
			cout << "PlayerInsight 맵에 새로운 데이터 삽입!(생성) playerid: " << playerid << " a, distance: " << dist << endl;
		}
		else {
			cout << "FootSound 맵에 새로운 데이터 삽입!(생성) playerid: " << playerid << " , distance: " << dist << endl;
		}
		spacing = true;
#endif
	}
	else if (setMap->find(playerid) != setMap->end()) {		// map에 이미 playerid가 이미 있으면 -> 수정, 갱신
		// PlayerInsight 업데이트
		if (distanceType == 1) {
			if (setMap->at(playerid) < 0) {	// 죽거나 연결 끊긴 플레이어
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG) || defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG)
				cout << "이미 포착 불가능한 (죽음/연결 끊김) 플레이어의 PlayerInsight 맵에 접근;;" << endl;
#endif
			}
			else {
				setMap->at(playerid) = dist;		// {주의} at은 해당 키값 없으면 abort() 에러 띄움 - 위에서 미리 예외처리 꼭! 필요!
				//(*setMap)[playerid] = dist;		// operator[] 이용해서 수정하기도 가능 
													// {주의} 이거 해당 키값이 없으면 자동으로 추가해주니까 조심해야함 
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)|| defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG)
				cout << "PlayerInsight 맵에 이미 있던 데이터 갱신! playerid: " << playerid << " , distance: " << dist << endl;
#endif
			}
		}
		// FootSound 업데이트
		else if (distanceType == 2) {
			if ((*setMap)[playerid] < 0) {	// 죽거나 연결 끊긴 플레이어
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
				cout << "이미 포착 불가능한 (죽음/연결 끊김) 플레이어의 FootSound 맵에 접근;;" << endl;
#endif
			}
			else {
				(*setMap)[playerid] = dist;
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
				cout << "FootSound 맵에 이미 있던 데이터 갱신! playerid: " << playerid << " , distance: " << dist << endl;
#endif
			}
		}
	}

#ifdef	ENABLE_BT_LOG
	if (spacing)
		cout << endl;
#endif

	return dist;
}

bool Zombie::RandomPatrol()
{
	float px, py, pz;

	std::random_device rd;
	std::mt19937 mt(rd());

	float max_radius = 0;
	float min_radius = 0;

	if (IsRunaway == true) {
		max_radius = 2000;
		min_radius = 500;
	}
	else {
		max_radius = 1500;
	}

	std::uniform_int_distribution<int> dist(-max_radius, max_radius);		//현 위치에서 반경 radius 만큼

	float offset_x = dist(mt);
	while (abs(offset_x) <= min_radius) {	// 최소 radius 설정
		offset_x = dist(mt);
	}

	float offset_y = dist(mt);
	while (abs(offset_y) <= min_radius) {	// 최소 radius 설정
		offset_y = dist(mt);
	}
	
	px = ZombieData.x + offset_x;
	py = ZombieData.y + offset_y;
	pz = ZombieData.z;

	while (px >= 2366.f) {		// 좀비가 계단 쪽, 그리고 그쪽 벽 너머로 자주 넘어가는 오류가 있어 이를 일단 방지하기 위해 (맵 수정 전까지는) 
								// => X 굳이? 어차피 CheckPath에서 계단 너머 쪽 포인트를 찍더라도 가장 가까운 실제 걸을 수 있는 지점을 찍을 꺼니까
								// ===> XXX 보니까 맵 바깥도 걸을 수 있는 지점이라고 표시한 곳이 몇군데 있음!!! (특히 화장실 옆쪽 - 계단 뒤) => 네비메시 일단 수정 들어감
		px = ZombieData.x + dist(mt);

		if (ZombieData.x >= 2366.f) {
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "[ERROR] 현재 좀비 걸을 수 있는 지형을 벗어남!!! (계단 쪽을 넘어감)" << endl;
			return false;
		}
	}

	while (py <= -1000.f) {		// 좀비가 화장실 뒤쪽 너머 빈 공간으로 자주 넘어가서 일단 막음
		py = ZombieData.y + dist(mt);

		if (ZombieData.y <= -1000.f) {
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "[ERROR] 현재 좀비 걸을 수 있는 지형을 벗어남!!! (화장실 뒤쪽을 넘어감)" << endl;
			return false;
		}
	}

	vector<tuple<float, float, float>> dest_test;

	// 랜덤 패트롤 지점이 갈 수 있는 곳 인지 검사
	if (CheckPath(dest_test, px, py, pz) == false) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_FLEE_LOG)
		cout << "좀비 #" << ZombieData.zombieID << " 랜덤 패트롤 지정 실패! (CheckPath 실패) -> 다시 검색!!!" << endl;
#endif
		return false;
	}

	if (dest_test.size() < 3) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_FLEE_LOG)
		cout << "좀비 #" << ZombieData.zombieID << " 새로운 패트롤 찾기 실패!!! => dest_test.size() < 3 -> 현재 지점과 너무 가까운 점" << endl;
#endif		
		return false;
	}

	if (IsRunaway == true && dest_test.size() < 8) {
#ifdef	ENABLE_BT_FLEE_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 새로운 도망치기 패트롤 포인트 찾기 실패!!! => dest_test.size() < 8 -> 현재 지점과 너무 가까운 점" << endl;
#endif		
		return false;
	}

	UpdatePath(dest_test);
	
#ifdef ENABLE_BT_LOG
	cout << "랜덤 패트롤 찾기 성공!" << endl;
	cout << "TargetLocation[Patrol]: ( " << TargetLocation[0][0][0] << " , " << TargetLocation[0][0][1] << " , " << TargetLocation[0][0][2] << " )" << endl;
#endif

	RandPatrolSet = true;

	return true;
}

void Zombie::SetTargetLocation(TARGET t)
{
	targetType = t;

	vector<vector<vector<float>>> closest_player_pos = {};

	switch (targetType) {

	case TARGET::PLAYER:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '플레이어'" << endl;
#endif
		SearchClosestPlayer(closest_player_pos, 1);
		if (closest_player_pos.size() != 0) {
			TargetLocation = closest_player_pos;
			LastKnownTargetLocation = TargetLocation;
			UpdatePath();
		}
//#ifdef	ENABLE_BT_LOG
		else {
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "[Error] 좀비 따라갈 플레이어를 찾지못함! (closest_player_pos.size() == 0)" << endl;
		}
//#endif
		break;

	case TARGET::SHOUTING:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '샤우팅'" << endl;
#endif
		TargetLocation = ShoutingLocation;
		UpdatePath();
		break;

	case TARGET::FOOTSOUND:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '발소리'" << endl;
#endif
		SearchClosestPlayer(closest_player_pos, 2);
		if (closest_player_pos.size() != 0) {
			SetRandomTargetLocation(closest_player_pos);	// 발소리 거리에 따른 탐지 랜덤위치 설정 (탐지 실수)
		}
		break;

	case TARGET::HORDESOUND:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '호드'" << endl;
#endif
		TargetLocation = HordeLocation;
		UpdatePath();
		break;

	case TARGET::INVESTIGATED:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '이전 발견 위치'" << endl;
#endif
		if (TargetLocation != LastKnownTargetLocation && LastKnownTargetLocation.size() != 0) {	// 타겟위치가 재설정되지 않았다면, -> 반복 UpdatePath 하지 않도록
			TargetLocation = LastKnownTargetLocation;
			UpdatePath();
		}
		break;

	case TARGET::PATROL:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '랜덤 패트롤'" << endl;
#endif
		
		if (RandPatrolSet == false) {
			
			if (IsStandingStill == true) {	// 숨고르기 상태라면 새로운 패트롤 지점 찾기 수행 X ====> 그냥 그자리에서 가만히 서있게함 (목적지가 같아서)
			
				auto waitAfterTime = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> deltaTime = waitAfterTime - waitBrainlessStartTime;
				
#ifdef ENABLE_BT_LOG
				cout << "좀비 #" << ZombieData.zombieID << " 가 숨고르기 중입니다!" << endl;
#endif

				if (deltaTime.count() >= ZombieStandingStillDuration) {
#ifdef ENABLE_BT_LOG
					cout << "좀비 #" << ZombieData.zombieID << " ";
					cout << "숨고르기 상태 끝 " << endl;
#endif

					IsStandingStill = false;
				}
				else {
#ifdef ENABLE_BT_LOG
					cout << "숨고르기 남은 시간: " << ZombieStandingStillDuration - deltaTime.count() << "s" << endl;
#endif
				}

				return;	// 아래, 새로운 랜덤 패트롤 지점 찾기 안하고 넘어감
			}


			bool result = false;
			int try_cnt = 0;
			while (result == false) {
				try_cnt++;
#ifdef	ENABLE_BT_LOG
				cout << "좀비 #" << ZombieData.zombieID << " 랜덤 패트롤 찾기 시도 - #" << try_cnt << endl;
#endif

				result = RandomPatrol();

				if (try_cnt >= 5 && result == false) {					// 랜덤 패트롤 목표점 찾기 5번까지만 시도
#ifdef	ENABLE_BT_LOG
					cout << "좀비 #" << ZombieData.zombieID << " 랜덤 패트롤 찾기 결국 실패!!! (연속 5번 실패...) => 좀비 해당 한 틱동안 멍때림!!" << endl;
#endif
					break;
				}
			}

		}
		break;

	case TARGET::RUNAWAY:
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 목표 타겟: '도망가기'" << endl;
#endif

		break;

	}

#ifdef	ENABLE_BT_LOG
	cout << "TargetLocation: ( " << TargetLocation[0][0][0] << " , " << TargetLocation[0][0][1] << " , " << TargetLocation[0][0][2] << " ) -> \'SetTargetLocation에서 호출\'" << endl;
	cout << endl;
#endif
}

// distanceType = 1: Detect(CanSeePlayer) / 2: FootSound
float Zombie::SearchClosestPlayer(vector<vector<vector<float>>>& closest_player_pos, int distanceType)
{
	float min_dist = FLT_MAX;	//float 최대값

	map<int, float> searchMap = {};
	float detect_dist = 0;
	if (distanceType == 1) {
		searchMap = DistanceTo_PlayerInsight;
		detect_dist = CanSeePlayerDistance;
	}
	else if (distanceType == 2) {
		searchMap = DistanceTo_FootSound;
		detect_dist = CanHearFootSoundDistance;
	}

	vector<int> closest_players = {};


	if (searchMap.size() != 0) {

		for (auto player : playerDB_BT[roomid]) {
			// 죽은 플레이어 무시
			if (player.second.health <= 0) {
				continue;
			}
			// 연결 끊긴 플레이어 무시
			if (g_players.find(player.first) == g_players.end()) {
				continue;
			}

			if (searchMap.find(player.first) != searchMap.end()) {
				if (min_dist > searchMap.at(player.first) && searchMap.at(player.first) > 0 && searchMap.at(player.first) <= detect_dist) {
					min_dist = searchMap.at(player.first);
#ifdef	ENABLE_BT_LOG
					if(distanceType == 1)
						cout << "플레이어 #" << player.first << " 가 좀비 #" << ZombieData.zombieID << " 의 시야에 존재함! --- 거리: " << searchMap.at(player.first) << endl;
					else if (distanceType == 2)
						cout << "플레이어 #" << player.first << " 가 좀비 #" << ZombieData.zombieID << " 에게 발소리 들킴! --- 거리: " << searchMap.at(player.first) << endl;
#endif
				}
				else if (searchMap.at(player.first) > detect_dist) {
#ifdef	ENABLE_BT_LOG
					if (distanceType == 1)
						cout << "플레이어 #" << player.first << " 가 좀비 #" << ZombieData.zombieID << " 의 시야에서 사라짐! --- 거리: " << searchMap.at(player.first) << endl;
					else if (distanceType == 2)
						cout << "플레이어 #" << player.first << "의 발소리가 좀비 #" << ZombieData.zombieID << " 에게서 사라짐! --- 거리: " << searchMap.at(player.first) << endl;
#endif
				}
			}
		}

		if (distanceType == 1 && PlayerInSight == false && min_dist == FLT_MAX) {
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "[ERROR] Data Race!!! -> PlayerInSight is false after CanSeePlayer Sequence check!" << endl;
			return min_dist;
		}

		// 혹시 같은 거리에 포착된 플레이어가 두명 이상일때, 그들중 랜덤한 플레이어 따라가게
		for (auto player : playerDB_BT[roomid]) {
			// 죽은 플레이어 무시
			if (player.second.health <= 0) {
				continue;
			}
			// 연결 끊긴 플레이어 무시
			if (g_players.find(player.first) == g_players.end()) {
				continue;
			}

			if (searchMap.find(player.first) != searchMap.end()) {
				if (min_dist == searchMap.at(player.first)) {
					closest_players.emplace_back(player.first);
#ifdef	ENABLE_BT_LOG
					cout << "플레이어 #" << player.first << " 가 좀비 #" << ZombieData.zombieID << " 와 가장 가까움! --- 거리: " << searchMap.at(player.first) << endl;
					cout << "플레이어 #" << player.first << " 의 위치: ( " << player.second.x << " , " << player.second.y << " , " << player.second.z << " )" << endl;
#endif
				}
			}
		}

		if (closest_players.size() == 0) {
			if (distanceType == 1) {
				cout << "좀비 #" << ZombieData.zombieID << " - ";
				cout << "[ERROR] closest_players.size() == 0 -> 가장 가까운 플레이어(좀비시야) 찾을 수 없음" << endl;
			}
			else if (distanceType == 2) {
				cout << "좀비 #" << ZombieData.zombieID << " - ";
				cout << "[ERROR] closest_players.size() == 0 -> 가장 가까운 플레이어(발소리) 찾을 수 없음" << endl;

				// [+] 발소리 같은 경우에는 이전에 발소리를 들은 기억이 있지만 지금은 모두 발소리를 내지 않거나 탐지 거리를 나갔을때에도, 이미 HeardFootSound가 true여서 SetTargetLocation이 무조건 실행되어 
				// 해당 함수인 SearchClosestPlayer가 불가피하게 실행되는 일이 발생함 
				// (즉, 가장 가까운 플레이어를 도출하지 못 할 상황이라도 해당 함수가 불려짐, 그래도 문제는 없는게 이전에 TargetLocation을 지정해 놔서 그거 따라 갈꺼임)
				// [X] => HasFootSound에서 불필요하게 SetTargetLocation을 부르는 코드였음 -> 주석처리함 이제
			}

			return min_dist;
		}

		std::random_device rd;
		std::mt19937 mt(rd());

		std::uniform_int_distribution<int> distribution(0, closest_players.size() - 1);

		int randomIndx = distribution(mt);

		ClosestPlayerID = closest_players[randomIndx];		// 가장 가까운 플레이어 인덱스 저장
		
		if (distanceType == 1 && IsRunaway == false) {
#ifdef	ENABLE_BT_LOG
			cout << "좀비 #" << ZombieData.zombieID << " 가 플레이어 #" << ClosestPlayerID << " 을 따라감!!!" << endl;
#endif
		}
		else if (distanceType == 2 && IsRunaway == false) {
#ifdef	ENABLE_BT_LOG
			cout << "좀비 #" << ZombieData.zombieID << " 가 플레이어 #" << ClosestPlayerID << " 의 발소리 따라감!!!" << endl;
#endif
		}

		// {주의} map 사용 할 때 주의할 점 (playerDB_BT[ClosestPlayerID].x) => 이런식으로 사용하면 키값이 없을 경우 "새로 해당 키에 실제 데이터는 없이" 더미 데이터가 새로 추가가 됨!
		closest_player_pos = vector<vector<vector<float>>>{ {{playerDB_BT[roomid][ClosestPlayerID].x, playerDB_BT[roomid][ClosestPlayerID].y, playerDB_BT[roomid][ClosestPlayerID].z}} };
	}
	else {	// (searchMap.size() == 0)
		if (distanceType == 1) {
			// BT에서 타겟을 플레이어로 했는데, DistanceTo_PlayerInsight 맵이 비어 있다면 절대 안됨
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "DistanceTo_PlayerInsight Map \"ERROR\"!!! -> Target is set to Player but DistanceTo_PlayerInsight Map is empty" << endl;
		}
		else if (distanceType == 2) {
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "DistanceTo_FootSound Map \"ERROR\"!!! -> Target is set to FootSound but DistanceTo_FootSound Map is empty" << endl;
		}
	}

	return min_dist;
}

void Zombie::Attack(int roomid)
{
#ifdef	ENABLE_BT_LOG
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 가 플레이어 \'#" << ClosestPlayerID << "\' 을 공격하였습니다!" << endl;
	cout << endl;
#endif

	// 어택 통신 패킷 보내기

	Protocol::Zombie_attack attackpacket;

	attackpacket.set_zombieid(ZombieData.zombieID);
	attackpacket.set_playerid(ClosestPlayerID);
	attackpacket.set_packet_type(13);

	string serializedData;
	attackpacket.SerializeToString(&serializedData);

	for (const auto& player : playerDB_BT[roomid]) {
		iocpServer->IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());	
	}

	IsAttacking = true;	// 좀비 공격중으로 변경

	HaveToWait = true;	// 좀비 BT 대기상태로 변경

	waitAnimStartTime = std::chrono::high_resolution_clock::now();		// 좀비 공격 시작 시간
}

void Zombie::MoveTo(float deltasecond)
{
	if (IsStandingStill == true // 숨고르기 상태라면 MoveTo 바로 종료 => ReachFinalDestionation 들어가서 ClearBlackBoard 하는거 방지해야함 (숨고르기 할때 계속 초기화해주니까 플레이어를 못 볼 수 있음)
		&& IsRunaway == false) {	// 다만 도망치기 중에 하는 숨고르기는 계속 작동해야하므로
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		cout << "좀비 #" << ZombieData.zombieID << " 숨고르기 상태. (MoveTo 바로 종료)" << endl;
		cout << endl;
#endif
		return;
	}

	bool spacing = false;

	//cout << "ZombiePathIndex: " << ZombiePathIndex << " , path.size(): " << path.size() << endl;
	//cout << endl;

	if (IsPathUpdated()) {
		ZombiePathIndex = 1;
	}

	if (ZombiePathIndex >= path.size() || ZombieData.x == TargetLocation[0][0][0] && ZombieData.y == TargetLocation[0][0][1] /*&& ZombieData.z == TargetLocation[0][0][2]*/) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		cout << "좀비 #" << ZombieData.zombieID << " 경로의 끝 도착. (before move)" << endl;
#endif
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		cout << endl;
#endif
		ReachFinalDestination();
		ZombiePathIndex = 1;

		return;
	}

	// 현재 목표 노드
	tuple<float, float, float> TargetNode = path[ZombiePathIndex];
	float PathX = get<0>(TargetNode);
	float PathY = get<1>(TargetNode);

#ifdef	ENABLE_BT_LOG
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 가 이동 해야할 현재 경로의 다음 좌표: ( " << get<0>(path[ZombiePathIndex]) << ", " << get<1>(path[ZombiePathIndex]) << ", " << get<2>(path[ZombiePathIndex]) << " )" << endl;
#endif
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
	spacing = true;
#endif

	if (PathX >= 2366.f) {	// 계단쪽 너머로 이동하려하면
#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 계단 너머로 이동 방지로 인해 정지" << endl;
#endif
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		cout << endl;
#endif
		ReachFinalDestination();
		ZombiePathIndex = 1;

		return;
	}

	// 타겟 방향 계산
	float dx = PathX - ZombieData.x;
	float dy = PathY - ZombieData.y;

	// 거리를 계산
	float distance = sqrt(dx * dx + dy * dy);

	// 이동 방향 벡터를 정규화
	float directionX = dx / distance;
	float directionY = dy / distance;

	// 좀비 속도 지정 (걷기/뛰기)
	if (targetType == TARGET::PATROL) {	// 걷기

		switch (GetZombieType()) {
		case ZOMBIE_TYPE::NULL_TYPE:
			cout << "[ERROR] 좀비 #" << ZombieData.zombieID << " ZombieType 변수값 미지정(NULL_TYPE)!!! ---> Speed = 0" << endl;
			break;
		case ZOMBIE_TYPE::NORMAL_ZOMBIE:
			ZombieSpeed = NormalZombieWalkSpeed;
			break;
		case ZOMBIE_TYPE::RUNNING_ZOMBIE:
			ZombieSpeed = RunningZombieWalkSpeed;
			break;
		case ZOMBIE_TYPE::SHOUTING_ZOMBIE:
			ZombieSpeed = ShoutingZombieWalkSpeed;
			break;
		}
	}
	// 뛰기
	else if (targetType == TARGET::PLAYER || targetType == TARGET::SHOUTING || targetType == TARGET::FOOTSOUND || targetType == TARGET::HORDESOUND || targetType == TARGET::INVESTIGATED 
		|| targetType == RUNAWAY) {	

		switch (GetZombieType()) {
		case ZOMBIE_TYPE::NULL_TYPE:
			cout << "[ERROR] 좀비 #" << ZombieData.zombieID << " ZombieType 변수값 미지정(NULL_TYPE)!!! ---> Speed = 0" << endl;
			break;
		case ZOMBIE_TYPE::NORMAL_ZOMBIE:
			ZombieSpeed = NormalZombieSpeed;
			break;
		case ZOMBIE_TYPE::RUNNING_ZOMBIE:
			ZombieSpeed = RunningZombieSpeed;
			break;
		case ZOMBIE_TYPE::SHOUTING_ZOMBIE:
			ZombieSpeed = ShoutingZombieSpeed;
			break;
		}
	}
	else {
		cout << "[ERROR] 좀비 #" << ZombieData.zombieID << " targetType 변수값(" << targetType << ") 알 수 없는 값!!! ---> Speed = 0" << endl;
	}

	// 이동 거리 계산
	float moveDistance = ZombieSpeed * deltasecond;

	// 이동 벡터 계산
	float moveX = directionX * moveDistance;
	float moveY = directionY * moveDistance;


	// 타겟 위치에 도달했는지 확인
	float newDistance = sqrt((PathX - ZombieData.x) * (PathX - ZombieData.x) + (PathY - ZombieData.y) * (PathY - ZombieData.y));

	if (newDistance < moveDistance) {
		ZombieData.x = PathX;
		ZombieData.y = PathY;

		// 다음 목표 노드로 이동
		ZombiePathIndex++;

		// 경로의 끝에 도착 = 최종 목표지점에 도착
		if (ZombiePathIndex >= path.size() || ZombieData.x == TargetLocation[0][0][0] && ZombieData.y == TargetLocation[0][0][1]) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
			cout << "좀비 #" << ZombieData.zombieID << " 경로의 끝 도착." << endl;
#endif
			ReachFinalDestination();
			ZombiePathIndex = 1;

			spacing = false;
		}
		else {	// 꼭지점을 넘어 갈 때
			float mid_deltasecond = newDistance / ZombieSpeed;
			float after_deltasecond = deltasecond - mid_deltasecond;

			spacing = false;

			MoveTo(after_deltasecond);
		}
	}
	else {
		// 타겟 방향으로 이동
		ZombieData.x += moveX;
		ZombieData.y += moveY;


		// 주위 둘러보기 초기화 => 주위 둘러보기 도중에 움직였다? => 상태가 바뀜 => 초기화 필요!
		if (IsLookingAround == true) {
			IsLookingAround = false;
			lookAroundCount = 0;

#ifdef ENABLE_BT_NODE_LOG
			cout << "[MoveTo] LookingAround 상태 초기화! => 움직임!" << endl;
#endif
		}
	}

#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
	if (spacing)
		cout << endl;
#endif
}

bool Zombie::IsPathUpdated()
{
	if (!path.empty() && !beforepath.empty()) {

		if (beforepath != path) {
#ifdef	ENABLE_BT_LOG
			//cout << "IsPathUpdated(): (!path.empty() && !beforepath.empty()) && (beforepath != path) => return true" << endl;
			//cout << endl;
#endif
			beforepath = path;

			return true;
		}

	}

	if (!path.empty() && beforepath.empty()) {
#ifdef	ENABLE_BT_LOG
		//cout << "IsPathUpdated(): (!path.empty() && beforepath.empty()) => return true" << endl;
		//cout << endl;
#endif
		beforepath = path;

		return true;
	}

#ifdef	ENABLE_BT_LOG
	//cout << "IsPathUpdated() => return false" << endl;
	//cout << endl;
#endif

	return false;
}

void Zombie::UpdatePath()
{
	pathfinder.UpdateStartGoal(ZombieData.x, ZombieData.y, ZombieData.z, TargetLocation[0][0][0], TargetLocation[0][0][1], TargetLocation[0][0][2]);
	pathfinder.Run(path, 0);		// [+] 목표지점의 위치 변화가 없으면 A* 를 돌리지 않는 코드 작성되어 있음
	//cout << endl;
}

bool Zombie::CheckPath(vector<tuple<float, float, float>>& goalTest_path, float goalTestX, float goalTestY, float goalTestZ)
{
	pathfinder.UpdateStartGoal(ZombieData.x, ZombieData.y, ZombieData.z, goalTestX, goalTestY, goalTestZ);
	pathfinder.Run(goalTest_path, 1);
	//cout << endl;

	if (goalTest_path.empty() == true) {
		return false;
	}

	goalTest_path.pop_back();	// 맨 마지막은 랜덤한 위치라서... 좀비가 걸을 수 있는 지형이 아닐 수 있음 -> 빼서 무조건 걸을 수 있는 위치가 최종 목적지!

	return true;
}

// 얜 랜덤 패트롤, 랜덤 발소리 위치용 - 함수 오버로딩 & TargetLocation 설정
void Zombie::UpdatePath(vector<tuple<float, float, float>> newPatrol_path)
{
	tuple<float, float, float> dest;
	if (!newPatrol_path.empty()) {
		dest = newPatrol_path.back();
	}

	TargetLocation[0][0][0] = get<0>(dest);
	TargetLocation[0][0][1] = get<1>(dest);
	TargetLocation[0][0][2] = ZombieData.z;

	path = newPatrol_path;
	// 참고로 pathfinder 변수의 start,goal 수정(UpdateStartGoal 함수 호출)은 CheckPath에서 미리 해줌
}

void Zombie::ReachFinalDestination()
{
#ifdef	ENABLE_BT_LOG
	cout << "좀비 '#" << ZombieData.zombieID << "' 타겟 좌표[최종 목표 지점] ( " << TargetLocation[0][0][0] << ", " << TargetLocation[0][0][1] << ", " << TargetLocation[0][0][2] << " ) 에 도착!!!" << endl;
#endif


	// BT 플래그 값 전체 초기화 -> 만약! 병렬 구조로 만들 생각이라면 이렇게 전체 초기화 하지 말고 이미 순회한 플래그 값만 초기화 해야함 ===> X
	//							=> 그냥 좀비는 멍청하게 만들어야 하니까 도착지점에 어디라도 도착하면 전부 잊는 방향으로
	//bool clear_flag[6] = { true, true, true, true, true, true };
	//ClearBlackBoard(clear_flag);
	// ===> 이렇게 하면 좀비가 플레이어를 따라 갈때 계속 쫒아가기만 하게됨 (공격을 못함 - 또는 공격에 딜레이(플레이어를 잠시 밀치다가 공격함;;))
	// [괜춘] -> 딜레이가 0.1초일땐 이런데 0.05초로 줄이면 발생 X
	// [X] ==> 패트롤에서 연속 숨고르기를 하게 될경우 블랙보드를 모두 초기화 하면 X (연속으로 숨고르는 상황이 나오면 다시 숨고르기 하는 순간에 초기화를 하면 플레이어가 코앞에 있어도 계속 무시하게 되서;; (다시 시야에 나갔다가 들어 오지 않는 이상))

	//PlayerInSight = false;	// 굳이 할필요는 없음
	//HeardShouting = false;
	//HeardFootSound = false;
	//HeardHordeSound = false;
	//KnewPlayerLocation = false;
	//RandPatrolSet = false;


	bool clear_flag[6] = {};

	//<Selector Detect>의 Task들의 실행 조건이 되는 bool값들 초기화
	switch (targetType) {
	case TARGET::PLAYER:
		// 작동할 일이 없을 것 같긴하지만;;
		if (IsLookingAround == false && lookAroundCount == 0) {
			IsLookingAround = true;
			lookAroundStartTime = std::chrono::high_resolution_clock::now();	// 주위 둘러보기 시작시간 기억
		}
		
		if (IsLookingAround == true) {
			LookAround();
		}
		
		if (IsLookingAround == false && lookAroundCount >= 3) {	// 주위 둘러보기 끝남
			lookAroundCount = 0;	// 주위 둘러보기 카운트 다시 초기화
			
			// 주위 둘러보기 끝나고 이제 블랙보드 초기화
			clear_flag[0] = true; clear_flag[1] = true; clear_flag[2] = true; clear_flag[3] = true; clear_flag[4] = true; clear_flag[5] = true;
			ClearBlackBoard(clear_flag);
			// ClearBlackBoard에서 PlayerInSight를 초기화 할 경우 반드시 이걸 클라에게도 알려줘야함 (+ 반대로, Patrol의 경우에는 PlayerInSight는 초기화 안하니까 바꿀 필요X)
			targetType = BLACKBOARDCLEARED;	// 블랙보드가 클리어 됨을 클라에게 targetType으로 전달 (클라도 detect 패킷 리셋 하도록[m_bPlayerInSight = false;[)])							
		}

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '플레이어'" << endl;
#endif
		break;

	case TARGET::SHOUTING:
		//HeardShouting = false;

		if (IsLookingAround == false && lookAroundCount == 0) {
			IsLookingAround = true;
			lookAroundStartTime = std::chrono::high_resolution_clock::now();	// 주위 둘러보기 시작시간 기억
		}

		if (IsLookingAround == true) {
			LookAround();
		}

		if (IsLookingAround == false && lookAroundCount >= 3) {	// 주위 둘러보기 끝남
			lookAroundCount = 0;	// 주위 둘러보기 카운트 다시 초기화

			// 주위 둘러보기 끝나고 이제 블랙보드 초기화
			clear_flag[0] = true; clear_flag[1] = true; clear_flag[2] = true; clear_flag[3] = true; clear_flag[4] = true; clear_flag[5] = true;
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;
		}

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '샤우팅'" << endl;
#endif
		break;

	case TARGET::FOOTSOUND:
		//HeardFootSound = false;

		if (IsLookingAround == false && lookAroundCount == 0) {
			IsLookingAround = true;
			lookAroundStartTime = std::chrono::high_resolution_clock::now();	// 주위 둘러보기 시작시간 기억
		}

		if (IsLookingAround == true) {
			LookAround();
		}

		if (IsLookingAround == false && lookAroundCount >= 3) {	// 주위 둘러보기 끝남
			lookAroundCount = 0;	// 주위 둘러보기 카운트 다시 초기화

			// 주위 둘러보기 끝나고 이제 블랙보드 초기화
			clear_flag[0] = true; clear_flag[1] = true; clear_flag[2] = true; clear_flag[3] = true; clear_flag[4] = true; clear_flag[5] = true;
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;
		}

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '발소리'" << endl;
#endif
		break;

	case TARGET::HORDESOUND:
		//HeardHordeSound = false;

		if (IsLookingAround == false && lookAroundCount == 0) {
			IsLookingAround = true;
			lookAroundStartTime = std::chrono::high_resolution_clock::now();	// 주위 둘러보기 시작시간 기억
		}

		if (IsLookingAround == true) {
			LookAround();
		}

		if (IsLookingAround == false && lookAroundCount >= 3) {	// 주위 둘러보기 끝남
			lookAroundCount = 0;	// 주위 둘러보기 카운트 다시 초기화

			// 주위 둘러보기 끝나고 이제 블랙보드 초기화
			clear_flag[0] = true; clear_flag[1] = true; clear_flag[2] = true; clear_flag[3] = true; clear_flag[4] = true; clear_flag[5] = true;
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;
		}

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '호드 사운드'" << endl;
#endif
		break;

	case TARGET::INVESTIGATED:
		//KnewPlayerLocation = false;

		if (IsLookingAround == false && lookAroundCount == 0) {
			IsLookingAround = true;
			lookAroundStartTime = std::chrono::high_resolution_clock::now();	// 주위 둘러보기 시작시간 기억
		}
		
		if (IsLookingAround == true) {
			LookAround();
		}
		
		if (IsLookingAround == false && lookAroundCount >= 3) {	// 주위 둘러보기 끝남
			lookAroundCount = 0;	// 주위 둘러보기 카운트 다시 초기화

			// 주위 둘러보기 끝나고 이제 블랙보드 초기화
			clear_flag[0] = true; clear_flag[1] = true; clear_flag[2] = true; clear_flag[3] = true; clear_flag[4] = true; clear_flag[5] = true;
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;
		}

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '이전 발견 위치'" << endl;
#endif
		break;

	case TARGET::PATROL:
		//RandPatrolSet = false;

		clear_flag[0] = false; clear_flag[1] = false; clear_flag[2] = false; clear_flag[3] = false; clear_flag[4] = false; clear_flag[5] = true;
		ClearBlackBoard(clear_flag);
		//==> 패트롤에서 연속 숨고르기를 하게 될경우 블랙보드를 모두 초기화 하면 X (연속으로 숨고르는 상황이 나오면 다시 숨고르기 하는 순간에 초기화를 하면 플레이어가 코앞에 있어도 계속 무시하게 되서;; (다시 시야에 나갔다가 들어 오지 않는 이상))

		// 일정 확률로 좀비 잠시 멍때리기/휴식 (숨고르기) 상태 만들기
		TakeABreak();

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '랜덤 패트롤'" << endl;
#endif
		break;

	case TARGET::RUNAWAY:

		IsStandingStill = true;	// 도망치기 상태에서 도망치기 포인트에 도달하면 그자리에서 가만히 숨고르기하며 체력회복하기

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 의 도달 타겟: '도망치기' => '숨어서 숨고르기 발동!'" << endl;
#endif
		break;
	}

#ifdef	ENABLE_BT_LOG
	cout << endl;
#endif
}

// 블랙보드 클리어 용 (distanceType = 1: Detect / 2: FootSound)
void Zombie::ClearDistance(int playerid, int distanceType)
{
	map<int, float>* setMap = {};

	bool spacing = false;

	if (distanceType == 1)
		setMap = &DistanceTo_PlayerInsight;
	else if (distanceType == 2)
		setMap = &DistanceTo_FootSound;

	if (setMap->find(playerid) == setMap->end()) {		// map에 playerid가 없으면 -> 스킵

	}
	else if (setMap->find(playerid) != setMap->end()) {		// map에 이미 playerid가 이미 있으면 -> 리셋
		if(distanceType == 1)
			setMap->at(playerid) = CanSeePlayerDistance + 6969.f;		// 탐지 거리 밖 표시 (초기화)
		if(distanceType == 2)
			setMap->at(playerid) = CanHearFootSoundDistance + 6969.f;		// 탐지 거리 밖 표시 (초기화)
	}

	return;
}

// 0-PlayerInsight, 1-HeardShouting, 2-HeardFootSound, 3-HeardHordeSound, 4-KnewPlayerLocation, 5-RandPatrolSet
void Zombie::ClearBlackBoard(bool clear_flag[6])
{
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG)
	printf("[ClearBlackBoard] Cleared flags\n");
#endif

	if (clear_flag[0]) {
		PlayerInSight = false;
		// 맵도 초기화
		for (auto player : playerDB_BT[roomid]) {
			// 죽은 플레이어 무시
			if (player.second.health <= 0) {
				continue;
			}
			// 연결 끊긴 플레이어 무시
			if (g_players.find(player.first) == g_players.end()) {
				continue;
			}

			ClearDistance(player.first, 1);
		}
		detectCanSeePlayer_randomChance = false;
	}

	if (clear_flag[1])
		HeardShouting = false;

	if (clear_flag[2]) {
		HeardFootSound = false;
		// 맵도 초기화
		for (auto player : playerDB_BT[roomid]) {
			// 죽은 플레이어 무시
			if (player.second.health <= 0) {
				continue;
			}
			// 연결 끊긴 플레이어 무시
			if (g_players.find(player.first) == g_players.end()) {
				continue;
			}

			ClearDistance(player.first, 2);
		}
		detectHasFootSound_randomChance = false;
	}

	if (clear_flag[3])
		HeardHordeSound = false;

	if (clear_flag[4])
		KnewPlayerLocation = false;

	if (clear_flag[5]) {
		RandPatrolSet = false;
		IsStandingStill = false;
	}

#ifdef ENABLE_BT_LOG
	printf(" PlayerInSight: %s, HeardShouting : %s, HeardFootSound : %s, HeardHordeSound : %s, KnewPlayerLocation : %s, RandPatrolSet : %s\n"
		, clear_flag[0] ? "cleared" : "stay", clear_flag[1] ? "cleared" : "stay", clear_flag[2] ? "cleared" : "stay", clear_flag[3] ? "cleared" : "stay", clear_flag[4] ? "cleared" : "stay", clear_flag[5] ? "cleared" : "stay");
	printf(" PlayerInSight: %s, HeardShouting: %s, HeardFootSound: %s, HeardHordeSound: %s, KnewPlayerLocation: %s, RandPatrolSet: %s\n"
		, PlayerInSight ? "true" : "false", HeardShouting ? "true" : "false", HeardFootSound ? "true" : "false", HeardHordeSound ? "true" : "false", KnewPlayerLocation ? "true" : "false", RandPatrolSet ? "true" : "false");
#endif
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
	cout << endl;
#endif
}

void Zombie::TakeABreak()
{
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> stand_still_chance(0, 100);		// 0~100
	std::uniform_int_distribution<int> stand_still_duration(5, 10);		// 5~10

	float the_chance = (float)stand_still_chance(mt);
	float the_duration = (float)stand_still_duration(mt);

	if (the_chance >= (100 - takeABreak_chance) && IsStandingStill == false) {	// 60퍼센트의 확률
		//HaveToWait = true;
		IsStandingStill = true;
		waitBrainlessStartTime = std::chrono::high_resolution_clock::now();	// 가만히 서있기 시작 시간
		ZombieStandingStillDuration = the_duration;

#ifdef	ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 잠시 휴식(숨고르기) 상태 발동 (" << ZombieStandingStillDuration << "초 동안)" << endl;
#endif
	}
}

bool Zombie::PlayerInSight_Update_Check()
{
	bool result = false;

	bool spacing = false;


	// 시야에 있는 플레이어들 DistanceTo_PlayerInsight 맵 갱신
	for (auto player : playerDB_BT[roomid]) {
		// 죽은 플레이어 무시
		if (player.second.health <= 0) {
			if (DistanceTo_PlayerInsight.find(player.first) != DistanceTo_PlayerInsight.end()) {	// 이미 DistanceTo_PlayerInsight에 저장된 값이 있다면
			}
			continue;
		}
		// 연결 끊긴 플레이어 무시
		if (g_players.find(player.first) == g_players.end()) {
			continue;
		}
	
		if (DistanceTo_PlayerInsight.find(player.first) != DistanceTo_PlayerInsight.end()) {	// 이미 DistanceTo_PlayerInsight에 저장된 값이 있다면
			if (DistanceTo_PlayerInsight.at(player.first) >= 0 &&	// 죽거나 연결 끊긴 플레이어 아님
				DistanceTo_PlayerInsight.at(player.first) <= CanSeePlayerDistance) {	// 그리고 포착 가능한 거리일 때 (원래 2500이지만 실제 검사에서는 2500보다 약간 크게 잡혀도 통신보내서 +50까지 봐줌;;)
				SetDistance(player.first, 1);	//-> 거리 갱신하기
			}
		}
		// 새로 생성은 ProcessPacket에서 처음으로 Detect 패킷이 왔을때 생성함
	}
	

	// 시야에 최소 한명이라도 플레이어가 있나 체크
	for (auto& distTo_playerinsight : DistanceTo_PlayerInsight) {
		if (distTo_playerinsight.second >= 0 &&
			distTo_playerinsight.second <= CanSeePlayerDistance) {
			bool really_detected = true;

			if (playerDB_BT[roomid].find(distTo_playerinsight.first) != playerDB_BT[roomid].end()) {	// 아래 at 사용시에 혹시 모를 abort에러 방지용
				if (playerDB_BT[roomid].at(distTo_playerinsight.first).health <= 0) { // 플레이어가 이제 죽었다면
					distTo_playerinsight.second = -1.0f;	// 더이상 탐지 불가로 바꾸기
					really_detected = false;
				}
			}

			if (g_players.find(distTo_playerinsight.first) == g_players.end()) {	// 플레이어 연결이 이제 끊겼다면
				distTo_playerinsight.second = -1.0f;	// 더이상 탐지 불가로 바꾸기
				really_detected = false;
			}


			if (really_detected) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG) /*|| defined(ENABLE_BT_FLEE_LOG)*/
				cout << "좀비 #" << ZombieData.zombieID << " 시야에 플레이어 #" << distTo_playerinsight.first << " 를 확인! --- 거리: " << distTo_playerinsight.second << endl;
				spacing = true;
#endif

				result = true;
			}
		}
	}


#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG) /*|| defined(ENABLE_BT_FLEE_LOG)*/
	if (result == false) {
		cout << "좀비 #" << ZombieData.zombieID << " 가 그 어떤 새로운 플레이어도 포착 못함! (PlayerInSight_Update_Check() == false)" << endl;
		spacing = true;
	}
#endif


#if defined(ENABLE_BT_LOG) /*|| defined(ENABLE_BT_FLEE_LOG)*/
	if (spacing)
		cout << endl;
#endif


	PlayerInSight = result;		// PlayerInSight 검사는 FootSound와는 다르게 실시간으로 포착 가능상태일 때만 true여야함 (FootSound는 이전에 발소리를 들은 적이 있다면 해당 값 놔둬야하지만 -> false로 바꾸는 건 도착지에 도착하고 나서)

	return result;
}

bool Zombie::FootSound_Update_Check()
{
	bool result = false;

	bool spacing = false;


	// 뛰고 있는 플레이어들 DistanceTo_FootSound 맵에 갱신
	for (auto player : playerDB_BT[roomid]) {
		// 죽은 플레이어 무시
		if (player.second.health <= 0) {
			continue;
		}
		// 연결 끊긴 플레이어 무시
		if (g_players.find(player.first) == g_players.end()) {
			continue;
		}

		if (player.second.IsRunning) {	// 일단 플레이어가 (거리 상관없이) 뛰었다면 
			if (player.second.x < 2366.f) {	// 그리고, 계단 너머가 아니라면 

				// 거리 계산
				float dx = player.second.x - ZombieData.x;
				float dy = player.second.y - ZombieData.y;

				float distance = sqrt(dx * dx + dy * dy);

				if (distance > CanHearFootSoundDistance) {	// 포착 거리 밖이라면
					continue;	//-> 무시
				}

				if (DistanceTo_FootSound.find(player.first) != DistanceTo_FootSound.end()) {	// 기존에 존재 
					if (DistanceTo_FootSound.at(player.first) >= 0) { // 죽거나 연결 끊긴 플레이어 아님
						// FootSound map 접근
						SetDistance(player.first, 2);	//-> 갱신
					}
				}
				else {	// 없었음 
					// FootSound map 접근
					SetDistance(player.first, 2);	//-> 새로 생성
				}
			}
		}
		else {	// 현재는 뛰고 있지 않은 상태
			// 근데 이미 맵에 저장되어 있다면 초기화 해주기 (더이상 탐지 불가)
			if (DistanceTo_FootSound.find(player.first) != DistanceTo_FootSound.end()) {
				DistanceTo_FootSound[player.first] = CanHearFootSoundDistance + 6969.f;		// 탐지 거리 밖 표시
			}
		}
	}


	// 좀비가 근처에서 발소리가 하나라도 났었는지 체크
	for (auto& distTo_footSound : DistanceTo_FootSound) {

		if (playerDB_BT[roomid].find(distTo_footSound.first) != playerDB_BT[roomid].end()) {	// 아래 at 사용시에 혹시 모를 abort에러 방지용
			if (playerDB_BT[roomid].at(distTo_footSound.first).health <= 0) { // 플레이어가 이제 죽었다면
				distTo_footSound.second = -1.0f;	// 더이상 탐지 불가로 바꾸기
				continue;
			}
		}

		if (g_players.find(distTo_footSound.first) == g_players.end()) {	// 플레이어 연결이 이제 끊겼다면
			distTo_footSound.second = -1.0f;	// 더이상 탐지 불가로 바꾸기
			continue;
		}


		if (distTo_footSound.second <= CanHearFootSoundDistance && distTo_footSound.second > 0) {	// 발소리 포착 거리 내라면
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
			cout << "좀비 #" << ZombieData.zombieID << " 가 플레이어 #" << distTo_footSound.first << " 의 새로운 발소리 포착함! --- 거리: " << distTo_footSound.second << endl;
			spacing = true;
#endif

			result = true;
		}
		else {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
			cout << "좀비 #" << ZombieData.zombieID << " 가 플레이어 #" << distTo_footSound.first << " 의 새로운 발소리 포착 못함! --- 거리: " << distTo_footSound.second << endl;
			spacing = true;
#endif

			distTo_footSound.second = CanHearFootSoundDistance + 6969.f;		// 탐지 거리 밖 표시
		}
	}


	if (result == false) {
#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
		cout << "좀비 #" << ZombieData.zombieID << " 가 그 어떤 새로운 발소리도 포착 못함! (FootSound_Update_Check() == false)" << endl;
		spacing = true;
#endif
	}

#if defined(ENABLE_BT_LOG) || defined(ENABLE_BT_NODE_LOG)
	if (spacing)
		cout << endl;
#endif

	return result;
}

void Zombie::SetRandomTargetLocation(vector<vector<vector<float>>> target_original_pos)
{
	float dx = target_original_pos[0][0][0] - ZombieData.x;
	float dy = target_original_pos[0][0][1] - ZombieData.y;

	// 거리를 계산
	float distance = sqrt(dx * dx + dy * dy);

	// 랜덤 탐색 범위 설정 (좀비와 발소리간의 거리에 따라, 멀면 멀수록 랜덤 탐색 범위도 커짐)
	float radius = 0.f;

	if (distance > CanHearFootSoundDistance) {	// ~1000
		//cout << "[Error] 좀비 #" << ZombieData.zombieID << " 's distance to FootSound is out of range already. (SetRandomTargetLocation -> over CanHearFootSoundDistance)" << endl;
		//cout << "=> SetTargetLocation 설정 안됨 (발소리 안 따라감)" << endl;
		// 사실 뜨면 안되는 에러로그 ->[x] 1000 너머에 왔다 갔다 할 경우 많이 뜸(한 틱마다 다시 계산하니까)
		//=> 꽤 많이 떠서;; 주석처리;;;
		return;
	}
	else if (distance >= 800.f) { //1000 ~ 800
		radius = 600.f;
	}
	else if (distance >= 500.f) {	//800 ~ 500
		radius = 300.f;
	}
	else if (distance >= 0.f) {	//500 ~ 0
		radius = 0.f;	// 사실 여기 들어가면 SearchRandomWalkableLocation 안 하는 게 좋음... -> path 체크를 하기 떄문에 시간 걸림;; (최적화)
	}
	else {	//0~
		cout << "[Error] 좀비 #" << ZombieData.zombieID << " 's distance to FootSound is out of range already. (SetRandomTargetLocation -> under Zero)" << endl;
		cout << "=> SetTargetLocation 설정 안됨 (발소리 안 따라감)" << endl;
		return;
	}

#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
	cout << "distance: " << distance << ", radius: " << radius << "=@=@=@=@=@=@=" << endl;
#endif

	// 랜덤 탐색 지점(걸을 수 있는) 검색
	bool result = false;
	int try_cnt = 0;
	while (result == false) {
		try_cnt++;

		result = SearchRandomWalkableLocation(target_original_pos, radius);

		if (try_cnt >= 5 && result == false) {
#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
			cout << "좀비 #" << ZombieData.zombieID << " SetRandomTargetLocation 결국 실패!!! (연속 5번 실패...) => 그냥 해당 지점으로 바로 감!!" << endl;
#endif
			break;
		}
	}

}

bool Zombie::SearchRandomWalkableLocation(vector<vector<vector<float>>> target_original_pos, int search_radius)
{
	float rx, ry, rz;

	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> dist(-search_radius, search_radius);		//현 위치에서 반경 search_radius +-

	rx = target_original_pos[0][0][0] + dist(mt);
	ry = target_original_pos[0][0][1] + dist(mt);
	rz = target_original_pos[0][0][2];

	while (rx >= 2366.f) {		
		rx = target_original_pos[0][0][0] + dist(mt);

		if (target_original_pos[0][0][0] >= 2366.f) {
#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "가 포착한 플레이어 발소리 이미 걸을 수 있는 지형을 벗어남!!! (계단 쪽을 넘어감)" << endl;
#endif
			return false;
		}
	}

	while (ry <= -1000.f) {		
		ry = target_original_pos[0][0][1] + dist(mt);

		if (target_original_pos[0][0][1] <= -1000.f) {	// 미리 footsound decorator(FootSound_Update_Check)에서 예외처리하긴함
#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
			cout << "좀비 #" << ZombieData.zombieID << " - ";
			cout << "가 포착한 플레이어 발소리 이미 걸을 수 있는 지형을 벗어남!!! (화장실 뒤쪽을 넘어)" << endl;	
#endif
			return false;
		}
	}

	vector<tuple<float, float, float>> dest_test;

	// 랜덤한 지점이 갈 수 있는 곳 인지 검사
	if (CheckPath(dest_test, rx, ry, rz) == false) {
#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
		cout << "좀비 #" << ZombieData.zombieID << " SearchRandomWalkableLocation 찾기 실패! (CheckPath 실패) 다시 검색!!!" << endl;
#endif
		return false;
	}

	if (dest_test.size() < 3) {
#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
		cout << "좀비 #" << ZombieData.zombieID << " SearchRandomWalkableLocation 찾기 실패!!! => dest_test.size() < 3 -> 현재 지점과 너무 가까운 점" << endl;
#endif
		return false;
	}

	UpdatePath(dest_test);

#ifdef	ENABLE_BT_FOOTSOUND_SEARCHRANDOMLOCATION_LOG
	cout << "발소리 탐지 랜덤 위치 설정 완료!" << endl;
	cout << "TargetLocation[FootSound Random Location]: ( " << TargetLocation[0][0][0] << " , " << TargetLocation[0][0][1] << " , " << TargetLocation[0][0][2] << " )" << endl;
#endif
	
	return true;
}

bool Zombie::CanSeePlayerRandomChance()
{
	float dx = ZombieData.x - playerDB_BT[roomid][ClosestPlayerID].x;
	float dy = ZombieData.y - playerDB_BT[roomid][ClosestPlayerID].y;

	// 거리를 계산
	float dist = sqrt(dx * dx + dy * dy);

	auto waitAfterTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = waitAfterTime - RandomChanceBuff_CanSeePlayer_StartTime;

	if (deltaTime.count() >= RandomChanceBuff_CanSeePlayer_Duration) {
		RandomChanceBuff_CanSeePlayer = 0;	// 확률 버프 초기화
	}

	if (dist <= 800) {	// 바로 탐지 (100확률로)
		detectCanSeePlayer_randomChance = true;
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
		cout << "CanSeePlayerRandomChance: 100(dist <= 800), dist: " << dist << endl;
#endif
		return true;
	}

	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> random_chance(0, 100);		// 0~100
	//std::uniform_int_distribution<int> fail_duration(5, 10);		// 5~10

	float the_chance = (float)random_chance(mt) + RandomChanceBuff_CanSeePlayer;	// random_chance(mt)는 부르는 매 순간 바뀌어서 이렇게 변수에 미리 넣고 사용

#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
	cout << "CanSeePlayerRandomChance: " << the_chance << ", dist: " << dist << endl;
#endif

	if (dist <= 1000) {
		if (the_chance >= (100 - 80)) {	// 80퍼센트의 확률
			detectCanSeePlayer_randomChance = true;
			return true;
		}
		else {
			detectCanSeePlayer_randomChance = false;
			detectCanSeePlayerFail_delayTime = 1.0;
			return false;
		}
	}
	else if (dist <= 1500) {
		if (the_chance >= (100 - 60)) {	// 60퍼센트의 확률
			detectCanSeePlayer_randomChance = true;
			return true;
		}
		else {
			detectCanSeePlayer_randomChance = false;
			detectCanSeePlayerFail_delayTime = 2.0;
			return false;
		}
	}
	else if (dist <= 2000) {
		if (the_chance >= (100 - 30)) {	// 30퍼센트의 확률
			detectCanSeePlayer_randomChance = true;
			return true;
		}
		else {
			detectCanSeePlayer_randomChance = false;
			detectCanSeePlayerFail_delayTime = 3.0;
			return false;
		}
	}
	else if (dist <= CanSeePlayerDistance) {
		if (the_chance >= (100 - 10)) {	// 10퍼센트의 확률
			detectCanSeePlayer_randomChance = true;
			return true;
		}
		else {
			detectCanSeePlayer_randomChance = false;
			detectCanSeePlayerFail_delayTime = 4.0;
			return false;
		}
	}

	return false;
}

bool Zombie::HasFootSoundRandomChance()
{
	vector<vector<vector<float>>> closest_player_pos = {};
	float dist = SearchClosestPlayer(closest_player_pos, 2);

	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> random_chance(0, 100);		// 0~100
	//std::uniform_int_distribution<int> fail_duration(5, 10);		// 5~10

	float the_chance = (float)random_chance(mt);		// random_chance(mt)는 부르는 매 순간 바뀌어서 이렇게 변수에 미리 넣고 사용

#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
	cout << "FootSoundRandomChance: " << the_chance << ", dist: " << dist << endl;
#endif

	if (dist <= 500) {	
		if (the_chance >= (100 - 70)) {	// 70퍼센트의 확률
			detectHasFootSound_randomChance = true;
			return true;
		}
		else {
			detectHasFootSound_randomChance = false;
			detectHasFootSoundFail_delayTime = 0.5;
			return false;
		}
	}
	else if (dist <= 800) {
		if (the_chance >= (100 - 50)) {	// 50퍼센트의 확률
			detectHasFootSound_randomChance = true;
			return true;
		}
		else {
			detectHasFootSound_randomChance = false;
			detectHasFootSoundFail_delayTime = 1.0;
			return false;
		}
	}
	else if (dist <= CanHearFootSoundDistance) {
		if (the_chance >= (100 - 30)) {	// 30퍼센트의 확률
			detectHasFootSound_randomChance = true;
			return true;
		}
		else {
			detectHasFootSound_randomChance = false;
			detectHasFootSoundFail_delayTime = 2.0;
			return false;
		}
	}

	return false;
}

void Zombie::MakeNoise()
{
	auto waitAfterTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = waitAfterTime - makeHordeSound_StartTime;

	if (deltaTime.count() >= ZombieMakeHordeNoiseDelay) {
		makeHordeSound_StartTime = std::chrono::high_resolution_clock::now();
	}
	else {
#ifdef	ENABLE_BT_LOG
		cout << "좀비 '#" << ZombieData.zombieID << "' 호드 사운드 연속 재생 방지 (이번에 호드 사운드 재생 X)" << endl;
		cout << endl;
#endif
		return;
	}


#ifdef	ENABLE_BT_LOG
	cout << "좀비 '#" << ZombieData.zombieID << "' 플레이어 발견! 호드 사운드 발생!!!" << endl;
	cout << endl;
#endif


	// 다른 좀비들 플레이어 발견 소리 포착 체크 (호드 사운드 알리기)
	vector<Zombie*> zombies = iocpServer->zombieDB_BT[roomid];

	for (auto& zom : zombies) {
		if (abs(ZombieData.z - zom->ZombieData.z) > 500.f)	// 해당 좀비와 같은 층 좀비만 검사
			continue;

		if (zom->ZombieData.zombieID == ZombieData.zombieID)	// 나 자신은 넘어가기
			continue;

		vector<vector<vector<float>>> hzl = vector<vector<vector<float>>>{ {{ZombieData.x, ZombieData.y, ZombieData.z}} };
		vector<vector<vector<float>>> zl = vector<vector<vector<float>>>{ {{zom->ZombieData.x, zom->ZombieData.y, zom->ZombieData.z}} };

		float dist = sqrt(powf(hzl[0][0][0] - zl[0][0][0], 2) + powf(hzl[0][0][1] - zl[0][0][1], 2)/* + powf(hzl[0][0][2] - zl[0][0][2], 2)*/);	// x, y 좌표 거리만 검사

		bool bHear = false;

		std::random_device rd;
		std::mt19937 mt(rd());

		std::uniform_int_distribution<int> random_chance(0, 100);		// 0~100

		float the_chance = (float)random_chance(mt);		// random_chance(mt)는 부르는 매 순간 바뀌어서 이렇게 변수에 미리 넣고 사용

#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
		cout << "HearGrowlSoundRandomChance: " << the_chance << ", dist: " << dist << endl;
#endif

		float shoutingZombieAttribute = 0.f;

		if (ZombieData.zombietype == 2) {	// 샤우팅 좀비는 더 큰 호드 사운드를 만들어내므로 (추가 특성)
			shoutingZombieAttribute = 500.0f;
		}

		if (dist <= 300.f + shoutingZombieAttribute) {
			// 100퍼센트의 확률
			bHear = true;
		}
		else if (dist <= 500.f + shoutingZombieAttribute) {
			if (the_chance >= (100 - 80)) 	// 80퍼센트의 확률
				bHear = true;
		}
		else if (dist <= 800.f + shoutingZombieAttribute) {
			if (the_chance >= (100 - 60)) 	// 60퍼센트의 확률
				bHear = true;
		}
		else if (dist <= CanHearHordeSoundDistance + shoutingZombieAttribute) {
			if (the_chance >= (100 - 40)) 	// 40퍼센트의 확률
				bHear = true;
		}

		if (bHear == true) {
#ifdef	ENABLE_BT_LOG
			cout << "좀비 '#" << zom->ZombieData.zombieID << "' 가 좀비 '#" << ZombieData.zombieID << "' 의 호드 사운드 포착!!!" << endl;
			cout << endl;
#endif

			zom->HeardHordeSound = true;

			zom->HordeLocation = hzl;
		}

	}
}

// wait(애니메이션) 실행 우선순위: 피격 > 공격 > 샤우팅
void Zombie::Wait()
{
#ifdef ENABLE_BT_LOG
	cout << "### [Wait] 호출" << endl;
	cout << "좀비 '#" << ZombieData.zombieID << "' BT 잠시 대기." << endl << endl;
#endif


	// 피격이 공격 보다 위에 => 공격 중에 피격 당하면 공격 캔슬되고 피격만 되게 하게 
	if (IsBeingAttacked) {

		auto waitAfterTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> deltaTime = waitAfterTime - waitAnimStartTime;

#ifdef ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 가 피격 당하였습니다!" << endl;
#endif

		if (deltaTime.count() >= ZombieBeAttackedAnimDuration) {
#ifdef ENABLE_BT_LOG
			cout << "피격 애니메이션 재생 끝 " << endl;
#endif

			IsBeingAttacked = false;
			HaveToWait = false;

			IsAttacking = false;	// 혹시 공격중이다가 피격 당했을 경우를 대비해서 -> 리셋 개념
			IsShouting = false;		

			IsLookingAround = false;	// 혹시 둘러보기 도중에 피격 당했을 경우를 대비해서 -> 다시 리셋
			lookAroundCount = 0;

			// 애니메이션 재생 후 블랙보드(BT 플래그값들) 전부 초기화 하고 다시 검사 (마지막으로 플레이어를 봤던 위치는 기억!)
			bool clear_flag[6] = { true, true, true, true, false, true };
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;	// 블랙보드가 클리어 됨을 클라에게 targetType으로 전달 (클라도 detect 패킷 리셋 하도록[m_bPlayerInSight = false;[)])

			RandomChanceBuff_CanSeePlayer = RandomChanceBuff_CanSeePlayer_const;	// 다시 검사 할 때 일시적 확률 버프 -> 다시 더 잘 쫒아오도록 하기 위해
			RandomChanceBuff_CanSeePlayer_StartTime = std::chrono::high_resolution_clock::now();
		}
		else {
#ifdef ENABLE_BT_LOG
			cout << "피격 애니메이션 남은 시간: " << ZombieBeAttackedAnimDuration - deltaTime.count() << "s" << endl;
#endif
		}
	}
	else if (IsAttacking) {

		auto waitAfterTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> deltaTime = waitAfterTime - waitAnimStartTime;

#ifdef ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 가 공격 중입니다!" << endl;
#endif

		if (deltaTime.count() >= ZombieAttackAnimDuration) {
#ifdef ENABLE_BT_LOG
			cout << "공격 애니메이션 재생 끝 " << endl;
#endif

			IsAttacking = false;
			HaveToWait = false;

			IsShouting = false;		

			IsLookingAround = false;
			lookAroundCount = 0;

			// 애니메이션 재생 후 블랙보드(BT 플래그값들) 전부 초기화 하고 다시 검사 (마지막으로 플레이어를 봤던 위치는 기억!)
			bool clear_flag[6] = { true, true, true, true, false, true };
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;	// 블랙보드가 클리어 됨을 클라에게 targetType으로 전달 (클라도 detect 패킷 리셋 하도록[m_bPlayerInSight = false;[)])

			RandomChanceBuff_CanSeePlayer = RandomChanceBuff_CanSeePlayer_const;	// 다시 검사 할 때 일시적 확률 버프 -> 다시 더 잘 쫒아오도록 하기 위해
			RandomChanceBuff_CanSeePlayer_StartTime = std::chrono::high_resolution_clock::now();
		}
		else {
#ifdef ENABLE_BT_LOG
			cout << "공격 애니메이션 남은 시간: " << ZombieAttackAnimDuration - deltaTime.count() << "s" << endl;
#endif
		}
	}
	else if (IsShouting) {	// - 샤우팅 좀비 샤우팅 애니메이션

		auto waitAfterTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> deltaTime = waitAfterTime - waitAnimStartTime;

#ifdef ENABLE_BT_LOG
		cout << "좀비 #" << ZombieData.zombieID << " 가 샤우팅 중입니다!" << endl;
#endif

		if (deltaTime.count() >= ZombieShoutingAnimDuration) {
#ifdef ENABLE_BT_LOG
			cout << "샤우팅 애니메이션 재생 끝 " << endl;
#endif

			IsShouting = false;
			HaveToWait = false;

			IsLookingAround = false;	
			lookAroundCount = 0;

			// 애니메이션 재생 후 블랙보드(BT 플래그값들) 전부 초기화 하고 다시 검사 (마지막으로 플레이어를 봤던 위치는 기억!)
			bool clear_flag[6] = { true, true, true, true, false, true };
			ClearBlackBoard(clear_flag);
			targetType = BLACKBOARDCLEARED;	// 블랙보드가 클리어 됨을 클라에게 targetType으로 전달 (클라도 detect 패킷 리셋 하도록[m_bPlayerInSight = false;[)])
		
			RandomChanceBuff_CanSeePlayer = RandomChanceBuff_CanSeePlayer_const;	// 다시 검사 할 때 일시적 확률 버프 -> 다시 더 잘 쫒아오도록 하기 위해
			RandomChanceBuff_CanSeePlayer_StartTime = std::chrono::high_resolution_clock::now();
		}
		else {
#ifdef ENABLE_BT_LOG
			cout << "샤우팅 애니메이션 남은 시간: " << ZombieShoutingAnimDuration - deltaTime.count() << "s" << endl;
#endif
		}
	}


#ifdef ENABLE_BT_LOG
	cout << endl;
#endif
}

void Zombie::Resurrect()
{
#ifdef	ENABLE_BT_LOG
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 부활함!" << endl << endl;
	cout << "==========좀비 \'#" << ZombieData.zombieID << "\' BT 종료========//" << endl;
	cout << endl;
#endif


	// 부활 통신 작업
	int ZombieId = ZombieData.zombieID;

	Protocol::Zombie_hp packet;
	packet.set_zombieid(ZombieId);
	packet.set_damage(ZombieStartHP * (-1.f));		// 계산은 '원래HP - 데미지' 이런식이니까, -StartHP 이렇게 넘겨줘서 오히려 체력이 원상복구되는 효과를 봄
	packet.set_packet_type(12);

	std::string serializedData;
	packet.SerializeToString(&serializedData);

	for (const auto& player : playerDB_BT[roomid]) {
		iocpServer->IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
	}


	// 좀비 체력 원상복구
	SetHP(ZombieStartHP);


	// 플래그 값들 초기화
	bool clear_flag[6] = { true, true, true, true, true, true };
	ClearBlackBoard(clear_flag);
	targetType = BLACKBOARDCLEARED;

	HaveToWait = false;

	IsBeingAttacked = false;
	IsAttacking = false;
	IsShouting = false;
	
	IsRunaway = false;
	runawayFailCount = 0;	// 도망치기 실패 카운트 초기화

	CanSeePlayer_result = false;
	detectCanSeePlayer_randomChance = false;
	detectHasFootSound_randomChance = false;
	RandomChanceBuff_CanSeePlayer = 0.f;

	IsLookingAround = false;
	lookAroundCount = 0;	// 주위 둘러보기 카운트 초기화

	// 샤우팅 좀비일 경우에
	if (ZombieData.zombietype == 1) {
		ShoutingZombie* sz = dynamic_cast<ShoutingZombie*>(this);  // 다운 캐스팅 사용!
		sz->bShouted = false;
	}
}

void Zombie::FleeRandChance()
{
#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << "### (Flee Decorator) 호출" << endl;
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 도망가기 랜덤확률 계산" << endl << endl;
#endif

	float randChance_below80p = 0.f;	// 체력이 80% 이하일때 도망갈 확률
	float randChance_below50p = 0.f;	// 체력이 50% 이하일때 도망갈 확률
	float randChance_below30p = 0.f;	// 체력이 30% 이하일때 도망갈 확률
	float randChance_below10p = 0.f;	// 체력이 10% 이하일때 도망갈 확률

	switch (ZombieType) {
	case NORMAL_ZOMBIE:
		randChance_below80p = 5.f;
		randChance_below50p = 30.f;
		randChance_below30p = 60.f;
		randChance_below10p = 70.f;
		break;
	case RUNNING_ZOMBIE:
		randChance_below80p = 10.f;
		randChance_below50p = 50.f;
		randChance_below30p = 70.f;
		randChance_below10p = 100.f;
		break;
	case SHOUTING_ZOMBIE:
		randChance_below80p = 3.f;
		randChance_below50p = 10.f;
		randChance_below30p = 30.f;
		randChance_below10p = 50.f;
		break;
	}

	float currentZHP_percent = (GetHP() / ZombieStartHP) * 100.f;

	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> random_chance(0, 100);		// 0~100

	float the_chance = (float)random_chance(mt);		// random_chance(mt)는 부르는 매 순간 바뀌어서 이렇게 변수에 미리 넣고 사용

	float randChance_below_percent = 0;

	if (currentZHP_percent <= 10.f) {	// 체력이 10% 이하일때
		randChance_below_percent = randChance_below10p;
	}
	else if (currentZHP_percent <= 30.f) {	// 체력이 30% 이하일때
		randChance_below_percent = randChance_below30p;
	}
	else if (currentZHP_percent <= 50.f) {	// 체력이 50% 이하일때
		randChance_below_percent = randChance_below50p;
	}
	else if (currentZHP_percent <= 80.f) {	// 체력이 80% 이하일때
		randChance_below_percent = randChance_below80p;
	}

#ifdef ENABLE_BT_FLEE_LOG
	cout << "currentZHP_percent: " << currentZHP_percent << ", randChance_below_percent: " << randChance_below_percent << ", DoRunaway chance: " << the_chance << endl;
#endif

	if (the_chance <= randChance_below_percent) {
		IsRunaway = true;
	}

	if (IsRunaway == true) {
#ifdef ENABLE_BT_FLEE_LOG
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 도망치기!!!" << endl;
#endif

		RandPatrolSet = false;	// => 도망치기 랜덤 패트롤 다시 돌릴 꺼니까 초기화
		IsStandingStill = false;	// 이미 이전에 도망치고 나서 숨어서 숨고르기 하다가 다시 피격 당한 경우가 있으니 다시 초기화

		IsLookingAround = false;	// 혹시 둘러보기 도중에 피격을 당해 도망치기를 했을 경우를 대비해서 -> 다시 리셋 (도망치기 끝나고 나중에 다시 주위 둘러보기를 잘 할 수 있도록)
		lookAroundCount = 0;
	}

#ifdef ENABLE_BT_FLEE_LOG
	cout << endl;
#endif
}

void Zombie::Flee()
{
#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << "=== Sequence {Flee} 호출" << endl;
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 도망치기 중!" << endl << endl;
#endif

	Flee_CanSeePlayer();

}

// 사실 시퀀스로 따로 클래스 만들어서 사용해야 하는데 귀찮고 Flee에서만 사용할 꺼고 간단하니까 그냥 여기서 구현;;
void Zombie::Flee_CanSeePlayer()
{
#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << "{Flee}의 (CanSeePlayer Decorator) 호출" << endl;
#endif

	bool d_result = false;

	int maxInSightFailCount = 5;		// BT 한싸이클에 시야 검사 최대 실패 허용 횟수
	int maxTotalFailCount = 3;			// 도망가기 랜덤 패트롤 찾기 최대 실패 허용 횟수 (= maxInSightFailCount * maxTotalFailCount 가 실제 총 최대 실패 허용 횟수) 
	

	bool pis_result = PlayerInSight_Update_Check();
#ifdef ENABLE_BT_FLEE_LOG
	cout << "좀비 \'#" << ZombieData.zombieID << "\' 의 시야에 플레이어가 있는가?: " << boolalpha << pis_result << endl;
#endif

	bool already_did_the_randpatrol_test = false;	// 밑에 도망치기 패트롤 최초 설정에서 필요 

	if (pis_result == true) {
		vector<vector<vector<float>>> closest_player_pos = {};
		float dist = SearchClosestPlayer(closest_player_pos, 1);
#ifdef ENABLE_BT_FLEE_LOG
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 와 가장 가까운 플레이어 사이의 거리: " << dist << endl;
#endif

		if (dist <= 1000.f) {	// 시야에 플레이어가 존재하며 10m 이내라면 => 다시 다른 방향으로 도망가기

			bool pis_rp_result = false;
			int try_cnt = 0;
			while (pis_rp_result == false) {

				try_cnt++;
#ifdef	ENABLE_BT_FLEE_LOG
				cout << "좀비 #" << ZombieData.zombieID << " 도망가기 랜덤 패트롤 찾기 시도 - #" << try_cnt << endl;
#endif

				pis_rp_result = RandomPatrol();

				already_did_the_randpatrol_test = true;

				if (pis_rp_result == true) {
					Vector2 zombiePos = { ZombieData.x, ZombieData.y };         // 좀비 위치
					Vector2 zombieForward = { TargetLocation[0][0][0], TargetLocation[0][0][1] };     // 좀비가 바라볼 방향 (랜덤 패트롤 지점)
					Vector2 playerPos = { 0, 0 };         // 플레이어 위치
					float viewAngle = 180.0f;            // 시야각 (FOV) => 실제로는 120도가 시야각이지만 도망가기 지점은 정반대 방향을 바라보도록 하기위해 그냥 전방 180도로 설정

					for (auto player : playerDB_BT[roomid]) {
						if (DistanceTo_PlayerInsight.find(player.first) != DistanceTo_PlayerInsight.end()) {	// DistanceTo_PlayerInsight에 저장된 값이 있다면 
							if (DistanceTo_PlayerInsight[player.first] <= 1000.f) {								// && 거리가 1000 이하라면
								// = 해당 플레이어가 시야에 존재했다면 => 다시는 그방향으로 도망가지 않도록
								playerPos.x = player.second.x;
								playerPos.y = player.second.y;

								// 좀비가 바라볼 방향 (랜덤 패트롤 지점)이 좀비의 시야각 (180도)이내에 플레이어가 위치하는 경우 다시 RandomPatrol 수행	
								if (CheckFleeRandomPoint(zombiePos, zombieForward, playerPos, viewAngle) == true) {
									RandPatrolSet = false;	// RandomPatrol 마지막에 무조건 'RandPatrolSet = true;' 하는 코드 들어가 있어서;
									pis_rp_result = false;

#ifdef	ENABLE_BT_FLEE_LOG
									cout << "좀비 #" << ZombieData.zombieID << " 도망가기 랜덤 패트롤 찾기 실패! (새로운 찾은 패트롤 포인트 방향에 플레이어 이미 존재함!) 다시 검색!!!" << endl;
									cout << "=> 플레이어 위치 : (" << playerPos.x << ", " << playerPos.y << ") , 좀비 위치: (" << ZombieData.x << ", " << ZombieData.y
										<< ") , 새로운 찾은 패트롤 포인트 : (" << TargetLocation[0][0][0] << ", " << TargetLocation[0][0][1] << ")" << endl;
#endif

									break;	// 한명이라도 시야 체크에 걸리면 실패!
								}
								else {
									pis_rp_result = true;
								}
							}
						}
					}	// for문
				}

				if (try_cnt >= maxInSightFailCount && pis_rp_result == false) {					// 시야범위 밖 랜덤 패트롤 목표점 찾기 maxInSightFailCount번 까지만 시도
#ifdef	ENABLE_BT_FLEE_LOG
					cout << "좀비 #" << ZombieData.zombieID << " 도망가기 랜덤 패트롤 찾기 결국 실패!!! (연속 " << maxInSightFailCount << "번 실패...) = > 좀비 해당 한 틱동안 멍때림!!" << endl;
#endif

					break;
				}
			}	// while문

			if (pis_rp_result == true) {
#ifdef	ENABLE_BT_FLEE_LOG
				cout << "도망가기 랜덤 패트롤 찾기 성공!!! (시야 검사까지 통과)" << endl;
				cout << "TargetLocation[Flee-Point]: ( " << TargetLocation[0][0][0] << " , " << TargetLocation[0][0][1] << " , " << TargetLocation[0][0][2] << " )" << endl;
#endif
			}

			d_result = pis_rp_result;
		}
	}	

	if (RandPatrolSet == false	// 여기까지 왔는데 패트롤이 아직 미지정? ===> 도망치기를 이제 시작했는데 시야체크에 안 걸렸었다는 뜻! -> 최초로 도망가기 할 때 위에서 시야 체크에 플레이어가 없었다면 랜덤 패트롤을 아직 지정 안하기 때문에 따로 여기서 해줌
		&& already_did_the_randpatrol_test == false) {	// runawayFailCount를 한 BT 싸이클에 두번 중복 중가 시키지는 않게 하려고 -> 위에 시야체크에 들어가서 패트롤 찾기를 했지만 결국 실패한 경우
#ifdef ENABLE_BT_FLEE_LOG
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 도망가기 포인트 최초 설정!" << endl;
#endif

		runawayHealthRegenLastTime = std::chrono::high_resolution_clock::now();	// 체력 회복 시작 시간

		bool rp_result = false;
		int try_cnt = 0;
		while (rp_result == false) {
			try_cnt++;
#ifdef	ENABLE_BT_FLEE_LOG
			cout << "좀비 #" << ZombieData.zombieID << " 도망가기 최초 랜덤 패트롤 찾기 시도 - #" << try_cnt << endl;
#endif
			rp_result = RandomPatrol();

			if (try_cnt >= maxInSightFailCount && rp_result == false) {					// 랜덤 패트롤 목표점 찾기 maxInSightFailCount번 까지만 시도 (이것도 그냥 시야 체크 실패 허용 횟수만큼)
#ifdef	ENABLE_BT_FLEE_LOG
				cout << "좀비 #" << ZombieData.zombieID << " 도망가기 최초 랜덤 패트롤 찾기 결국 실패!!! (연속 " << maxInSightFailCount << "번 실패...) => 좀비 해당 한 틱동안 멍때림!!" << endl;
#endif

				RandPatrolSet = false;	// 다시 최초 랜덤 패트롤 찾도록...

				break;
			}
		}

		if (rp_result == true) {
#ifdef	ENABLE_BT_FLEE_LOG
			cout << "도망가기 최초 랜덤 패트롤 찾기 성공!!!" << endl;
			cout << "TargetLocation[Flee-Point]: ( " << TargetLocation[0][0][0] << " , " << TargetLocation[0][0][1] << " , " << TargetLocation[0][0][2] << " )" << endl;
#endif
		}

		d_result = rp_result;
	}

	if (RandPatrolSet == true) {	// 이전에 이미 도망치기 랜덤 패트롤 포인트를 찾은 상태라면
		d_result = true;
	}

#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << endl;
#endif

	if (d_result == true) {	// 유효한 랜덤 패트롤 지점임과 동시에 플레이어가 없는 방향으로 패트롤을 찍었을때 

		IsStandingStill = false;	// 이전에 숨고르기를 하고 있던 상태였다면 초기화 해야 하니

		HealthRegenerate();
		Runaway();

		runawayFailCount = 0;	// 도망치기 실패 카운트 초기화
	}
	else if (d_result == false) {	// 유효한 패트롤 지점을 못 찾았을때
		runawayFailCount++;

		if (runawayFailCount < maxTotalFailCount) {
			HealthRegenerate();	// 길찾기 실패하면 움직이지는 않고 체력 회복만
			// 클라에서도 MoveTo가 움직이지 않도록 하기 위해서 -> 제자리에 정지
			get<0>(path[ZombiePathIndex]) = ZombieData.x;
			get<1>(path[ZombiePathIndex]) = ZombieData.y;
			SetTargetLocation(Zombie::TARGET::RUNAWAY);

#ifdef	ENABLE_BT_FLEE_LOG
			cout << "좀비 #" << ZombieData.zombieID << " 도망가기 랜덤 패트롤 찾기 총 연속 " << runawayFailCount * maxInSightFailCount << "번 실패!!!" << endl;
#endif
		}
		else if (runawayFailCount >= maxTotalFailCount) {
			IsRunaway = false;	// 도망치기 끝내기
			IsStandingStill = false;	// 숨고르기 하고 있었다면 초기화
			RandPatrolSet = false;	// 다시 패트롤 하도록
			KnewPlayerLocation = false;	// 도망가기 직전에 플레이어 위치를 기억하고 있어서 얘도 초기화

			makeHordeSound_StartTime = std::chrono::high_resolution_clock::now() - std::chrono::seconds(30);	// 호드 사운드 강제로 실행시키려고 30초 전에 시간을 넣음
			MakeNoise(); // 호드 사운드 내기 => 도움 요청! & 플레이어에게 더이상 도망가는 상태가 아님을 알림

#ifdef	ENABLE_BT_FLEE_LOG
			cout << "좀비 #" << ZombieData.zombieID << " 도망가기 랜덤 패트롤 찾기 결국 실패!!! (총 연속 " << maxInSightFailCount * runawayFailCount << "번(= " <<
				maxInSightFailCount << "번x" << runawayFailCount << "번) 실패...) => 좀비 코너에 몰림 ===> 도망가지 않고 맞서 싸움!!!!!" << endl;
#endif

			runawayFailCount = 0;	// 도망치기 실패 카운트 초기화
		}
	}

}

// 좀비의 새로운 도망지점이 플레이어쪽 방향인지 아닌지 체크
bool Zombie::CheckFleeRandomPoint(Vector2 zombiePos, Vector2 zombieForward, Vector2 playerPos, float viewAngle) {
	Vector2 playerDir = { playerPos.x - zombiePos.x, playerPos.y - zombiePos.y };  // 좀비 → 플레이어 벡터
	playerDir = playerDir.Normalize();  // 정규화

	zombieForward = zombieForward.Normalize();  // 정규화

	float dot = zombieForward.Dot(playerDir);  // 점곱
	float cosHalfFOV = std::cos(viewAngle * 0.5f * (PI / 180.0f)); // FOV의 절반 각도를 cos 값으로 변환

	return dot >= cosHalfFOV;  // dot 값이 cos(절반 FOV)보다 크면 시야 안에 있음
}

void Zombie::Runaway()
{
#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << "{Flee_CanSeePlayer}의 [Runaway] 호출" << endl;
#endif
	
	SetTargetLocation(Zombie::TARGET::RUNAWAY);

	MoveTo(IOCP_CORE::BT_deltaTime.count());
}

void Zombie::HealthRegenerate()
{
#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << "{Flee_CanSeePlayer}의 [HealthRegenerate] 호출" << endl;
#endif

	auto waitAfterTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = waitAfterTime - runawayHealthRegenLastTime;

	if (deltaTime.count() >= runawayHealthRegenInterval
		&& GetHP() > 0) {	// 좀비 체력관련 데이터 레이스 발생 방지 코드 => (이벤트 방식-비동기적으로)피격으로 좀비 체력을 같이 수정하니까 ===> 좀비 죽고 시체 상태로 안 움직이도록 방지 / 이미 죽었는데 체력이 회복되는 거 방지
		
		SetHP(GetHP() + runawayHealthRegenPoint);
		
		float sendDamage = runawayHealthRegenPoint;

		if (GetHP() > ZombieStartHP) {	// 어차피 100%가 기준점이니까 100% 이상은 체력 회복 안되도록
			sendDamage = GetHP() - ZombieStartHP;
			SetHP(ZombieStartHP);
		}

		// 좀비 도망치기 체력 회복 동기화 통신 작업
		int ZombieId = ZombieData.zombieID;

		Protocol::Zombie_hp packet;
		packet.set_zombieid(ZombieId);
		packet.set_damage(sendDamage * (-1.f));	// 계산은 '원래HP - 데미지' 이런식이니까, -hpRegen 이렇게 넘겨줘서 오히려 체력이 회복되는 효과를 봄
		packet.set_packet_type(12);

		std::string serializedData;
		packet.SerializeToString(&serializedData);

		for (const auto& player : playerDB_BT[roomid]) {
			iocpServer->IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
		}


		runawayHealthRegenLastTime = std::chrono::high_resolution_clock::now();	// 시간 다시 초기화

#ifdef ENABLE_BT_FLEE_LOG
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 체력 회복:" << GetHP() << endl;
#endif
	}
	else {
#ifdef ENABLE_BT_FLEE_LOG
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 체력 회복 중 - 다음 회복까지 남은 시간:" << runawayHealthRegenInterval - deltaTime.count() << "s" << endl;
#endif
	}

	float currentZHP_percent = (GetHP() / ZombieStartHP) * 100.f;

	if (currentZHP_percent >= runawayHealthRegenMaxPercent	// 일정 체력 이상을 회복하면 
		|| GetHP() >= ZombieStartHP) {	// 어차피 100%가 기준점이니까 (이상하게 체력 100% 이상으로도 계속 회복하길래 방지코드로 넣음)
		SetHP(ZombieStartHP);

		IsRunaway = false;	// 도망치기 끝내기
		IsStandingStill = false;	// 숨고르기 하고 있었다면 초기화
		RandPatrolSet = false;	// 다시 패트롤 하도록
		KnewPlayerLocation = false;	// 도망가기 직전에 플레이어 위치를 기억하고 있어서 얘도 초기화

		runawayFailCount = 0;	// 도망치기 실패 카운트 초기화

#ifdef ENABLE_BT_FLEE_LOG
		cout << "좀비 \'#" << ZombieData.zombieID << "\' 체력 회복 완료!!! - " << GetHP() << " => 도망치기 종료" << endl;
#endif
	}

#if defined(ENABLE_BT_FLEE_LOG) || defined(ENABLE_BT_LOG)
	cout << endl;
#endif

}

void Zombie::LookAround()
{
#ifdef ENABLE_BT_NODE_LOG
	cout << "[LookAround] 호출!" << endl;
#endif


	targetType = LOOKINGAROUND;


	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> random_chance(-4000, 4000);		// -4000~4000 <- 맵 x,y 사이즈가 이 범위 안에 들어가서

	float rand_look_x = (float)random_chance(mt);		// random_chance(mt)는 부르는 매 순간 바뀌어서 이렇게 변수에 미리 넣고 사용
	float rand_look_y = (float)random_chance(mt);


	auto lookaroundAfterTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = lookaroundAfterTime - lookAroundStartTime;

#ifdef ENABLE_BT_NODE_LOG
	cout << "주위 둘러보기 경과 시간: " << deltaTime.count() << "s" << endl;
#endif

	if (0.0f <= deltaTime.count() && deltaTime.count() < 1.0f) {		// 0초대에 가만히  
#ifdef ENABLE_BT_NODE_LOG
		cout << "주위 둘러보기 - 가만히..." << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
	else if (deltaTime.count() >= 1.0f && lookAroundCount == 0) {	// 1~3초에 1번째 주위 둘러보기  
		get<0>(path[ZombiePathIndex]) = rand_look_x;
		get<1>(path[ZombiePathIndex]) = rand_look_y;
		lookAroundCount++;

#ifdef ENABLE_BT_NODE_LOG
		cout << "rand_look_x: " << rand_look_x << ", rand_look_y: " << rand_look_y << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
	else if (deltaTime.count() >= 3.0f && lookAroundCount == 1) {	// 3~5초에 2번째 주위 둘러보기  
		get<0>(path[ZombiePathIndex]) = rand_look_x;
		get<1>(path[ZombiePathIndex]) = rand_look_y;
		lookAroundCount++;

#ifdef ENABLE_BT_NODE_LOG
		cout << "rand_look_x: " << rand_look_x << ", rand_look_y: " << rand_look_y << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
	else if (deltaTime.count() >= 5.0f && lookAroundCount == 2) {	// 5~7초에 3번째 주위 둘러보기  
		get<0>(path[ZombiePathIndex]) = rand_look_x;
		get<1>(path[ZombiePathIndex]) = rand_look_y;
		lookAroundCount++;

#ifdef ENABLE_BT_NODE_LOG
		cout << "rand_look_x: " << rand_look_x << ", rand_look_y: " << rand_look_y << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
	else if (7.0f <= deltaTime.count()  && deltaTime.count() < 8.0f) {	// 7초대에는 그냥 가만히
#ifdef ENABLE_BT_NODE_LOG	
		cout << "주위 둘러보기 - 가만히..." << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
	else if (deltaTime.count() >= 8.0f) {							// 8초를 넘기면 이제 주위 둘러보기 그만
		IsLookingAround = false;

#ifdef ENABLE_BT_NODE_LOG
		cout << "주위 둘러보기 끝!" << endl;
		cout << "lookAroundCount: " << lookAroundCount << endl;
#endif
	}
}