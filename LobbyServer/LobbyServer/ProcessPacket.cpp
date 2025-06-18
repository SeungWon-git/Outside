#pragma once
#include "Common.h"
#include "algorithm"

bool IOCP_CORE::IOCP_ProcessPacket(int id, Packet* buffer, int bufferSize) {
    // g_players에서 클라이언트 정보 검색
    auto it = g_players.find(id);
    if (it == g_players.end()) {
        // 유효하지 않은 클라이언트 ID에 대한 처리
        return false;
    }

    auto min_it = std::min_element(g_players.begin(), g_players.end(),
        [](const std::pair<unsigned int, PLAYER_INFO*>& a, const std::pair<unsigned int, PLAYER_INFO*>& b) {
            return a.first < b.first;
        });

    PLAYER_INFO* clientInfo = it->second;

    Protocol::CS_Login tempPacket;
    tempPacket.ParseFromArray(buffer, bufferSize);

    // 패킷의 타입을 확인하여 처리
    switch (tempPacket.type()) {
    case 1: {
        printf("[ No. %3u ] Login Packet Received !!\n", id);

        Protocol::CS_Login CS_Packet;

        CS_Packet.ParseFromArray(buffer, bufferSize);

        Protocol::SC_Login SC_Packet;

        bool b_login = loginmanager.Login(CS_Packet.id(), CS_Packet.password());

        SC_Packet.set_type(3);
        SC_Packet.set_b_login(b_login);
        SC_Packet.set_id(CS_Packet.id());

        if (b_login) {
            g_players[id]->username = CS_Packet.id();
        }

        string serializedData;
        SC_Packet.SerializeToString(&serializedData);

        IOCP_SendPacket(id, serializedData.data(), serializedData.size());

        return true;

    }

    case 2: {
        printf("[ No. %3u ] Register Packet Received !!\n", id);
        Protocol::CS_Register CS_Packet;
        CS_Packet.ParseFromArray(buffer, bufferSize);

        Protocol::SC_Register SC_Packet;

        SC_Packet.set_type(4);
        SC_Packet.set_b_register(loginmanager.RegisterName(CS_Packet.id(), CS_Packet.password()));

        string serializedData;
        SC_Packet.SerializeToString(&serializedData);

        IOCP_SendPacket(id, serializedData.data(), serializedData.size());

        return true;

    }

    case 5: {
        printf("[ No. %3u ] Ready Packet Received !!\n", id);

        Protocol::WaitingReady CS_Packet;
        CS_Packet.ParseFromArray(buffer, bufferSize);

        g_players[CS_Packet.playerid()]->ready = CS_Packet.ready();

        std::string serializedData;
        CS_Packet.SerializeToString(&serializedData);

        int ready_cnt = 0;
        int room_player_cnt = 0;

        for (const auto& player : g_players) {
            if (player.second->room_num == g_players[CS_Packet.playerid()]->room_num) {
                room_player_cnt++;
                IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());

                if (player.second->ready == true) {
                    ready_cnt++;
                }
            }
        }

        if (ready_cnt == room_player_cnt) {
            Protocol::WaitingAllReady SC_Packet;
            SC_Packet.set_type(14);
            SC_Packet.set_allready(true);

            std::string serializedData;
            SC_Packet.SerializeToString(&serializedData);

            int player_num_cnt = 0;
            for (const auto& player : g_players) {
                if (player.second->room_num == g_players[CS_Packet.playerid()]->room_num) {
                    IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
                    g_players[player.first]->ready = false;
                    g_players[player.first]->player_num = ++player_num_cnt;
                }
            }
        }

        return true;
    }

    case 7:
    {
        printf("[ No. %3u ] Select Packet Received !!\n", id);

        Protocol::Select_Character Select_Packet;
        Select_Packet.ParseFromArray(buffer, bufferSize);
        //cout << Select_Packet.character_type() << "(1 - Girl, 2 - Idol, 3 - FireFighter, 4 - CompanyGuy)" << endl;

        Select_Packet.set_playerid(g_players[Select_Packet.playerid()]->player_num);
        Select_Packet.set_character_type(Select_Packet.character_type());
        Select_Packet.set_type(7);

        string serializedData;
        Select_Packet.SerializeToString(&serializedData);

        int room_id = g_players[id]->room_num;

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        for (const auto& player : g_players) {
            if (player.second->room_num == room_id) {
                IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
                //printf("전송된 방번호: %d\n", player.second->room_num);
            }
        }
        return true;
    }

    case 8:
    {
        printf("[ No. %3u ] Join Packet Received !!\n", id);

        Protocol::CS_Join CS_Packet;
        CS_Packet.ParseFromArray(buffer, bufferSize);
        
        int room_id = CS_Packet.roomid();
        int player_id = CS_Packet.playerid();

        Room* room = nullptr;

        auto it = rooms.find(room_id);
        if (it != rooms.end()) {
            room = it->second;
        }

        if (room->JoinRoom()) {
            g_players[id]->room_num = room_id;

            // 새로 들어온 플레이어에게 방의 기존 플레이어 정보 전송
            for (const auto& player : g_players) {
                if (player.second->room_num == room_id && player.first != id) {
                    Protocol::SC_JoinPlayer ExistingPlayerPacket;

                    ExistingPlayerPacket.set_name(player.second->username); // 기존 플레이어 정보
                    ExistingPlayerPacket.set_playerid(player.first);
                    ExistingPlayerPacket.set_roomid(room_id);
                    ExistingPlayerPacket.set_type(10);

                    //cout << "새로 들어온 플레이어 #" << id << "에게 기존에 방 #" << room_id << "에 있던 플레이어 #" << player.first << "(" << player.second->username << ") 전송!" << endl;

                    string existingSerializedData;
                    ExistingPlayerPacket.SerializeToString(&existingSerializedData);

                    // 새로 들어온 플레이어에게 기존 플레이어 정보 전송
                    IOCP_SendPacket(id, existingSerializedData.data(), existingSerializedData.size());
                }
            }

            Protocol::SC_JoinPlayer SC_Packet;
            SC_Packet.set_name(g_players[id]->username); // 새로 들어온 플레이어 정보
            SC_Packet.set_playerid(id);
            SC_Packet.set_roomid(room_id);
            SC_Packet.set_type(10);
            string serializedData;
            SC_Packet.SerializeToString(&serializedData);

            for (const auto& player : g_players) {
                if (player.second->room_num == room_id) {

                    //cout << "방 #" << room_id << "에 기존 플레이어 #" << player.first << "에게 새로운 플레이어 #" << id << "(" << g_players[id]->username << ") 전송!" << endl;

                    // 새 플레이어 정보 브로드캐스트
                    IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
                }
            }
            return true;
        }

        else {
            Protocol::SC_Join SC_Packet;

            SC_Packet.set_b_join(false);
            SC_Packet.set_type(9);
            string serializedData;
            SC_Packet.SerializeToString(&serializedData);
        }

        printf("Player %d : join room %d", id, CS_Packet.roomid());

        return true;
    }

    case 11:
    {
        printf("[ No. %3u ] Leave Packet Received !!\n", id);

        Protocol::CS_Leave CS_Packet;
        CS_Packet.ParseFromArray(buffer, bufferSize);

        int player_id = CS_Packet.playerid();
        int room_id = g_players[player_id]->room_num;

        auto it = rooms.find(room_id);
        Room* room = it->second;

        room->LeaveRoom();

        Protocol::SC_LeavePlayer SC_Packet;
        SC_Packet.set_username(g_players[id]->username);
        SC_Packet.set_playerid(player_id);
        SC_Packet.set_type(12);
        string serializedData;
        SC_Packet.SerializeToString(&serializedData);

        for (const auto& player : g_players) {
            if (player.second->room_num == room_id && player.first != id) {
                IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
            }
        }

        g_players[player_id]->room_num = 0;

        return true;
    }

    case 13:
    {
        printf("[ No. %3u ] Chatting Packet Received !!\n", id);

        Protocol::CS_Chatting Packet;
        Packet.ParseFromArray(buffer, bufferSize);

        string serializedData;
        Packet.SerializeToString(&serializedData);

        int room_id = g_players[id]->room_num;

        // 모든 연결된 클라이언트에게 패킷 전송 (브로드캐스팅)
        for (const auto& player : g_players) {
            if (player.second->room_num == room_id) {
                IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
            }
        }
        return true;
    }

    case 15:
    {
        printf("[ No. %3u ] SelectReady Packet Received !!\n", id);

        Protocol::SelectReady CS_Packet;
        CS_Packet.ParseFromArray(buffer, bufferSize);

        g_players[CS_Packet.playerid()]->ready = CS_Packet.ready();

        CS_Packet.set_player_num(g_players[CS_Packet.playerid()]->player_num);

        std::string serializedData;
        CS_Packet.SerializeToString(&serializedData);

        int ready_cnt = 0;
        int room_player_cnt = 0;

        for (const auto& player : g_players) {
            if (player.second->room_num == g_players[CS_Packet.playerid()]->room_num) {

                room_player_cnt++;

                IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());

                if (player.second->ready == true) {
                    ready_cnt++;
                }
            }
        }

        if (ready_cnt == room_player_cnt){
            Protocol::SelectAllReady SC_Packet;
            SC_Packet.set_type(6);
            SC_Packet.set_ready(true);

            std::string serializedData;
            SC_Packet.SerializeToString(&serializedData);

            int player_num_cnt = 0;
            for (const auto& player : g_players) {
                if (player.second->room_num == g_players[CS_Packet.playerid()]->room_num) {
                    IOCP_SendPacket(player.first, serializedData.data(), serializedData.size());
                    g_players[player.first]->ready = false;
                    g_players[player.first]->player_num = ++player_num_cnt;
                }
            }
        }

        return true;

    }


    default: {
        printf("ERROR, Unknown signal -> [ %u ] protocol num = %d\n", id, tempPacket.type());
        // 클라이언트나 서버 종료, 로깅 등의 처리 가능
        return true;
    } break;
    }
}

