// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"
#include <fstream>
#include <string>
#include "ProGamemode/ProGameInstance.h"

ClientSocket::ClientSocket(UProGameInstance* Inst)
{
	gameInst = Inst;
	CurrentServerType = ServerType::LOBBY_SERVER;
	//CurrentServerType = ServerType::GAME_SERVER;

	recvBuffer.buf = recvData;
	recvBuffer.len = BUFSIZE;

	if (ConnectServer(CurrentServerType)) {
		Thread = FRunnableThread::Create(this, TEXT("Network Thread"));
	}
}

ClientSocket::~ClientSocket()
{
	for (auto& thread : workerThreads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	if (sendThread.joinable()) {
		sendThread.join();
	}
}

bool ClientSocket::Init()
{
	UE_LOG(LogNet, Warning, TEXT("Thread has been initialized"));
	return true;
}

void ClientSocket::StartRecv()
{
	ZeroMemory(&recvOverlap, sizeof(recvOverlap));

	recvBuffer.len = BUFSIZE;
	recvBuffer.buf = reinterpret_cast<char*>(recvData);

	DWORD flags = 0;
	int result = WSARecv(Socket, &recvBuffer, 1, NULL, &flags, &recvOverlap, NULL);
	if (result == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			UE_LOG(LogNet, Error, TEXT("WSARecv failed: %d"), err);
		}
	}
}

uint32 ClientSocket::Run()
{
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	LPOVERLAPPED overlapped = nullptr;

	while (true)
	{
		BOOL result = GetQueuedCompletionStatus(hIocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);
		if (result == 0 || bytesTransferred == 0){
			UE_LOG(LogNet, Warning, TEXT("Connection closed or error occurred"));
			return 0;
		}

		// 수신 완료된 데이터를 처리
		if (overlapped == &recvOverlap){
			// lock-free 큐에 데이터 추가
			recvQueue.push(std::vector<char>(recvData, recvData + bytesTransferred));
			StartRecv();

			// 수신 큐 처리
			ProcessRecvQueue();
		}
		else{
			StartSend();

			//delete overlapped;
		}
	}
}

void ClientSocket::ProcessRecvQueue()
{
	std::vector<char> raw;
	while (recvQueue.try_pop(raw)) {
		// raw 데이터(길이+페이로드)가 여러 번 들어올 수도, 분할될 수도 있으므로,
		// 누적 버퍼에 append한 뒤, 패킷 파싱 로직 수행
		recvBufferCache.insert(recvBufferCache.end(), raw.begin(), raw.end());

		// 패킷 파싱
		while (true) {
			// 최소 4바이트 있어야 길이 확인 가능
			if (recvBufferCache.size() < 4) {
				break;
			}
			// 4바이트 길이 읽기
			uint32_t packetSize = 0;
			memcpy(&packetSize, recvBufferCache.data(), 4);

			// 아직 패킷 전부 못 받았으면 대기
			if (recvBufferCache.size() < (4 + packetSize)) {
				break;
			}

			// 패킷 하나 완성
			std::vector<char> onePacket(recvBufferCache.begin() + 4,
				recvBufferCache.begin() + 4 + packetSize);

			// 나머지 leftover
			recvBufferCache.erase(recvBufferCache.begin(),
				recvBufferCache.begin() + (4 + packetSize));

			// 이제 onePacket이 "순수 데이터"
			ProcessPacket(onePacket);
		}
	}
}


void ClientSocket::ProcessPacket(const std::vector<char>& buffer)
{
	if (!ReceivedPlayerId)
	{
		ReceivedPlayerId = true;

		memcpy(&MyPlayerId, buffer.data(), sizeof(MyPlayerId));

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MyPlayerId: %d"), MyPlayerId));
		UE_LOG(LogNet, Display, TEXT("Received MyPlayerId: %d"), MyPlayerId); //=> 여러개 받을때 마지막으로 받는게 진짜 MyId임 -> 근데 코드상으로 보면 1번만 나와야하는데;;

		return;
	}

	if (CurrentServerType == ServerType::LOBBY_SERVER) {
		Protocol::SC_Login Packet;
		if (Packet.ParseFromArray(buffer.data(), buffer.size()))
		{
			switch (Packet.type())
			{
			case 3:
			{
				Protocol::SC_Login Login_Packet;
				if (Login_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					b_login = Login_Packet.b_login();
					recv_login = true;
					MyUserName = Login_Packet.id();
				}
				break;
			}
			case 4:
			{
				Protocol::SC_Register Register_Packet;
				if (Register_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					b_register = Register_Packet.b_register();
					recv_register = true;
				}
				break;
			}
			case 5:
			{
				Protocol::WaitingReady WR_Packet;

				if (WR_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_wready.push(WaitingReady(WR_Packet.playerid(), WR_Packet.ready()));
				}

				UE_LOG(LogNet, Display, TEXT("Received WaitingReady"));
				break;
			}

			case 6:
			{
				Protocol::SelectAllReady Ready_Packet;
				if (Ready_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					if (Ready_Packet.ready())
					{
						b_allready = true;
						// 로비 서버 연결 종료
						Exit();

						// 게임 서버에 연결
						if (ConnectServer(ServerType::GAME_SERVER))
						{
							CurrentServerType = ServerType::GAME_SERVER;
							UE_LOG(LogNet, Display, TEXT("Connected to Game Server"));
						}
						else
						{
							UE_LOG(LogNet, Warning, TEXT("Failed to connect to Game Server"));
						}
					}
				}
				break;
			}

			case 7:
			{
				Protocol::Select_Character Select_Packet;

				if (Select_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_select.push(CharacterSelect(Select_Packet.playerid(), Select_Packet.character_type()));
				}

				UE_LOG(LogNet, Display, TEXT("Received Select Character: %d, %d"), Select_Packet.playerid(), Select_Packet.character_type());

				break;
			}

			case 9:
			{
				UE_LOG(LogNet, Display, TEXT("Received Join:"));


				break;
			}

			case 10:
			{
				Protocol::SC_JoinPlayer JP_Packet;

				if (JP_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_jplayer.push(JoinPlayer(JP_Packet.name(), JP_Packet.playerid(), JP_Packet.roomid()));
				}

				if (MyPlayerId == JP_Packet.playerid()) {
					MyRoomId = JP_Packet.roomid();
				}

				UE_LOG(LogNet, Display, TEXT("Received Join Player:"));

				break;
			}

			case 12:
			{
				Protocol::SC_LeavePlayer LP_Packet;

				if (LP_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_lplayer.push(LeavePlayer(LP_Packet.username(), LP_Packet.playerid()));
				}

				UE_LOG(LogNet, Display, TEXT("Received Leave Player:"));

				break;
			}

			case 13:
			{
				UE_LOG(LogNet, Display, TEXT("Received Chatting:"));

				Protocol::SC_Chatting Chat_Packet;

				if (Chat_Packet.ParseFromArray(buffer.data(), buffer.size())) {

					FString FStringchat = FString(UTF8_TO_TCHAR(Chat_Packet.chat().c_str()));

					Q_chat.push(Chatting(Chat_Packet.playerid(), FStringchat));
				}

				break;
			}
			case 14:
			{
				Protocol::WaitingAllReady Ready_Packet;
				if (Ready_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_wAllready.push(true);
				}
				break;
			}

			case 15:
			{
				Protocol::SelectReady SR_Packet;
				if (SR_Packet.ParseFromArray(buffer.data(), buffer.size())) {
					Q_sready.push(SelectReady(SR_Packet.playerid(), SR_Packet.player_num(), SR_Packet.ready()));
				}
				break;
			}

			}
		}
	}


	else if (CurrentServerType == ServerType::GAME_SERVER) {
		Protocol::Character tempCharacterPacket;
		if (tempCharacterPacket.ParseFromArray(buffer.data(), buffer.size()))
		{
			// 메시지 타입 확인
			switch (tempCharacterPacket.packet_type())
			{
			case 1: // Character 메시지 타입 값
			{
				Protocol::Character CharacterPacket;
				if (CharacterPacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					OtherPlayerId = CharacterPacket.playerid();
					FVector NewLocation(CharacterPacket.x(), CharacterPacket.y(), CharacterPacket.z());
					FRotator NewRotation(CharacterPacket.pitch(), CharacterPacket.yaw(), CharacterPacket.roll());

					if (OtherPlayerId != MyPlayerId) {
						Q_player.push(PlayerData(OtherPlayerId, NewLocation, NewRotation, CharacterPacket.charactertype(), CharacterPacket.username(), CharacterPacket.hp()));
						Q_run.push(PlayerRun(CharacterPacket.playerid(), CharacterPacket.b_run()));
					}
				}
				break;
			}
			case 2: // Zombie 메시지 타입 값 ==> 사용 안함 (예전에 사용하던 패킷)
			{
				Protocol::Zombie ZombiePacket;
				if (ZombiePacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					FVector NewLocation(ZombiePacket.x(), ZombiePacket.y(), ZombiePacket.z());
					FRotator NewRotation(ZombiePacket.pitch(), ZombiePacket.yaw(), ZombiePacket.roll());
					Q_zombie.push(ZombieData(ZombiePacket.zombieid(), NewLocation, NewRotation, ZombiePacket.zombietype()));
					//UE_LOG(LogNet, Display, TEXT("push Zombie: ZombieId=%d"), ZombiePacket.zombieid());
				}
				break;
			}
			case 3:
			{
				Protocol::Time TimePacket;
				if (TimePacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Timer = TimePacket.timer();
					//UE_LOG(LogNet, Display, TEXT("Timer: %d"), Timer);
				}
				break;
			}

			case 4:
			{
				Protocol::Character_Attack AttackPacket;
				if (AttackPacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_pattack.push(PlayerAttack(AttackPacket.playerid(), AttackPacket.attack(), AttackPacket.attacktype(), AttackPacket.aimoffset()));
				}
				break;
			}

			case 5:
			{
				Protocol::Equip_Item EquipPacket;
				if (EquipPacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_eitem.push(EquipItem(EquipPacket.playerid(), EquipPacket.itemname(), EquipPacket.itemtype()));
				}
				break;
			}

			case 7:
			{
				Protocol::jump jumpPacket;
				if (jumpPacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_jump.push(PlayerJump(jumpPacket.playerid()));
				}
				break;
			}

			case 8:	//==> 좀비 생성 패킷
			{
				Protocol::ZombieDataList zombieDataList;
				if (zombieDataList.ParseFromArray(buffer.data(), buffer.size()))
				{
					if (!recv_zombiepacket) {

						recv_zombiepacket = true;

						for (const auto& zombie : zombieDataList.zombies())
						{
							FVector NewLocation(zombie.x(), zombie.y(), zombie.z());
							FRotator NewRotation(zombie.pitch(), zombie.yaw(), zombie.roll());
							Q_zombie.push(ZombieData(zombie.zombieid(), NewLocation, NewRotation, zombie.zombietype()));
							//UE_LOG(LogNet, Display, TEXT("ZombieSpawnData recv: %d, playerid = %d"), zombie.zombieid(), MyPlayerId);
						}
					}

					Protocol::send_complete completepacket;
					completepacket.set_packet_type(21);
					completepacket.set_complete_type(1);

					std::string serializedData;
					completepacket.SerializeToString(&serializedData);
					Send(serializedData.size(), (void*)serializedData.data());
				}
				break;
			}
			case 10:	//==> 좀비 움직임 동기화 패킷
			{
				Protocol::ZombiePathList zombiePathList;
				if (zombiePathList.ParseFromArray(buffer.data(), buffer.size())) {
					for (int i = 0; i < zombiePathList.zombiepaths_size(); ++i) {
						const auto& zombiepath = zombiePathList.zombiepaths(i);

						// 로컬 좀비 데이터 구조 생성
						ZombiePath localZombiePath;
						localZombiePath.ZombieId = zombiepath.zombieid();

						// 타겟 타입 설정
						localZombiePath.targetType = zombiepath.target_type();

						// 경로 입력
						localZombiePath.Path1.emplace_back(
							zombiepath.path1().x(), zombiepath.path1().y(), zombiepath.path1().z());
						localZombiePath.Path2.emplace_back(
							zombiepath.path2().x(), zombiepath.path2().y(), zombiepath.path2().z());

						// 위치 값 추가
						localZombiePath.Location = FVector(
							zombiepath.location().x(), zombiepath.location().y(), zombiepath.location().z());

						// 큐에 ZombiePath 객체 추가
						Q_path.push(localZombiePath);

						// 디버그 로그 (필요에 따라 활성화)
						// UE_LOG(LogNet, Display, TEXT("Zombie #%d's Location: ( %.2f, %.2f, %.2f )"),
						//     localZombiePath.ZombieId, localZombiePath.Location.X, localZombiePath.Location.Y, localZombiePath.Location.Z);
					}

					// 디버그 로그
					//UE_LOG(LogNet, Display, TEXT("Processed %d ZombiePaths"), zombiePathList.zombiepaths_size());
				}

				break;
			}

			case 11:
			{
				Protocol::ping pingpacket;
				if (pingpacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_ping.push(true);
					//UE_LOG(LogNet, Display, TEXT("ping"));
				}
				break;
			}

			case 12:
			{
				Protocol::Zombie_hp zombiehp;
				if (zombiehp.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_zhp.push(ZombieHP(zombiehp.zombieid(), zombiehp.damage()));
					UE_LOG(LogNet, Display, TEXT("Zombie HP"));
				}
				break;
			}

			case 13:
			{
				Protocol::Zombie_attack zombieattack;
				if (zombieattack.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_zattack.push(ZombieAttack(zombieattack.zombieid(), zombieattack.playerid()));
					UE_LOG(LogNet, Display, TEXT("Zombie Attack"));
				}
				break;
			}

			case 15:
			{
				Protocol::ItemDataList itemdatalist;
				if (itemdatalist.ParseFromArray(buffer.data(), buffer.size()))
				{
					if (!recv_itempacket) {
						recv_itempacket = true;

						for (const auto& item : itemdatalist.items()) {
							Q_setitem.push(Set_Item(item.itemid(), item.itemname(), item.itemclass(), item.texture_path(),
								item.count(), item.floor(), FVector(item.posx(), item.posy(), item.posz())));
							//UE_LOG(LogNet, Display, TEXT("ItemSpawnData recv: %d, playerid = %d"), item.itemid(), MyPlayerId);
						}
					}

					Protocol::send_complete completepacket;
					completepacket.set_packet_type(21);
					completepacket.set_complete_type(2);

					std::string serializedData;
					completepacket.SerializeToString(&serializedData);
					Send(serializedData.size(), (void*)serializedData.data());
				}
				break;
			}

			case 16:
			{
				Protocol::CarDataList cardatalist;
				if (cardatalist.ParseFromArray(buffer.data(), buffer.size()))
				{
					if (!recv_carpacket) {
						recv_carpacket = true;

						for (const auto& car : cardatalist.cars()) {
							Q_setcar.push(Set_Car(car.carid(), car.carname(), FVector(car.posx(), car.posy(), car.posz()), FRotator(car.pitch(), car.yaw(), car.roll()),
								car.carkeyname()));
							//UE_LOG(LogNet, Display, TEXT("CarSpawnData recv: %d, playerid = %d"), car.carid(), MyPlayerId);
						}

					}

					Protocol::send_complete completepacket;
					completepacket.set_packet_type(21);
					completepacket.set_complete_type(3);

					std::string serializedData;
					completepacket.SerializeToString(&serializedData);
					Send(serializedData.size(), (void*)serializedData.data());
				}
				break;
			}

			case 17:
			{
				Protocol::destroy_item destroyitem;
				if (destroyitem.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_ditem.push(Destroy_Item(destroyitem.itemid(), destroyitem.playerid()));
					UE_LOG(LogNet, Display, TEXT("Destroy item %d"), destroyitem.itemid());
				}
				break;
			}
			case 18:
			{
				Protocol::get_key getkey;
				if (getkey.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_getkey.push(Get_Key(getkey.itemid(), getkey.playerid()));
					UE_LOG(LogNet, Display, TEXT("get key %d"), getkey.itemid());
				}
				break;
			}
			case 19:
			{
				Protocol::escape escapepacket;
				if (escapepacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_escape.push(Escape_Root(escapepacket.playerid(), escapepacket.root(), escapepacket.carid()));
					UE_LOG(LogNet, Display, TEXT("escape root open %d"), escapepacket.root());
				}
				break;
			}
			case 20:
			{
				Protocol::game_clear clearpacket;
				if (clearpacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_gclear.push(GameClear(clearpacket.root(), clearpacket.alive_players(), clearpacket.dead_players(), clearpacket.open_player(),
						clearpacket.my_killcount(), clearpacket.best_killcount(), clearpacket.best_kill_player()));
					UE_LOG(LogNet, Warning, TEXT("Game Clear"));
				}
				break;
			}
			case 22:
			{
				Protocol::drop_item droppacket;
				if (droppacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_dropitem.push(Drop_Item(droppacket.itemid(), droppacket.itemname(), droppacket.itemclass(), droppacket.texture_path(),
						droppacket.count(), FVector(droppacket.posx(), droppacket.posy(), droppacket.posz()), droppacket.durability(), droppacket.durability_max()));

					UE_LOG(LogNet, Display, TEXT("recv drop itempacket"));
				}
				break;
			}
			case 23:
			{
				Protocol::detach_item detachpacket;
				if (detachpacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_detachitem.push(DetachItem(detachpacket.playerid(), detachpacket.itemtype()));

					UE_LOG(LogNet, Display, TEXT("recv detach itempacket"));
				}
				break;
			}
			case 24:
			{
				Protocol::slice_vector slicepacket;
				if (slicepacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					SliceVector slicevector;

					slicevector.zombieid = slicepacket.zombieid();

					slicevector.location = FVector(slicepacket.location().x(), slicepacket.location().y(), slicepacket.location().z());
					slicevector.position = FVector(slicepacket.position().x(), slicepacket.position().y(), slicepacket.position().z());
					slicevector.normal = FVector(slicepacket.normal().x(), slicepacket.normal().y(), slicepacket.normal().z());
					slicevector.impulse = FVector(slicepacket.impulse().x(), slicepacket.impulse().y(), slicepacket.impulse().z());

					Q_slicevector.push(slicevector);

					UE_LOG(LogNet, Display, TEXT("recv slicevector"));
				}
				break;
			}
			case 25:
			{
				Protocol::Zombie_shouting shoutingpacket;
				if (shoutingpacket.ParseFromArray(buffer.data(), buffer.size()))
				{
					Q_shouting.push(ZombieShouting(shoutingpacket.zombieid(), shoutingpacket.playerid()));

					UE_LOG(LogNet, Display, TEXT("recv zombie shouting packet"));
				}
				break;
			}
			case 26:
			{
				Protocol::chatting gamechatpacket;
				if (gamechatpacket.ParseFromArray(buffer.data(), buffer.size())) {

					FString FStringchat = FString(UTF8_TO_TCHAR(gamechatpacket.chat().c_str()));

					Q_Gchat.push(GameChatting(gamechatpacket.playerid(), FStringchat));
				}
				break;
			}
			}
		}
	}
}



void ClientSocket::Exit()
{
	if (Socket)
	{
		closesocket(Socket);
		WSACleanup();
		Socket = INVALID_SOCKET;
	}
}

bool ClientSocket::ConnectServer(ServerType serverType)
{
	ReceivedPlayerId = false;

	WSADATA wsaData;
	int retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retval != 0)
	{
		return false;
	}

	Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == INVALID_SOCKET)
	{
		return false;
	}

	BOOL bNoDelay = true;
	retval = setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));

	SOCKADDR_IN ServerAddr;

	ServerAddr.sin_family = AF_INET;
	if (serverType == ServerType::LOBBY_SERVER)
	{
		ServerAddr.sin_port = htons(7777);
	}

	if (serverType == ServerType::GAME_SERVER)
	{
		ServerAddr.sin_port = htons(8777);
	}

	FString SourcePath = FPaths::LaunchDir() + TEXT("ipconfig.txt");

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*SourcePath))
	{
		UE_LOG(LogNet, Error, TEXT("Config file not found at: %s"), *SourcePath);
		return false;
	}


	std::ifstream configFile(TCHAR_TO_UTF8(*SourcePath));
	if (!configFile.is_open())
	{
		UE_LOG(LogNet, Error, TEXT("Failed to open config file at: %s"), *SourcePath);
		return false;
	}

	std::string serverAddress;
	std::getline(configFile, serverAddress); // 첫 줄 읽기
	configFile.close();

	if (serverAddress.empty())
	{
		UE_LOG(LogNet, Error, TEXT("Server address is empty in config file"));
		return false;
	}

	inet_pton(AF_INET, serverAddress.c_str(), &ServerAddr.sin_addr);

	retval = connect(Socket, (sockaddr*)&ServerAddr, sizeof(sockaddr));

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Trying to connect.")));

	if (retval == SOCKET_ERROR)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Fail")));
		return false;
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Success!")));

		UE_LOG(LogNet, Display, TEXT("Socket Initialized"));

		hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (hIocp == NULL)
		{
			UE_LOG(LogNet, Error, TEXT("Failed to create IOCP"));
			closesocket(Socket);
			WSACleanup();
			return false;
		}

		// 소켓을 IOCP에 연결
		if (CreateIoCompletionPort((HANDLE)Socket, hIocp, (ULONG_PTR)this, 0) == NULL)
		{
			UE_LOG(LogNet, Error, TEXT("Failed to associate socket with IOCP"));
			return false;
		}

		// 비동기 수신 시작
		StartRecv();
		return true;
	}
}

bool ClientSocket::Send(const int SendSize, void* SendData)
{
	// 1) [4바이트 길이 + 실제 데이터]로 구성할 버퍼 생성
	uint32_t packetSize = static_cast<uint32_t>(SendSize);  // 실제 데이터 크기
	uint32_t totalSize = 4 + packetSize;                    // 4바이트 길이 + 실제 데이터

	// 메모리를 한번에 할당하여, 헤더(4) + Payload(SendData)
	std::vector<char> buffer(totalSize);

	// [0..3]에 패킷 길이 기록
	memcpy(buffer.data(), &packetSize, sizeof(packetSize));
	// [4..(4+packetSize-1)]에 실제 데이터 복사
	memcpy(buffer.data() + 4, (char*)SendData, SendSize);

	// 2) 큐에 넣기
	sendQueue.push(std::move(buffer));

	// 3) isSending 상태 확인 후, 전송 스레드 기동
	bool expected = false;
	if (isSending.compare_exchange_strong(expected, true)) {
		StartSend(); // 송신 작업 시작
	}
	return true;
}

void ClientSocket::StartSend()
{
	while (true) {
		std::vector<char> data;

		// 큐에서 데이터 꺼내기
		if (!sendQueue.try_pop(data)) {
			// 송신할 데이터가 없으면 플래그 해제 후 종료
			isSending.store(false);
			return;
		}

		// 4바이트 길이 + 실제 데이터가 담긴 버퍼
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(data.data());
		wsaBuf.len = static_cast<ULONG>(data.size());

		// Overlapped 생성
		auto sendOverlap = std::make_unique<OVERLAPPED>();
		ZeroMemory(sendOverlap.get(), sizeof(OVERLAPPED));

		DWORD bytesSent = 0;
		int result = WSASend(Socket, &wsaBuf, 1, &bytesSent, 0, sendOverlap.get(), NULL);

		if (result == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				UE_LOG(LogNet, Error, TEXT("WSASend failed: %d"), err);

				// 재시도 로직
				constexpr int MaxRetries = 3;
				for (int retry = 1; retry <= MaxRetries; ++retry) {
					UE_LOG(LogNet, Warning, TEXT("Retrying WSASend... Attempt %d"), retry);
					result = WSASend(Socket, &wsaBuf, 1, &bytesSent, 0, sendOverlap.get(), NULL);
					if (result != SOCKET_ERROR || WSAGetLastError() == WSA_IO_PENDING) {
						sendOverlap.release(); // 성공 혹은 IO_PENDING이면 Overlapped 유지
						return;
					}
				}

				UE_LOG(LogNet, Error, TEXT("WSASend failed after retries"));
				bool expected = true;
				isSending.compare_exchange_strong(expected, false); // 송신 작업 중단
				return;
			}
		}
		// 정상 혹은 IO_PENDING이면 Overlapped 유지(WorkerThread에서 해제)
		sendOverlap.release();
	}
}



uint32 ClientSocket::GetMyPlayerId() const
{
	return MyPlayerId;
}