#pragma once

#include <vector>
#include <string>

#include "ZombiePathfinder.h"

using std::vector;
using std::string;

class Player {
public:
	//서버에서 실행되니 서버가 해킹되지 않는 이상 public으로 편하게 써도 될듯하다
	float x, y, z;

	FLOOR floor;

	std::string username;

	int killcount;

	//================================아래 변수들은 혹시 나중에 사용 할지 안할지 몰라서 회의해보고 수정하기
	float health;		//체력

	bool IsRunning;

	//int characterClass;	//캐릭터 클래스(종류)

	//int holdingItem;	//들고 있는 아이템

	//int animation;		//재생 중인 애니메이션


	Player();

	Player(float x, float y, float z);

	void Move(float dx, float dy, float dz);

	void Teleport(float tx, float ty, float tz); //디버깅용

	~Player();

	int send_cnt = 0;	// 안 쓰는 변수 같음
};