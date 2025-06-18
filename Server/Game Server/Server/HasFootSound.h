#pragma once

#include "Sequence.h"


class Seq_HasFootSound : public Sequence {
public:
    Seq_HasFootSound() { t_name = "HasFootSound"; }


    bool Detect(Zombie& zom) override {
#ifdef ENABLE_BT_NODE_LOG
        cout << "<Detect>의 (HasFootSound Decorator) 호출" << endl;
#endif

        d_result = zom.FootSound_Update_Check();

#ifdef ENABLE_BT_NODE_LOG
        cout << "좀비 새로운 플레이어 발소리 정보를 포착했는가?: " << boolalpha << d_result << endl;
#endif

        if (d_result == true) {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
            vector<vector<vector<float>>> closest_player_pos = {};
            float dist = zom.SearchClosestPlayer(closest_player_pos, 2);
            cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' 와 가장 가까운 발소리 사이의 거리: " << dist << endl;
#endif

            if (zom.detectHasFootSound_randomChance == false) {

                auto delayAfterTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> deltaTime = delayAfterTime - zom.detectHasFootSoundFail_StartTime;

                if (deltaTime.count() >= zom.detectHasFootSoundFail_delayTime) {

                    d_result = zom.HasFootSoundRandomChance();  // 포착 랜덤확률 부여
                    if (d_result == false) {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                        cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-HasFootSoundPlayer(발소리 포착) RandomChance 실패!!!" << endl;
#endif
                        zom.detectHasFootSoundFail_StartTime = std::chrono::high_resolution_clock::now();
                    }
                    else if (d_result == true) {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                        cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-HasFootSoundPlayer(발소리 포착) RandomChance 성공!!!" << endl;
#endif
                    }

                }
                else {
#ifdef ENABLE_BT_DETECT_RANDOMCHANCE_LOG
                    cout << "좀비 \'#" << zom.ZombieData.zombieID << "\' Detect-HasFootSound(발소리 포착) RandomChance 실패 상태" << endl;
                    cout << "남은 시간: " << zom.detectHasFootSoundFail_delayTime - deltaTime.count() << "s" << endl;
#endif
                    d_result = false; // 남은 딜레이 시간 동안 실패 상태 유지
                }
            }

        }
        else {
            zom.detectHasFootSound_randomChance = false;    // 다시 초기화
        }

        if (d_result == true) { // 새로운 발소리가 청각범위내에 있었고 랜덤 확률도 뚫고 들었다면
            zom.HeardFootSound = true;
            zom.SetTargetLocation(Zombie::TARGET::FOOTSOUND);   // 가장 가까운 플레이어의 발소리를 목표지점으로 (TargetLocation 설정)
        }

        if (zom.HeardFootSound == true) { // FootSound_Update_Check 결과 상관없이 이미 근처 발소리를 들은 경우에는(이전에 들은 발소리를 아직 기억하는 경우) -> 계속 실행
#ifdef ENABLE_BT_NODE_LOG
            if (d_result == false)
                cout << "좀비 플레이어 발소리 정보를 이미 가지고 있는가?: true" << endl;
#endif
            // zom.SetTargetLocation(Zombie::TARGET::FOOTSOUND);   // 가장 가까운 플레이어의 발소리를 목표지점으로 (TargetLocation 설정) 
            // 생각 해보니 굳이 안해줘도 됨! -> 이미 HeardFootSound = true라면 TargetLocation, targetType 이 이전에 지정되어 있는 상태이고 
            // FootSound_Update_Check 결과가 true였다면 위에 d_result == true에서 SetTargetLocation 작업 해주고 있고, 
            // 결과가 false였다면 새로운 발소리가 포착 안된거고 그럼 SetTargetLocation에서 SearchClosestPlayer도 작동 안할 꺼니까 ===> 그냥 어느 상황이든 할 필요가 없음

            d_result = true;
        }

#ifdef ENABLE_BT_NODE_LOG
        cout << "따라서, 좀비 \'#" << zom.ZombieData.zombieID << "\' 에 <Detect>의 (HasFootSound Decorator) 결과: \"" << boolalpha << d_result << "\"" << endl;
        cout << endl;
#endif


        if (d_result == true)
            HasFootSound(zom);

        return d_result;
    }

    bool HasFootSound(Zombie& zom) {
#ifdef ENABLE_BT_NODE_LOG
        cout << "Sequence {HasFootSound} 호출" << endl;
        cout << endl;
#endif

        for (const auto& child : seq_children) {
            d_result = child->HasFootSound(zom);

            if (d_result == false) {
                cout << "\"Sequence HasFootSound [ERROR]!!!\" - ZombieID #" << zom.ZombieData.zombieID << endl;
                cout << endl;

                return d_result;
            }
        }

        return d_result;
    }

};