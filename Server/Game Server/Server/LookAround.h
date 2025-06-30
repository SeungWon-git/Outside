#pragma once

#include "Task.h"


class T_LookAround : public Task {
public:

    bool Detect(Zombie& zom) override {
#ifdef ENABLE_BT_NODE_LOG
        cout << "<Detect>의 (LookAround Decorator) 호출" << endl;
#endif

        if (zom.IsLookingAround == true) {
            d_result = true;

            zom.targetType = zom.LOOKINGAROUND;
        }
        else if (zom.IsLookingAround == false && zom.lookAroundCount >= 3) {	// 주위 둘러보기 끝남
            zom.ReachFinalDestination();    // 플래그 초기화를 위해
            
            d_result = false;
        }
        else {
            d_result = false;
        }
      
#ifdef ENABLE_BT_NODE_LOG
        cout << "따라서, 좀비 \'#" << zom.ZombieData.zombieID << "\' 에 <Detect>의 (LookAround Decorator) 결과: \"" << boolalpha << d_result << "\"" << endl;
        cout << endl;
#endif

        if (d_result == true)
            LookAround(zom);

        return d_result;
    }

    bool LookAround(Zombie& zom) {
#ifdef ENABLE_BT_NODE_LOG
        cout << "Task [LookAround] 호출" << endl;
        cout << endl;
#endif

        zom.LookAround();

        d_result = true;
        return d_result;
    }


};