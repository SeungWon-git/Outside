#pragma once

#include "Selector.h"
#include "ShoutingZombie.h"

class Sel_CanSeePlayer : public Selector {
public:
    Sel_CanSeePlayer() { t_name = "CanSeePlayer"; }


    bool Detect(Zombie& zom) override {
#ifdef ENABLE_BT_NODE_LOG
        cout << "<Detect>의 (CanSeePlayer Decorator) 호출" << endl;
#endif

        d_result = zom.PlayerInSight_Update_Check();
#if defined(ENABLE_BT_NODE_LOG) || defined(ENABLE_BT_DETECT_RANDOMCHANCE_LOG)
        cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' 의 시야에 플레이어가 있는가?: " << boolalpha << d_result << endl;
#endif

        bool Prev_CanSeePlayer_result = zom.CanSeePlayer_result;
        bool Current_CanSeePlayer_result = d_result;

        if (Prev_CanSeePlayer_result == true && Current_CanSeePlayer_result == false) { // 플레이어를 잠시 놓쳤을때
            zom.RandomChanceBuff_CanSeePlayer = zom.RandomChanceBuff_CanSeePlayer_const;	// 다시 검사 할 때 일시적 확률 버프 -> 다시 더 잘 쫒아오도록 하기 위해
            zom.RandomChanceBuff_CanSeePlayer_StartTime = std::chrono::high_resolution_clock::now();
        }

        if (d_result == true) {
            vector<vector<vector<float>>> closest_player_pos = {};
            float dist = zom.SearchClosestPlayer(closest_player_pos, 1);
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
            cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' 와 가장 가까운 플레이어 사이의 거리: " << dist << endl;
#endif

            if (zom.detectCanSeePlayer_randomChance == false) {

                auto delayAfterTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> deltaTime = delayAfterTime - zom.detectCanSeePlayerFail_StartTime;

                if (deltaTime.count() >= zom.detectCanSeePlayerFail_delayTime 
                    || dist <= 800.f) { // 탐지 확률 100% 거리일 때는 detectCanSeePlayerFail_delayTime 무시하고 바로 포착가능 하도록

                    d_result = zom.CanSeePlayerRandomChance();  // 포착 랜덤확률 부여
                    if (d_result == false) {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                        cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-CanSeePlayer(플레이어 포착) RandomChance 실패!!!!!!!" << endl;
#endif
                        zom.detectCanSeePlayerFail_StartTime = std::chrono::high_resolution_clock::now();
                    }
                    else if (d_result == true) {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                        cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-CanSeePlayer(플레이어 포착) RandomChance 성공!" << endl;
#endif
                    }
                }
                else {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                    cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-CanSeePlayer(플레이어 포착) RandomChance 실패 상태" << endl;
                    cout << "남은 시간: " << zom.detectCanSeePlayerFail_delayTime - deltaTime.count() << "s" << endl;
#endif
                    d_result = false; // 남은 딜레이 시간 동안 실패 상태 유지
                }

            }

        }
        else {
            zom.detectCanSeePlayer_randomChance = false;    // 다시 초기화
        }

#ifdef ENABLE_BT_NODE_LOG
        cout << "따라서, 좀비 \'#" << zom.ZombieData.zombieID << "\' 에 <Detect>의 (CanSeePlayer Decorator) 결과: \"" << boolalpha << d_result << "\"" << endl;
        cout << endl;
#endif

        Current_CanSeePlayer_result = d_result; // 확률 검사 까지 수행한 결과
        
        if (Prev_CanSeePlayer_result == false && Current_CanSeePlayer_result == true) {    // 플레이어를 처음 발견했거나 플레이어를 놓쳤다가 다시 발견했다면 -> 호드 사운드 재생
            zom.MakeNoise();

            // 샤우팅 좀비일 경우에는 샤우팅 실행
            if (zom.ZombieData.zombietype == 1) {
                ShoutingZombie* sz = dynamic_cast<ShoutingZombie*>(&zom);  // 다운 캐스팅 사용!
                sz->Shout(zom.ClosestPlayerID, zom.roomid); // ClosestPlayerID 지정은 위에서 미리 진행
            }
        }

        zom.CanSeePlayer_result = d_result;

        if (d_result == true) {
            zom.KnewPlayerLocation = true;

            zom.HeardFootSound = false;     // 발소리를 먼저 듣고 플레이어를 발견한 경우, 좀비가 후에 플레이어를 놓치면 이전에 들은 발소리를 따라가는 게 아닌 플레이어를 마지막으로 본 위치를 탐색하도록 하기 위해서
                                            // 문제는 없는데 이런 방식으로 작동하는게 맞는 거 같다고 생각해서 (플레이어를 눈으로 보면 이전 기억하던 발소리는 까먹는다!)
                                            // 노노 이거 안 했으면 문제 있었음;; => 호드 사운드는 까먹고 안 했더니 플레이어 쫒아오다가 잠깐 놓치니까 저 뒤로 이전에 들은 호드 사운드 쪽으로 가버림;;
            
            zom.HeardHordeSound = false;    // 호드 사운드도 잊기
            
            zom.HeardShouting = false;      // 샤우팅도

            CanSeePlayer(zom);
        }

        return d_result;
    }

    bool CanSeePlayer(Zombie& zom) override {
#ifdef ENABLE_BT_NODE_LOG
        cout << "Selector <CanSeePlayer> 호출" << endl;
        cout << endl;
#endif

        d_result = false;
        for (const auto& child : sel_children) {
            bool task_result = child->CanSeePlayer(zom);    // 다형성 이용 (함수 오버라이딩)

            if (d_result == false)
                d_result = task_result; // 어느 하나라도 결과값이 true이면 true

            if (d_result == true)   // <= 나중에 병렬로 돌리려면 지워야함
                break;
        }

        if (d_result == false) {  // selector의 모든 task가 실패 할 경우(그럴 일은 없어야 하지만..)
            cout << "\"Selector CanSeePlayer [ERROR]!!!\" - ZombieID #" << zom.ZombieData.zombieID << endl;
            cout << endl;
        }

        return d_result;
    }

};