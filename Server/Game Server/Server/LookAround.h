#pragma once

#include "Task.h"


class T_LookAround : public Task {
public:

    bool Detect(Zombie& zom) override {
#ifdef ENABLE_BT_NODE_LOG
        cout << "<Detect>�� (LookAround Decorator) ȣ��" << endl;
#endif

        if (zom.IsLookingAround == true) {
            d_result = true;

            zom.targetType = zom.LOOKINGAROUND;
        }
        else if (zom.IsLookingAround == false && zom.lookAroundCount >= 3) {	// ���� �ѷ����� ����
            zom.ReachFinalDestination();    // �÷��� �ʱ�ȭ�� ����
            
            d_result = false;
        }
        else {
            d_result = false;
        }
      
#ifdef ENABLE_BT_NODE_LOG
        cout << "����, ���� \'#" << zom.ZombieData.zombieID << "\' �� <Detect>�� (LookAround Decorator) ���: \"" << boolalpha << d_result << "\"" << endl;
        cout << endl;
#endif

        if (d_result == true)
            LookAround(zom);

        return d_result;
    }

    bool LookAround(Zombie& zom) {
#ifdef ENABLE_BT_NODE_LOG
        cout << "Task [LookAround] ȣ��" << endl;
        cout << endl;
#endif

        zom.LookAround();

        d_result = true;
        return d_result;
    }


};