#pragma once
#include "Common.h"
#include "iocpServerClass.h"
#include "algorithm"
#include "ZombieController.h"
#include "ItemController.h"
#include "Zombie.h"
//#include "ShoutingZombie.h"

std::mutex roomPlayersMutex;


bool IOCP_CORE::IOCP_ProcessPacket(int id, const std::string& packet) {
    // g_players에서 클라이언트 정보 검색
    auto it = g_players.find(id);
    if (it == g_players.end()) {
        // 유효하지 않은 클라이언트 ID에 대한 처리
        return false;
    }

    PLAYER_INFO* clientInfo = it->second;

    Protocol::Character tempPacket;
    tempPacket.ParseFromArray(packet.data(), packet.size());

    if ((tempPacket.playerid() == id || tempPacket.isingame()) && !clientInfo->isInGame) {
        clientInfo->isInGame = true;
        b_Timer = true;
        //printf("SendDatas!! Playerid=#%d\n", id);
    }

    if (clientInfo->roomid != 0 && !zombieDB[clientInfo->roomid].empty()) { // ==> 항상 두번씩 보냄 (이유랑 왜 어떻게 그런지는 사실 잘 모르겠음; 한번으로는 잘 못 받아서 그랬던가?)
        if (!clientInfo->send_zombie) {
            zombieControllers[clientInfo->roomid]->SendZombieData(clientInfo->id);
        }

        if (clientInfo->send_zombie && !clientInfo->send_item) {
            itemControllers[clientInfo->roomid]->SendItemData(clientInfo->id);
        }

        if (clientInfo->send_zombie && clientInfo->send_item && !clientInfo->send_car) {
            itemControllers[clientInfo->roomid]->SendCarData(clientInfo->id);
        }
    }

    // 패킷의 타입을 확인하여 처리
    switch (tempPacket.packet_type()) {
    case 1: {
        //printf("\n[ No. %3u ] character Packet Received !!\n", id);
        //printf("Received packet type = %d\n", CharacterPacket.type());
        //printf("Received playerID = %d\n", CharacterPacket.playerid());
        //printf("Received packet x = %f, y = %f, z = %f\n\n", CharacterPacket.x(), CharacterPacket.y(), CharacterPacket.z());
        //printf("Received packet x = %f, y = %f, z = %f\n\n", CharacterPacket.pitch(), CharacterPacket.yaw(), CharacterPacket.roll());

        // 서버로 받은 패킷을 그대로 돌려줌
        Protocol::Character Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        // 게임룸에 플레이어 추가
        if (clientInfo->roomid == 0) {
            //out << "g_players (룸 아이디는?): " << g_players[id]->roomid << endl;
            clientInfo->roomid = Packet.roomnum();
            //cout << "g_players (룸 아이디는?): " << g_players[id]->roomid << endl;
            AddPlayerToRoom(Packet.roomnum(), clientInfo);
            cout << "Player " << clientInfo->id << " 의 게임 ID: " << Packet.playerid() << endl;
        }


        // 지금은 수정 됐지만 혹시해서 남김 -> 클라 플레이어 초기화 id 설정값이 99인데 이걸 전송 받는 경우가 생겼었다
        if (Packet.playerid() != 99) {
            //playerDB[Packet.playerid()] = pl; //-> 이렇게 사용하면 초기화 빼먹은 값 더미 값 씌워질 수 있음
            //* playerDB 새로 생성
            playerDB[Packet.roomnum()][Packet.playerid()].username = Packet.username();

            playerDB[Packet.roomnum()][Packet.playerid()].x = Packet.x();
            playerDB[Packet.roomnum()][Packet.playerid()].y = Packet.y();
            playerDB[Packet.roomnum()][Packet.playerid()].z = Packet.z();

            playerDB[Packet.roomnum()][Packet.playerid()].health = Packet.hp();

            //cout << "player hp: " << Packet.hp() << endl;

            if (Packet.b_run() == 1) {
                playerDB[Packet.roomnum()][Packet.playerid()].IsRunning = false;
                //printf("\n[ No. %3u ] Walk Packet Received !!\n", id);
            }
            else if (Packet.b_run() == 2) {
                playerDB[Packet.roomnum()][Packet.playerid()].IsRunning = true;
                //printf("\n[ No. %3u ] Run Packet Received !!\n", id);
            }

            //cout << "player IsRunning: " << Packet.b_run() << endl;

            if (Packet.z() < 800.f) {
                playerDB[Packet.roomnum()][Packet.playerid()].floor = FLOOR::FLOOR_B2;
            }
            else if (Packet.z() < 1800.f) {
                playerDB[Packet.roomnum()][Packet.playerid()].floor = FLOOR::FLOOR_B1;
            }
            else if (Packet.z() < 2500.f) {
                playerDB[Packet.roomnum()][Packet.playerid()].floor = FLOOR::FLOOR_F1;
            }
            else if (Packet.z() < 3600.f) {
                playerDB[Packet.roomnum()][Packet.playerid()].floor = FLOOR::FLOOR_F2;
            }
            else {
                playerDB[Packet.roomnum()][Packet.playerid()].floor = FLOOR::FLOOR_F3;
            }
        }


        bool bAllPlayersInRange = true;
        int alive_cnt = 0;
        int dead_cnt = 0;
        int disconnected = 0;
        int bestkill_cnt = 0;
        std::string bestkill_player = "None";   // 모두 0킬이면 None 띄움

        // 탈출 체크
        if (room_states[Packet.roomnum()].b_IsEscaping) { // 탈출존이 열린 상태
            int roomId = clientInfo->roomid;
            for (const auto& player : playerDB[roomId]) {
                if (room_players[roomId].find(player.first) == room_players[roomId].end()) { // 연결이 끊긴 플레이어라면  
                    disconnected++;
                    if (bestkill_cnt < player.second.killcount) {   // 연결이 끊겼어도 젤 마니 좀비를 죽였을 수도 있으니
                        bestkill_cnt = player.second.killcount;
                        bestkill_player = player.second.username;
                    }
                    continue;
                }

                if (player.second.health > 0) {
                    alive_cnt++;
                    float DeltaX = std::abs(player.second.x - room_states[roomId].Escape_Location.x);
                    float DeltaY = std::abs(player.second.y - room_states[roomId].Escape_Location.y);

                    // 탈출존 범위를 벗어나는지 확인
                    if (DeltaX >= 150.0f || DeltaY >= 150.0f || player.second.floor != room_states[roomId].Escape_Location.floor) {
                        bAllPlayersInRange = false; // 한 명이라도 범위를 벗어나면 false
                        break;
                    }
                }
                else {
                    dead_cnt++;
                }

                if (bestkill_cnt < player.second.killcount) {
                    bestkill_cnt = player.second.killcount;
                    bestkill_player = player.second.username;
                }
            }

            if (bAllPlayersInRange) {
                Send_GameEnd(alive_cnt, dead_cnt, bestkill_cnt, bestkill_player, clientInfo->roomid);

                return true;
            }
        }


        // 접속해있는 플레이어들이 모두 사망했는지 검사
        int totalplayer_cnt = room_players[Packet.roomnum()].size();
        int deadplayer_cnt = 0;

        for (const auto player : room_players[Packet.roomnum()]) {   // 연결이 되어 있는 플레이어들 중
            if (playerDB[Packet.roomnum()].find(player.first) != playerDB[Packet.roomnum()].end()) {    // abort 에러 크래쉬 방지
                if (playerDB[Packet.roomnum()].at(player.first).health <= 0) {        // 체력확인
                    deadplayer_cnt++;
                }
            }
        }

        // 모든 플레이어들이 사망 게임오버 엔딩 점수판 띄우게하기
        if (totalplayer_cnt == deadplayer_cnt && playerDB_BT[Packet.roomnum()].size() != 0) {
            // 점수판 계산
            alive_cnt = 0;
            dead_cnt = 0;
            disconnected = 0;
            bestkill_cnt = 0;
            bestkill_player = "None";

            for (const auto player : playerDB[Packet.roomnum()]) {
                if (room_players[Packet.roomnum()].find(player.first) == room_players[Packet.roomnum()].end()) { // 연결이 끊긴 플레이어라면  
                    disconnected++;
                    if (bestkill_cnt < player.second.killcount) {   // 연결이 끊겼어도 젤 마니 좀비를 죽였을 수도 있으니
                        bestkill_cnt = player.second.killcount;
                        bestkill_player = player.second.username;
                    }
                    continue;
                }

                if (player.second.health > 0) {
                    alive_cnt++;
                }
                else {
                    dead_cnt++;
                }

                if (bestkill_cnt < player.second.killcount) {
                    bestkill_cnt = player.second.killcount;
                    bestkill_player = player.second.username;
                }
            }

            room_states[Packet.roomnum()].Escape_Root = 0;    // 탈출방법 0(실패)으로 초기화 => 이전에 문을 연적이 있으면 해당 변수 갱신되서, 게임오버에서 탈출방법 실패가 안뜸;;
            //Root_Open_Player = "None";

            // 전송작업
            Send_GameEnd(alive_cnt, dead_cnt, bestkill_cnt, bestkill_player, clientInfo->roomid);

            return true;
        }


        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        return true;
    } break;

    case 2: {
        // 예전에 클라에서 좀비 움직이면 해당 패킷을 서버로 보냈음 -> 이제는 움직임을 서버에서 담당하니 사실상 사용 안 함

#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] zombie Packet Received !!\n", id);
#endif
        Protocol::Zombie Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        /*ZombieData zombiedata;
        zombiedata.zombieID = Packet.zombieid();
        zombiedata.x = Packet.x();
        zombiedata.y = Packet.y();
        zombiedata.z = Packet.z();
        zombiedata.pitch = Packet.pitch();
        zombiedata.yaw = Packet.yaw();
        zombiedata.roll = Packet.roll();*/

        //ZombieController zombiecontroller;

        //printf("zombie id: %d \n", Packet.zombieid());

        /*if (find(m_zombie.begin(), m_zombie.end(), zombiedata.zombieID) == m_zombie.end()) {
            zombiecontroller.addZombie(zombiedata);
            m_zombie.push_back(zombiedata.zombieID);
        }
        else {
            zombiecontroller.setZombiePosition(zombiedata);
        }*/

        /*if (id == min_it->first) {
            for (const auto& player : g_players) {
                if (player.first != id && player.second->isInGame && player.second->roomid == clientInfo->roomid) {
                    IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
                }
            }
        }*/
        return true;
    } break;

    case 4:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] character attack Packet Received !!\n", id);
#endif
        Protocol::Character_Attack Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

#ifdef ENABLE_PACKET_LOG
        cout << "에임 오프셋: " << Packet.aimoffset() << endl;
#endif

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }
        return true;
    }

    case 5:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] Equip Packet Received !!\n", id);
#endif
        Protocol::Equip_Item Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }
        return true;
    }

    //case 6:
    //{
    //    printf("\n[ No. %3u ] Run Packet Received !!\n", id);
    //    Protocol::run Packet;
    //    Packet.ParseFromArray(packet.data(), packet.size());
    //    string serializedData;
    //    Packet.SerializeToString(&serializedData);
    //    
    //    // 해당 플레이어 run-bool값 변경
    //    for (auto& player : playerDB) {
    //        if (player.first == Packet.playerid()) {
    //            player.second.IsRunning = Packet.b_run();
    //        }
    //    }
    //    // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
    //    for (const auto& player : g_players) {
    //        if (player.first != id && player.second->isInGame) {
    //            IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
    //        }
    //    }
    //    return true;
    //}

    case 7:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] Jump Packet Received !!\n", id);
#endif
        Protocol::jump Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        // ==== 해당 변수들이 어떻게 작동하는지 디버깅용
        /*cout << "<<<<< roomId: " << roomId << " >>>>>" << endl;

        cout << "--- playerDB[roomId]" << endl;
        for (const auto player : playerDB[roomId]) {
            cout << "playerDB.first: " << player.first << endl;
            cout << "playerDB.second.username: " << player.second.username << endl;
        }

        cout << "--- g_players" << endl;
        for (const auto player : g_players) {
            cout << "g_player.first: " << player.first << endl;
            cout << "g_player.second->id: " << player.second->id << endl;
            cout << "g_player.second->roomid: " << player.second->roomid << endl;
            cout << "g_player.second->connected: " << player.second->connected << endl;
        }

        cout << "--- room_players[roomId]" << endl;
        for (const auto player : room_players[roomId]) {
            cout << "room_player.first: " << player.first << endl;
            cout << "room_player.second->id: " << player.second->id << endl;
            cout << "room_player.second->roomid: " << player.second->roomid << endl;
            cout << "room_player.second->connected: " << player.second->connected << endl;
        }*/

        return true;
    }

    case 9:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> [ No. %3u ] Detected Packet Received !!\n", id);
#endif
        Protocol::Detected Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        int recvzombieid = Packet.zombieid();
        int roomId = clientInfo->roomid;

        if (Packet.player_insight()) {
            for (auto& z : zombieDB[roomId]) {
                if (z->ZombieData.zombieID == recvzombieid) {
                    if (z->IsAttacking)
                        break;

                    //z->PlayerInSight = true;
                    //z->KnewPlayerLocation = true;
                    float dist = z->SetDistance(Packet.playerid(), 1);   // DistanceTo_PlayerInsight 맵 에 해당 플레이어와 거리 데이터 새로 추가하기 (생성) / 이미 있었다면 갱신하기

#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_NODE_LOG)
                    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
                    cout << "좀비 \'#" << z->ZombieData.zombieID << "\' 의 시야에 - 플레이어 \'#" << id << "\' 포착!!! - 거리: " << dist << endl;
#endif

                    // 샤우팅 좀비일 경우에
                    //if (z->ZombieData.zombietype == 1) {
                    //    ShoutingZombie* sz = dynamic_cast<ShoutingZombie*>(z);  // 다운 캐스팅 사용!
                    //    sz->Shout(id, roomId);
                    //}
                    // 시야 포착 확률 넣어서 이제는 이렇게 작동하면 X

                    break;
                }
            }
        }

        else {
            for (auto& z : zombieDB[roomId]) {
                if (z->ZombieData.zombieID == recvzombieid) {
                    if (z->IsAttacking)
                        break;

                    //z->PlayerInSight = false;
                    z->DistanceTo_PlayerInsight[Packet.playerid()] = z->CanSeePlayerDistance + 6969.f;     // 탐지 거리 밖 표시 => 데이터 레이스 발생시키는데 문제는 발생 안시킴

#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_NODE_LOG)
                    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
                    cout << "좀비 \'#" << z->ZombieData.zombieID << "\' 의 시야에서 - 플레이어 \'#" << id << "\' 놓침!!!" << endl;
#endif

                    break;
                }
            }
        }

        return true;
    }

    case 11:
    {
        Protocol::ping Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        auto it = g_players.find(Packet.playerid());

        if (it != g_players.end() && it->second != nullptr)
        {
            PLAYER_INFO* player = it->second;
            player->pingcnt = 0; // 클라이언트가 응답했으므로 pingcnt 초기화
        }

        return true;
    }

    case 12:
    {
        Protocol::Zombie_hp Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_FLEE_LOG)
        cout << "recv zombie #" << Packet.zombieid() << " HP Packet" << endl;
#endif
        string serializedData;
        Packet.SerializeToString(&serializedData);

        int roomId = clientInfo->roomid;

        // 바로 다른 클라들에게도 좀비 HP 동기화 작업 실행
        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        // 좀비 HP 업데이트
        int recvzombieid = Packet.zombieid();

        for (auto& z : zombieDB[roomId]) {
            if (z->ZombieData.zombieID == recvzombieid) {
                float newZombieHP = max(0, z->zombieHP - Packet.damage());  // 0이하로 안 떨어짐

                // 좀비 사망시에
                if (newZombieHP <= 0) {
                    playerDB[roomId][id].killcount++;

                    z->resurrectionStartTime = std::chrono::high_resolution_clock::now();		// 좀비 부활 타이머 시작 시간

                    z->zombieHP = newZombieHP;  // 혹시 모를 데이터 레이스 방지 (resurrectionStartTime 세팅 전에 hp를 업데이트해서(먼저 0으로 바꿔서, iocp에서 부활처리하면) 좀비가 죽고 바로 부활하는 버그 생기는 거 같음)

#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_LOG) || defined(ENABLE_BT_FLEE_LOG)
                    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
                    cout << "좀비 \'#" << z->ZombieData.zombieID << "\' 사망!!! - (Normal Dead)" << endl;
                    cout << endl;
#endif
                }
                else if (newZombieHP < z->ZombieStartHP || Packet.damage() == 0) { // * 이거 혹시 클라에서 좀비 최초로 때릴때 데미지 0으로 때리면 그럼 어떻게 되는 거지? (서버는 BT 그냥 돌아가나?) =>  Packet.damage() == 0 일때도 그냥 피격으로 만듬
                    z->IsBeingAttacked = true;  // 좀비 피격중으로 변경
                    z->HaveToWait = true;	// 좀비 BT 대기상태로 변경
                    z->waitAnimStartTime = std::chrono::high_resolution_clock::now();		// 좀비 피격 시작 시간

#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_FLEE_LOG)
                    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
                    cout << "좀비 \'#" << z->ZombieData.zombieID << "\' 피격!! 남은 HP: " << z->GetHP() << endl;
                    cout << endl;
#endif

                    z->zombieHP = newZombieHP;  // 혹시 모를 데이터 레이스 방지

                    z->FleeRandChance();   // 도망가기 랜덤 가챠 돌리기
                }

                break;
            }
        }

        return true;
    }

    case 14:
    {
        Protocol::patrol_hit Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        //cout << "recv patrol zombie hit #" << Packet.zombieid() << endl;

        return true;
    }

    case 17:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] Destroy ItemBox Packet Received !!\n", id);
#endif
        Protocol::destroy_item Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        //printf("Get itemId %d\n", Packet.itemid());

        int roomId = clientInfo->roomid;

        // 아이템 중복 획득 방지 코드
        if (itemControllers.find(roomId) != itemControllers.end()) {
            for (auto& item : itemControllers[roomId]->items) {

                if (item.itemID == Packet.itemid() // 해당 아이템 찾기
                    && item.owner_playerid == 0) {  // 해당 아이템를 이미 보유한 플레이어가 있는지 확인 (0=보유한 플레이어 없음)
                    item.owner_playerid = id;   // 없다면 해당 플레이어 아이디 저장

                    // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
                    if (room_players.find(roomId) != room_players.end()) {
                        for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                            if (playerInfo != nullptr) {    // 자기 자신한테도 다시 패킷 전송해야함 => 패킷을 받아야 진짜 해당 아이템을 획득하도록 바꿨으니
                                IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                            }
                        }
                    }

                }

            }
        }

        return true;
    }

    case 18:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] get key item !!\n", id);
#endif
        Protocol::get_key Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        int itemboxid = Packet.itemboxid();
        int playerid = Packet.playerid();

#ifdef ENABLE_PACKET_LOG
        printf("%d item - (1: Car Key, 2: Roof Key)\n", Packet.itemid());
#endif

        //printf("Get key itemId %d\n", Packet.itemboxid());

        string serializedData;
        Packet.SerializeToString(&serializedData);

        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        //key 먹었을 경우에는 아이템박스 삭제 패킷을 여기서 따로 보내줌

        Protocol::destroy_item destroyPacket;

        destroyPacket.set_packet_type(17);
        destroyPacket.set_itemid(itemboxid);
        destroyPacket.set_playerid(playerid);

        string DserializedData;
        destroyPacket.SerializeToString(&DserializedData);

        // 키 아이템 중복 획득 방지 코드
        if (itemControllers.find(roomId) != itemControllers.end()) {
            for (auto& item : itemControllers[roomId]->items) {

                if (item.itemID == itemboxid // 해당 아이템 찾기
                    && item.owner_playerid == 0) {  // 해당 아이템를 이미 보유한 플레이어가 있는지 확인 (0=보유한 플레이어 없음)
                    item.owner_playerid = id;   // 없다면 해당 플레이어 아이디 저장

                    if (room_players.find(roomId) != room_players.end()) {
                        for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                            if (playerInfo != nullptr) {    // 자기 자신한테도 다시 패킷 전송해야함 => 패킷을 받아야 진짜 해당 아이템을 획득하도록 바꿨으니
                                IOCP_SendPacket(playerId, DserializedData.data(), DserializedData.size());
                            }
                        }
                    }

                }

            }
        }

        return true;
    }

    case 19:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] escape root open !!\n", id);
#endif
        Protocol::escape Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        int roomId = clientInfo->roomid;

        if (Packet.root() == 2) {
            room_states[roomId].roofkey_cnt++;
        }

        if (room_states[roomId].roofkey_cnt == 2 || Packet.root() == 1) {
            // 문 연 위치 저장, 상태를 탈출 준비 상태로 변경
            room_states[roomId].Escape_Location.x = playerDB[roomId][id].x;
            room_states[roomId].Escape_Location.y = playerDB[roomId][id].y;
            room_states[roomId].Escape_Location.floor = playerDB[roomId][id].floor;

            room_states[roomId].b_IsEscaping = true;
            room_states[roomId].Escape_Root = Packet.root();
            room_states[roomId].Root_Open_Player = playerDB[roomId][id].username;
        }


        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }
        return true;
    }

    case 21:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] recv send complete packet !!\n", id);
#endif
        Protocol::send_complete Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        if (Packet.complete_type() == 1) {
            clientInfo->send_zombie = true;
            //printf("Recv-send complete Zombie spawn packets\n");
        }
        else if (Packet.complete_type() == 2) {
            clientInfo->send_item = true;
            //printf("Recv-send complete Item spawn packets\n");
        }
        else if (Packet.complete_type() == 3) {
            clientInfo->send_car = true;
            //printf("Recv-send complete Car spawn packets\n");
        }

        return true;
    }

    case 22:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] recv item drop packet !!\n", id);
#endif
        Protocol::drop_item Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        string serializedData;
        Packet.SerializeToString(&serializedData);

        //printf("Drop itemId %d\n", Packet.itemid());

        int roomId = clientInfo->roomid;

        if (itemControllers[roomId]->items.size() == 60 && Packet.itemid() > 60 && Packet.itemname() == "SuitCase") {  // 회사원 시작 아이템인 서류가방을 떨구면 아이템 벡터에 추가 => 추후에 다시 줍기가 가능하게
            Item_Data employeeSuitCase;

            employeeSuitCase.itemID = Packet.itemid();
            employeeSuitCase.itemName = Packet.itemname();
            employeeSuitCase.itemClass = Packet.itemclass();
            employeeSuitCase.texturePath = Packet.texture_path();
            employeeSuitCase.count = Packet.count();
            //employeeSuitCase.itemFloor = ;
            employeeSuitCase.x = Packet.posx();
            employeeSuitCase.y = Packet.posy();
            employeeSuitCase.z = Packet.posz();

            itemControllers[roomId]->items.push_back(employeeSuitCase);
        }

        if (itemControllers.find(roomId) != itemControllers.end()) {
            for (auto& item : itemControllers[roomId]->items) {
                if (item.itemID == Packet.itemid()) { // 해당 아이템 찾기
                    item.owner_playerid = 0;    // 바닥에 다시 떨구며 아이템 소유자 초기화
                }
            }
        }

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        return true;

    }

    case 23:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] recv item detach packet !!\n", id);
#endif
        Protocol::detach_item Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        string serializedData;
        Packet.SerializeToString(&serializedData);

        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        return true;
    }

    case 24:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] slice vector packet !!\n", id);
#endif
        Protocol::slice_vector Packet;
        Packet.ParseFromArray(packet.data(), packet.size());

        string serializedData;
        Packet.SerializeToString(&serializedData);

        int roomId = clientInfo->roomid;

        // 좀비 HP 업데이트
        int recvzombieid = Packet.zombieid();

        for (auto& z : zombieDB[roomId]) {
            if (z->ZombieData.zombieID == recvzombieid) {
                z->zombieHP = 0;
                playerDB[roomId][id].killcount++;

                z->resurrectionStartTime = std::chrono::high_resolution_clock::now();		// 좀비 부활 타이머 시작 시간

#if defined(ENABLE_PACKET_LOG) || defined(ENABLE_BT_LOG) || defined(ENABLE_BT_FLEE_LOG)
                cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
                cout << "좀비 \'#" << z->ZombieData.zombieID << "\' 사망!!! - (Cut Dead)" << endl;
                cout << endl;
#endif
            }
        }

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr && playerId != id) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }

        return true;
    }

    case 26:
    {
#ifdef ENABLE_PACKET_LOG
        printf("\n[ No. %3u ] recv chatting packet !!\n", id);
#endif
        Protocol::chatting Packet;
        Packet.ParseFromArray(packet.data(), packet.size());
        string serializedData;
        Packet.SerializeToString(&serializedData);

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        int roomId = clientInfo->roomid;

        if (room_players.find(roomId) != room_players.end()) {
            for (const auto& [playerId, playerInfo] : room_players[roomId]) {
                if (playerInfo != nullptr) {
                    IOCP_SendPacket(playerId, serializedData.data(), serializedData.size());
                }
            }
        }
        return true;

    }

    default: {
        printf("\nERROR, Unknown signal -> [ %u ] protocol num = %d\n", id, tempPacket.packet_type());
        // 클라이언트나 서버 종료, 로깅 등의 처리 가능
        return true;
    } break;
    }
}

void IOCP_CORE::Send_GameEnd(int alive_cnt, int dead_cnt, int bestkill_cnt, std::string bestkill_player, int roomid)
{
    Protocol::game_clear clear_packet;

    clear_packet.set_packet_type(20);
    clear_packet.set_root(room_states[roomid].Escape_Root);
    clear_packet.set_alive_players(alive_cnt);
    clear_packet.set_dead_players(dead_cnt);
    clear_packet.set_open_player(room_states[roomid].Root_Open_Player);

    clear_packet.set_best_killcount(bestkill_cnt);
    clear_packet.set_best_kill_player(bestkill_player);

    for (const auto& player : room_players[roomid]) {

        clear_packet.set_my_killcount(playerDB[roomid][player.first].killcount);

        string serializedData;
        clear_packet.SerializeToString(&serializedData);
        IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
    }
}

void IOCP_CORE::AddPlayerToRoom(int roomId, PLAYER_INFO* clientInfo) {
    std::lock_guard<std::mutex> lock(roomPlayersMutex);

    // 해당 roomId가 존재하지 않으면 방 생성
    if (room_players.find(roomId) == room_players.end()) {
        //* room_players 새로 생성
        room_players[roomId] = std::unordered_map<int, PLAYER_INFO*>();
        ZombieBT* tmp_zbt = new ZombieBT();
        zombie_bt_map[roomId] = *tmp_zbt;
        zombie_threads.emplace_back(&IOCP_CORE::Zombie_BT_Thread, this, roomId);
        zombieControllers[roomId] = new ZombieController(this, roomId);
        itemControllers[roomId] = new ItemController(this, roomId);
        std::cout << "Room " << roomId << " created." << std::endl;
    }
    
    room_players[roomId][clientInfo->id] = clientInfo;
    std::cout << "Player " << clientInfo->id << " added to room " << roomId << std::endl;
}
