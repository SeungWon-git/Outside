#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>
//#include <mutex>

#include "Task.h"

//#include "Player.h"

#include "ZombiePathfinder.h"

#include "iocpServerClass.h"


using std::vector;
using std::string;
using std::tuple;
using std::map;


// 전방 선언 (순환 포함 문제를 피하기 위해)
class Task;
//class Player;
class IOCP_CORE;           
class ZombiePathfinder; 


// 통신에서 주로 사용할 데이터 클래스
class Zombie_Data {
public:
    int zombieID;
    float x, y, z;
    float pitch, yaw, roll;
    int zombietype;             // 0 - 일반 좀비, 1 - 샤우팅 좀비, 2 - 뛰는 좀비
    int roomID;

    // 사실상 밑에 두개는 사용 X
    //int patroltype;
    //float patrolrange;

    Zombie_Data() = default;
    Zombie_Data(int zid, float x, float y, float z, float pitch, float yaw, float roll, int ztype) 
        : zombieID(zid), x(x), y(y), z(z), pitch(pitch), yaw(yaw), roll(roll), zombietype(ztype) {} //초기화 리스트
};

class Zombie {
public:
    enum TARGET {
        NULL_TARGET = 1,
        PLAYER,
        SHOUTING,
        FOOTSOUND,
        INVESTIGATED,
        PATROL,
        HORDESOUND,
        RUNAWAY,
        WAIT,
        LOOKINGAROUND,

        BLACKBOARDCLEARED = 69
    };

    enum ZOMBIE_TYPE {
        NULL_TYPE,
        NORMAL_ZOMBIE,
        RUNNING_ZOMBIE,
        SHOUTING_ZOMBIE
    };


    struct Vector2 {
        float x, y;

        // 벡터 정규화 (길이를 1로 만들기)
        Vector2 Normalize() const {
            float length = std::sqrt(x * x + y * y);
            if (length == 0) return { 0, 0 };
            return { x / length, y / length };
        }

        // 벡터 점곱 (Dot Product)
        float Dot(const Vector2& other) const {
            return x * other.x + y * other.y;
        }

        // 벡터 길이 (Magnitude)
        float Length() const {
            return std::sqrt(x * x + y * y);
        }
    };


    Zombie();
    Zombie(IOCP_CORE* mainServer, Zombie_Data zd);
    virtual ~Zombie();


    IOCP_CORE* iocpServer;
    //std::mutex zombieMutex;


    vector<tuple<float, float, float>> path;        // PathFinder로부터 받을 경로 좌표값들 저장
    vector<tuple<float, float, float>> beforepath;  // PathFinder로부터 받은 이전 경로 저장 (비교용)


    const float CanAttackDistance = 150.f;          // 공격 사정거리 150cm (언리얼 단위로는 => 150UU)
    const float CanSeePlayerDistance = 2500.0f + 50.f;    // 최대 시야거리 2500cm + 50cm (원래 2500이지만 실제 검사에서는 2500보다 약간 크게 잡혀도 통신보내서 +50까지 봐줌;;)
    const float CanHearFootSoundDistance = 1000.f;            // 발소리 포착 최대 가능거리 1000cm
    const float CanHearShoutDistance = 3000.f;      // 샤우팅 소리 포착 가능 거리 3000cm
    const float CanHearHordeSoundDistance = 1000.0f;         // 호드 사운드 포착 가능 거리 1000cm

    const float ZombieAttackAnimDuration = 2.63f;    // 좀비 공격 애니메이션 재생 시간 (* 정확히는 2.63초)
    const float ZombieBeAttackedAnimDuration = 2.0f;    // 좀비 피격 애니메이션 재생 시간 (* 정확히는 2.00초)
    const float ZombieShoutingAnimDuration = 2.8f;    // 좀비 샤우팅 애니메이션 재생 시간 (* 정확히는 2.80초)

    float ZombieStandingStillDuration = 0.f;    // 좀비 숨고르기 (멍때리기) 지속 시간 (5~10초)
    const float ZombieMakeHordeNoiseDelay = 20.0f;   // 호드 사운드 재생 딜레이 (20초)

    const float NormalZombieStartHP = 20.0f;        // 20.0f
    const float NormalZombieSpeed = 200.0f;         // 200.0f (뛰기 스피드)
    const float NormalZombieWalkSpeed = 100.0f;     // 100.0f (걷기 스피드)
     
    const float RunningZombieStartHP = 20.0f;        // 20.0f
    const float RunningZombieSpeed = 300.0f;         // 300.0f (뛰기 스피드)
    const float RunningZombieWalkSpeed = 200.0f;     // 200.0f (걷기 스피드)

    const float ShoutingZombieStartHP = 30.0f;        // 30.0f
    const float ShoutingZombieSpeed = 230.0f;         // 230.0f (뛰기 스피드)
    const float ShoutingZombieWalkSpeed = 150.0f;     // 150.0f (걷기 스피드)

    float ZombieStartHP = 0.f;
    float ZombieSpeed;

    ZOMBIE_TYPE ZombieType;

    Zombie_Data ZombieData;     // 통신에서 주로 사용할 데이터

    ZombiePathfinder pathfinder;

    map<int, float> DistanceTo_PlayerInsight;     // Detect용 맵 <플레이어 인덱스, 좀비-플레이어 거리>

    map<int, float> DistanceTo_FootSound;         // FootSound용 맵 <플레이어 인덱스, 좀비-플레이어 거리>

    vector<vector<vector<float>>> TargetLocation;

    vector<vector<vector<float>>> LastKnownTargetLocation;       // 플레이어를 마지막으로 본 위치

    vector<vector<vector<float>>> ShoutingLocation;       // 샤우팅 좀비 위치

    vector<vector<vector<float>>> HordeLocation;       // 호드 좀비 위치

    std::chrono::steady_clock::time_point WalkingSamePosition_StartTime;
    bool IsWalkingSamePosition = false;
    const float Reset_WalkingSamePosition_Duration = 2.0f;  // 2초

    int ClosestPlayerID;

    bool PlayerInSight;
    bool KnewPlayerLocation;
    bool HeardShouting;
    bool HeardFootSound;
    bool HeardHordeSound;

    bool RandPatrolSet;     // 랜덤 패트롤 지점이 set 되면 true

    bool IsAttacking;       // 해당 좀비 지금 공격 중인가? (애니메이션 재생 중인가?)
    bool IsBeingAttacked;   // 해당 좀비 지금 피격 당하는 중인가? (애니메이션 재생 중인가?)
    bool IsShouting;        // 해당 '샤우팅' 좀비 지금 샤우팅 중인가? (애니메이션 재생 중인가?) 
                            //-> 샤우팅 좀비만을 위한 변수지만, shouting zombie(하위-자식 클래스)에 만들면 zombie(상위-부모 클래스)에서 shouting zombie의 멤버 변수에 접근 못해서 그냥 여기에;;

    bool IsStandingStill;   // 해당 좀비가 잠시 가만히 서있는 상태인가? (숨고르기)

    bool HaveToWait;        // BT가 대기상태를 해야 하는지 판별 => 애니메이션 재생 상태일때

    bool IsRunaway = false; // 좀비가 도망가기 상태인가?

    bool IsLookingAround = false; // 좀비가 주위를 둘러보는 상태인가?

    FLOOR z_floor;          // 좀비가 스폰 된 층

    std::chrono::steady_clock::time_point waitAnimStartTime;            // 좀비 대기 시작 시간 (애니메이션을 맞춰 대기하기 위한 용도)
    std::chrono::steady_clock::time_point waitBrainlessStartTime;      // 좀비 대기 시작 시간 (숨고르기를 맞춰 대기하기 위한 용도)

    bool CanSeePlayer_result = false;       // 호드 사운드 재생 시점 & 플레이어 시야에서 잠시 놓쳤을떄 다시 검사에서 일시적 버프 주는 곳에서 필요

    bool detectCanSeePlayer_randomChance = false;   // 포착 한번 성공하면 계속 포착하도록 도와주는 플래그 (다시 포착 성공 확률 안 돌아가도록)  
    bool detectHasFootSound_randomChance = false;   // 포착 한번 성공하면 계속 포착하도록 도와주는 플래그 (다시 포착 성공 확률 안 돌아가도록) 

    std::chrono::steady_clock::time_point detectCanSeePlayerFail_StartTime;     // 좀비 탐지 확률 fail시에 지속해서 계속 fail 시켜주기 위한 용도 => 플레이어 시야에 포착
    std::chrono::steady_clock::time_point detectHasFootSoundFail_StartTime;     // => 플레이어 발소리

    float detectCanSeePlayerFail_delayTime = 0;
    float detectHasFootSoundFail_delayTime = 0;

    std::chrono::steady_clock::time_point makeHordeSound_StartTime;     // 호드 사운드 재생 시작 시간

    float RandomChanceBuff_CanSeePlayer = 0;    // 애니메이션 재생 후 또는 한번 포착후 금방 놓쳤을 때, 다시 포착 검사 할때 조금의 버프를 주어 플레이어를 더욱 쉽게 다시 찾도록 하기위해 사용
    const float RandomChanceBuff_CanSeePlayer_const = 50.0f;    // +50.f
    std::chrono::steady_clock::time_point RandomChanceBuff_CanSeePlayer_StartTime;
    const float RandomChanceBuff_CanSeePlayer_Duration = 5.0f;  // 5초

    std::chrono::steady_clock::time_point resurrectionStartTime;    // 좀비 부활 타이머 시작 시간
    const float resurrectionTimer = 15.f + 0.5f; // 좀비 부활 타이머 세팅 (15.5초 - 원래는 15초인데 +0.5초 해서 네트워크 딜레이까지 고려)

    std::chrono::steady_clock::time_point runawayHealthRegenLastTime;   // 좀비 도망치기 체력회복 마지막(직전) 시간
    float runawayHealthRegenInterval = 9999.0f; // 좀비 도망치기 체력회복 인터벌 (노말좀비: +1씩 2초간격, 러닝좀비: +1씩 1초간격, 샤우팅좀비: +4씩 3초간격)
    float runawayHealthRegenPoint = 0.0f;   // 좀비 도망치기 체력회복 크기 (노말좀비: +1씩 2초간격, 러닝좀비: +1씩 1초간격, 샤우팅좀비: +4씩 3초간격)
    float runawayHealthRegenMaxPercent = 100.0f; // 좀비 도망치기 최대 체력회복 퍼센트 (100%)
    int runawayFailCount = 0;   // 도망가기 랜덤 패트롤 찾기 실패 카운트 

    std::chrono::steady_clock::time_point lookAroundStartTime;
    int lookAroundCount = 0;

    TARGET targetType;  // 현재 쫓아가고 있는 타겟의 타입 (1-NULL_TARGET, 2-PLAYER, 3-SHOUTING, 4-FOOTSOUND, 5-INVESTIGATED, 6-PATROL, 7-HORDESOUND, 8-RUNAWAY, 9-WAIT, 10-LOOKINGAROUND, 69-BLACKBOARDCLEARED)

    int ZombiePathIndex = 0;

    float zombieHP;

    bool printLog = true;

    float takeABreak_chance = 60.f; // 숨고르기 확률 - 60퍼센트

    int roomid = 0;     // 사실 ZombieData에 roomID라고 따로 이미 가지고 있긴한데(맨첨에 초기화도 이값을 통해서 함) 부를 때마다 ZombieData.roomID하기에 넘 귀찮아서 사용... 
    // 참고로 생성자에서 초기화해줌 (ZombieData.roomID로)


    void DetermineFloor(float startZ);

    float SetDistance(int playerid, int distanceType);    // distanceType = 1: Detect / 2: FootSound
    
    void ClearDistance(int playerid, int distanceType); // distanceType = 1: Detect / 2: FootSound
    //원래 float SetDistance(int playerid, int distanceType, bool b_clear); 이런식으로 함수 오버로딩을 하려 했는데, 그럼 SetDistance(1,3)으로 불러도 SetDistance(1,3,true)로 3을 암시적 변환을 해버림;;

    void SetTargetLocation(TARGET t);

    void Attack(int roomid);

    bool PlayerInSight_Update_Check();

    bool FootSound_Update_Check();

    void Wait();    // 애니메이션 재생 이후 초기화 작업들을 하고 이동은 잠시 멈추는 역활이라 wait

    void MoveTo(float deltasecond);

    bool IsPathUpdated();

    bool RandomPatrol();

    void ReachFinalDestination();

    // 0-PlayerInsight, 1-HeardShouting, 2-FootSound, 3-HordeSound, 4-KnewPlayerLocation, 5-RandPatrolSet
    void ClearBlackBoard(bool clear_flag[6]);   

    void UpdatePath();
    // 얜 랜덤 패트롤, 랜덤 발소리 위치용 - 함수 오버로딩
    void UpdatePath(vector<tuple<float, float, float>> newPatrol_path);

    bool CheckPath(vector<tuple<float, float, float>>& goalTest_path, float goalTestX, float goalTestY, float goalTestZ);

    float SearchClosestPlayer(vector<vector<vector<float>>>& closest_player_pos, int distanceType);

    // 발소리 포착시에 거리에 따라 탐지 실수하도록 하는 함수
    void SetRandomTargetLocation(vector<vector<vector<float>>> target_original_pos);

    bool SearchRandomWalkableLocation(vector<vector<vector<float>>> target_original_pos, int search_radius);

    float GetHP() const { return zombieHP; }
    void SetHP(float hp) { zombieHP = hp; }

    float GetSpeed() const { return ZombieSpeed; }
    void SetSpeed(float speed) { ZombieSpeed = speed; }

    ZOMBIE_TYPE GetZombieType() const { return ZombieType; }
    void SetZombieType(ZOMBIE_TYPE z_type) { ZombieType = z_type; }

    bool CanSeePlayerRandomChance();
    bool HasFootSoundRandomChance();

    void MakeNoise();

    void TakeABreak();

    void Resurrect();

    void FleeRandChance();

    void Flee();

    void Flee_CanSeePlayer();

    bool CheckFleeRandomPoint(Vector2 zombiePos, Vector2 zombieForward, Vector2 playerPos, float viewAngle);

    void Runaway();

    void HealthRegenerate();

    void LookAround();

};