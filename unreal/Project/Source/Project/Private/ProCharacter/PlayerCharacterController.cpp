// Fill out your copyright notice in the Description page of Project Settings.

#include "ProCharacter/PlayerCharacterController.h"

// Engine
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

// GameMode
#include "ProGamemode/OneGameModeBase.h"
#include "ProGamemode/ProGameInstance.h"

// Character
#include "ProCharacter/BaseCharacter.h"
#include "ProCharacter/EmployeeCharacter.h"
#include "ProCharacter/GirlCharacter.h"
#include "ProCharacter/IdolCharacter.h"
#include "ProCharacter/FireFighterCharacter.h"
#include "ProCharacter/PlayerCharacterAnimInstance.h"

// Zombie
#include "ProZombie/BaseZombie.h"

//Network
#include "GStruct.pb.h"

#include "ProUI/GameChatUI.h"

// EnhancedInput
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"


APlayerCharacterController::APlayerCharacterController(const FObjectInitializer& ObjectInitalizer)
	:Super(ObjectInitalizer)
{
	InputMapping = CreateDefaultSubobject<UInputMappingContext>(TEXT("INPUTMAPPING"));

	InputActions = CreateDefaultSubobject<UInputDataAsset>(TEXT("INPUTACTIONS"));

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> SK_INPUTMAPPINGCONTEXT(TEXT("/Game/input/IMC_InputMappingCon.IMC_InputMappingCon"));

	if (SK_INPUTMAPPINGCONTEXT.Succeeded()) {
		InputMapping = SK_INPUTMAPPINGCONTEXT.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputDataAsset> SK_INPUTDATAASSET(TEXT("/Game/input/BP_InputDataAsset.BP_InputDataAsset"));

	if (SK_INPUTDATAASSET.Succeeded()) {
		InputActions = SK_INPUTDATAASSET.Object;
	}

}

void APlayerCharacterController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APlayerCharacterController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UProGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}


void APlayerCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	//// 캐릭터 움직임 통신 작업
	//TimeSinceLastSend += DeltaTime; // 시간 누적

	//if (TimeSinceLastSend >= 0.2f) // 0.016초마다 전송 실행 
	//	// (약 60분에 1초 => 최소 60 프레임이 방어되면 캐릭터 움직임 딜레이되는 일 없음 -> 딜레이 발생하더라도 아래에서 큐에 2개 이상 쌓이면 모두 빼도록 처리해둠)
	//{
	//	CheckAndSendMovement();
	//	TimeSinceLastSend = 0.0f; // 초기화
	//}

	CheckAndSendMovement(DeltaTime);
	Check_run();
	Send_Attack();
	Send_Equipment();
	//Send_run();
	Send_jump();


	// ClientSocketPtr 큐에 쌓인 명령들 실행(pop)
	if (GameInstance && GameInstance->ClientSocketPtr)
	{

		if (GameInstance->ClientSocketPtr->Q_player.try_pop(recvPlayerData))
		{
			latestData = recvPlayerData;

			//// 현재 GameMode 인스턴스를 얻기
			//if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			//{
			//	// GameMode 내의 함수 호출하여 다른 플레이어의 위치 업데이트
			//	MyGameMode->UpdateOtherPlayer(recvPlayerData.PlayerId, recvPlayerData.Location, recvPlayerData.Rotation, recvPlayerData.charactertype,
			//		recvPlayerData.username, recvPlayerData.hp, DeltaTime);

			//	// 현재 컨트롤러가 빙의한 Pawn 가져오기
			//	APawn* ControlledPawn = GetPawn();

			//	//UE_LOG(LogNet, Display, TEXT("[Update Other Player]: hp=%f"), recvPlayerData.hp);

			//	// 빙의된 Pawn이 ACharacter라면 캐스팅
			//	if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
			//	{
			//		if (ControlledCharacter) {
			//			ControlledCharacter->UpdateOtherPlayerUI(recvPlayerData.PlayerId, recvPlayerData.hp, recvPlayerData.charactertype, recvPlayerData.username);
			//		}
			//	}
			//}

			// 플레이어 큐가 너무 많이 쌓여서 전체적인 캐릭터 동기화에 딜레이가 생기는 걸 방지하기 위해
			int threshold = 2;	// 2개 이상 쌓이면 이전 데이터 try_pop해서 비우기 
			int loop = 0;	// 몇번 try_pop 할 횟수

			if (GameInstance->ClientSocketPtr->Q_player.unsafe_size() >= threshold) {
				loop = GameInstance->ClientSocketPtr->Q_player.unsafe_size();	// 쌓여있는거 다 비우기
			}
			else {
				loop = 1;	// threshold 이하면 아래 for문 한번만 실행되게(try_pop 한번만) 하려고
			}

			int pop_cnt = 0;

			for (pop_cnt = 0; pop_cnt < loop; pop_cnt++) {

				//UE_LOG(LogNet, Display, TEXT("[Update Other Player]: PlayerId=%d"), recvPlayerData.PlayerId);

				// 현재 GameMode 인스턴스를 얻기
				if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
				{
					// GameMode 내의 함수 호출하여 다른 플레이어의 위치 업데이트
					MyGameMode->UpdateOtherPlayer(recvPlayerData.PlayerId, recvPlayerData.Location, recvPlayerData.Rotation, recvPlayerData.charactertype,
						recvPlayerData.username, recvPlayerData.hp, DeltaTime);

					// 현재 컨트롤러가 빙의한 Pawn 가져오기
					APawn* ControlledPawn = GetPawn();

					//UE_LOG(LogNet, Display, TEXT("[Update Other Player]: hp=%f"), recvPlayerData.hp);

					// 빙의된 Pawn이 ACharacter라면 캐스팅
					if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
					{
						if (ControlledCharacter) {
							ControlledCharacter->UpdateOtherPlayerUI(recvPlayerData.PlayerId, recvPlayerData.hp, recvPlayerData.charactertype, recvPlayerData.username);
						}
					}
				}

				if (GameInstance->ClientSocketPtr->Q_player.try_pop(recvPlayerData) == false)
					break;
			}
			if (loop != 1) {
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("{Q_player} cleared (try_pop count): %d"), pop_cnt));
				UE_LOG(LogTemp, Error, TEXT("{Q_player} cleared (try_pop count): %d"), pop_cnt);
			}
		}
		else {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))) {
				MyGameMode->UpdateOtherPlayer(latestData.PlayerId, latestData.Location, latestData.Rotation, latestData.charactertype,
					latestData.username, latestData.hp, DeltaTime);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_pattack.try_pop(recvPlayerAttack))
		{
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdatePlayerAttack(recvPlayerAttack.PlayerId, recvPlayerAttack.b_attack, recvPlayerAttack.attacktype, recvPlayerAttack.aimoffset);
				UE_LOG(LogNet, Display, TEXT("Update Other Player: PlayerId=%d"), recvPlayerData.PlayerId);
				UE_LOG(LogTemp, Warning, TEXT("Q_pattack.try_pop(recvPlayerAttack) aimoffset: %f"), recvPlayerAttack.aimoffset);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_eitem.try_pop(recvEquipItem))
		{
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				FString EquipItem = *FString(recvEquipItem.Itemname.c_str());
				MyGameMode->UpdateEquipItem(recvEquipItem.PlayerId, EquipItem, recvEquipItem.Itemtype);
				//UE_LOG(LogNet, Display, TEXT("Update Other Player: PlayerId=%d"), recvEquipItem.PlayerId);
			}
		}
		// MyGameMode->UpdateUnEquipItem(recvUnEquipItem.PlayerId, recvUnEquipItem.Itemtype); Send부분 필요(아이템 장착해제시 동기화를 위해 필요)
		// recvUnEquipItem.PlayerId, recvUnEquipItem.ItemType 와 이걸 보내줄수있는 Send()부분 있으면 남은 부분 제작 예정

		if (GameInstance->ClientSocketPtr->Q_detachitem.try_pop(recvDetachItem))
		{
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdateUnEquipItem(recvDetachItem.PlayerId, recvDetachItem.Itemtype);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_slicevector.try_pop(recvSliceVector))
		{
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdateCuttingZombie(recvSliceVector.zombieid, recvSliceVector.location, recvSliceVector.position, recvSliceVector.normal, recvSliceVector.impulse);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_run.try_pop(recvRun))
		{
			// 러닝 큐가 너무 많이 쌓여서 걷기/뛰기 애니메이션 동기화에 딜레이가 생기는 걸 방지하기 위해
			int threshold = 2;	// 2개 이상 쌓이면 이전 데이터 try_pop해서 비우기 
			int loop = 0;	// 몇번 try_pop 할 횟수

			if (GameInstance->ClientSocketPtr->Q_run.unsafe_size() >= threshold) {
				loop = GameInstance->ClientSocketPtr->Q_run.unsafe_size();	// 쌓여있는거 다 비우기
			}
			else {
				loop = 1;	// threshold 이하면 아래 for문 한번만 실행되게(try_pop 한번만) 하려고
			}

			int pop_cnt = 0;

			for (pop_cnt = 0; pop_cnt < loop; pop_cnt++) {

				if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
				{
					if (recvRun.b_run == 1) {
						MyGameMode->UpdatePlayerRun(recvRun.PlayerId, false);
					}
					else if (recvRun.b_run == 2) {
						MyGameMode->UpdatePlayerRun(recvRun.PlayerId, true);
					}
					//UE_LOG(LogNet, Display, TEXT("Update Other Player: PlayerId=%d"), recvPlayerData.PlayerId);
				}

				if (GameInstance->ClientSocketPtr->Q_run.try_pop(recvRun) == false)
					break;
			}
			if (loop != 1) {
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("{Q_run} cleared (try_pop count): %d"), pop_cnt));
				//UE_LOG(LogTemp, Error, TEXT("{Q_run} cleared (try_pop count): %d"), pop_cnt);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_jump.try_pop(recvJump)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdatePlayerJump(recvJump.PlayerId);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_setitem.try_pop(recvSetItem)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				FName Fitemname = FName(recvSetItem.itemname.c_str());

				FString Ftexture_path = FString(recvSetItem.texture_path.c_str());

				// FString을 TCHAR*로 변환하여 LoadObject에 전달
				UTexture2D* LoadedTexture = LoadObject<UTexture2D>(NULL, *Ftexture_path, NULL, LOAD_None, NULL);

				MyGameMode->SpawnItemBoxes(recvSetItem.itemid, Fitemname, recvSetItem.itemclass, LoadedTexture, recvSetItem.count, recvSetItem.floor, recvSetItem.itempos);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_dropitem.try_pop(recvDropItem)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				FName Fitemname = FName(recvDropItem.itemname.c_str());

				FString Ftexture_path = FString(recvDropItem.texture_path.c_str());

				UTexture2D* LoadedTexture = LoadObject<UTexture2D>(NULL, *Ftexture_path, NULL, LOAD_None, NULL);

				UE_LOG(LogNet, Display, TEXT("queue try pop Q_dropitem"));

				MyGameMode->SpawnOtherCharGroundItemBoxes(recvDropItem.itemid, Fitemname, recvDropItem.itemclass, LoadedTexture, recvDropItem.count, recvDropItem.itempos, recvDropItem.durability, recvDropItem.durability_max);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_setcar.try_pop(recvSetCar)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				FName Fcarname = FName(recvSetCar.carname.c_str());
				FName Fcarkeyname = FName(recvSetCar.carkeyname.c_str());

				MyGameMode->SpawnInterItem(recvSetCar.carid, Fcarname, recvSetCar.carpos, recvSetCar.carrotator, Fcarkeyname);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_ditem.try_pop(recvDestroyItem)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				UE_LOG(LogNet, Display, TEXT("queue try pop destroy item: itemid= %d"), recvDestroyItem.itemid);
				MyGameMode->DestroyItem(recvDestroyItem.itemid, recvDestroyItem.playerid);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_getkey.try_pop(recvGetkey)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				//처리 recvGetKey.itemid, recvGetKey.playerid

				UE_LOG(LogNet, Display, TEXT("queue try pop get_key: itemid= %d"), recvGetkey.itemid);
				APawn* ControlledPawn = GetPawn();
				if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
				{
					if (ControlledCharacter) {
						ControlledCharacter->UpdatePickUpKey(recvGetkey.itemid, recvGetkey.playerid);
					}
				}

			}
		}

		if (GameInstance->ClientSocketPtr->Q_escape.try_pop(recvEscapeRoot)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				//처리 recvEscapeRoot.playerid, recvEscapeRoot.root
				APawn* ControlledPawn = GetPawn();
				if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
				{
					if (ControlledCharacter) {
						ControlledCharacter->UpdateOpenKey(recvEscapeRoot.root);
					}
				}

				MyGameMode->TurnOnCarHeadlights(recvEscapeRoot.carid);
				UE_LOG(LogNet, Display, TEXT("turn on headlight: carid = %d"), recvEscapeRoot.carid);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_Gchat.try_pop(recvGameChat)) {
			APawn* ControlledPawn = GetPawn();
			if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
			{
				if (ControlledCharacter->GameChatUIWidget)
				{
					ControlledCharacter->GameChatUIWidget->AddChatMessage(recvGameChat.chat);
				}
			}
		}

		if (GameInstance->ClientSocketPtr->Q_gclear.try_pop(recvGameClear)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				/*uint32 root;
				uint32 alive_players;
				uint32 dead_players;
				std::string open_player;
				uint32 my_kill_count;
				uint32 best_kill_count;
				std::string best_kill_player;*/


				UE_LOG(LogNet, Warning, TEXT("recvGameClearrecvGameClearrecvGameClearrecvGameClearrecvGameClear"));

				//처리 recvEscapeRoot.playerid, recvEscapeRoot.root
				APawn* ControlledPawn = GetPawn();
				if (ABaseCharacter* ControlledCharacter = Cast<ABaseCharacter>(ControlledPawn))
				{
					if (ControlledCharacter) {
						std::string bestkillStr = recvGameClear.best_kill_player;
						FString bestkillplayerFStr = FString(bestkillStr.c_str());  // std::string을 FString으로 변환

						std::string OpenStr = recvGameClear.open_player;
						FString OpenPlayerFStr = FString(OpenStr.c_str());  // std::string을 FString으로 변환

						ControlledCharacter->ProGameClear(recvGameClear.root
							, recvGameClear.alive_players
							, recvGameClear.dead_players
							, OpenPlayerFStr
							, recvGameClear.my_kill_count
							, recvGameClear.best_kill_count
							, bestkillplayerFStr);


						// 서버에서 게임 클리어 패킷 그만 보내도록 다시 답장
						Protocol::send_complete completepacket;
						completepacket.set_packet_type(21);
						completepacket.set_complete_type(4);

						std::string serializedData;
						completepacket.SerializeToString(&serializedData);

						bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
				
					}
				}
			}
		}

		//UE_LOG(LogNet, Display, TEXT("Update call Zombie: Playerid=%d"), GameInstance->ClientSocketPtr->MyPlayerId);
		if (GameInstance->ClientSocketPtr->Q_zombie.try_pop(recvZombieData))
		{
			//UE_LOG(LogNet, Display, TEXT("try_pop Zombie: ZombieId=%d"), recvZombieData.ZombieId);
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdateZombie(recvZombieData.ZombieId, recvZombieData.ZombieType, recvZombieData.Location, recvZombieData.Rotation);
				//UE_LOG(LogNet, Display, TEXT("Spawn Zombie: Playerid=%d, Zombieid=%d"), GameInstance->ClientSocketPtr->MyPlayerId, recvZombieData.ZombieId);	// 좀비가 스폰될때 불림
			}
		}

		if (GameInstance->ClientSocketPtr->Q_zattack.try_pop(recvZombieAttack))
		{
			//UE_LOG(LogNet, Display, TEXT("try_pop Zombie: ZombieId=%d"), recvZombieData.ZombieId);
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdateZombieAttack(recvZombieAttack.ZombieId, recvZombieAttack.PlayerId);
				//UE_LOG(LogNet, Display, TEXT("Update Zombie Attack: Zombieid=%d, Playerid=%d"), recvZombieAttack.ZombieId, recvZombieAttack.PlayerId);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_zhp.try_pop(recvZombieHP)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				MyGameMode->UpdateZombieHP(recvZombieHP.ZombieId, recvZombieHP.Damage);
			}
		}

		if (GameInstance->ClientSocketPtr->Q_shouting.try_pop(recvZombieShouting)) {
			if (AOneGameModeBase* MyGameMode = Cast<AOneGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
			{
				// zombieshouting 함수 호출 추가
				MyGameMode->UpdateShoutingZombie(recvZombieShouting.zombieid, recvZombieShouting.playerid);
			}
		}

		//UpdateCuttingZombie(uint32 ZombieId, FVector zombieLocation, FVector planePosition, FVector planeNormal, FVector impulseDirection)

		bool recvping = false;
		if (GameInstance->ClientSocketPtr->Q_ping.try_pop(recvping))
		{
			if (recvping)
			{
				Protocol::ping packet;

				uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();

				packet.set_packet_type(11);
				packet.set_playerid(MyPlayerId);

				std::string serializedData;
				packet.SerializeToString(&serializedData);

				bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
			}
		}
	}


	// 좀비 관련 이지만 일단 좀비 관련 싱글턴 객체가 딱히 없어 - 여기에서 작업
	// 좀비 관련 네트워크 데이터 각 좀비 객체에 할당

	if (GameInstance->ClientSocketPtr->Q_path.empty()) {
		return;
	}
	while (GameInstance->ClientSocketPtr->Q_path.empty() != true) {
		ZombiePath tmp_path;
		GameInstance->ClientSocketPtr->Q_path.try_pop(tmp_path);

		if (ZombieMap.IsEmpty() == true) {
			break;
		}

		if (&tmp_path == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("(Q_path) tmp_path is empty!"));	// 레어한데 갑자기 계단에 있을때 tmp_path에 잘못된 메모리 참조 오류가 발생해서 추가;;
			continue;
		}

		ABaseZombie** zombie = ZombieMap.Find(tmp_path.ZombieId);

		if (zombie == nullptr || *zombie == nullptr) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("(Q_path) Could not find Zombie ID: %d"), tmp_path.ZombieId));
			UE_LOG(LogTemp, Warning, TEXT("(Q_path) Could not find Zombie ID: %d"), tmp_path.ZombieId);
			continue;  // 좀비를 찾을 수 없으면 다음으로 넘어감
		}

		FString prevType;
		//FString currentType;

		switch ((*zombie)->targetType) {
		case 1:
			prevType = TEXT("[NULL]");
			break;
		case 2:
			prevType = TEXT("[Player]");
			break;
		case 3:
			prevType = TEXT("[Shouting]");
			break;
		case 4:
			prevType = TEXT("[FootSound]");
			break;
		case 5:
			prevType = TEXT("[Investigated]");
			break;
		case 6:
			prevType = TEXT("[Patrol]");
			break;
		case 7:
			prevType = TEXT("[HordeSound]");
			break;
		case 8:
			prevType = TEXT("[Runaway]");
			break;
		case 9:
			prevType = TEXT("[HaveToWait]");
			break;
		case 10:
			prevType = TEXT("[LookAround]");
			break;

		case 69:
			prevType = TEXT("[BlackboardClear]");
			break;
		default:
			prevType = FString::Printf(TEXT("[Error: %d]"), (*zombie)->targetType);
			break;
		}

		switch (tmp_path.targetType) {

		case 1:
			if ((*zombie)->targetType != (*zombie)->TARGET::NULL_TARGET) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [NULL]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [NULL]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::NULL_TARGET;
			break;

		case 2:
			if ((*zombie)->targetType != (*zombie)->TARGET::PLAYER) {	// 좀비 플레이어를 처음 발견 또는 놓쳤다가 다시 발견했을 때 호드 사운드 재생

				// 도망치기가 끝나고 바로 플레이어를 발견? => 코너에 몰려서 도망치기를 포기하고 맞서 싸울때이다!!! ===> 호드 사운드로 주위에 도움 요청 & 플레이어에게 상태가 바뀜을 알림
				if ((*zombie)->targetType == (*zombie)->TARGET::RUNAWAY) {
					(*zombie)->IsGrowlSoundPlaying = false;	// 도망치기가 끝난 직후에 바로 눈 앞에 플레이어 있으면 곧바로 소리치도록
					GetWorld()->GetTimerManager().ClearTimer((*zombie)->GrowlSoundTimerHandle);	// 타이머도 초기화시켜주고
				}

				(*zombie)->PlayGrowlSound();

				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [Player]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [Player]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::PLAYER;
			break;

		case 3:
			if ((*zombie)->targetType != (*zombie)->TARGET::SHOUTING) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [Shouting]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [Shouting]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::SHOUTING;
			break;

		case 4:
			if ((*zombie)->targetType != (*zombie)->TARGET::FOOTSOUND) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [FootSound]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [FootSound]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::FOOTSOUND;
			break;

		case 5:
			if ((*zombie)->targetType != (*zombie)->TARGET::INVESTIGATED) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [Investigated]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [Investigated]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::INVESTIGATED;
			break;

		case 6:
			if ((*zombie)->targetType != (*zombie)->TARGET::PATROL) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [Patrol]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[[Q_path] Zombie %d's targetType: %s -> [Patrol]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::PATROL;
			break;

		case 7:
			if ((*zombie)->targetType != (*zombie)->TARGET::HORDESOUND) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [HordeSound]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [HordeSound]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::HORDESOUND;
			break;

		case 8:
			if ((*zombie)->targetType != (*zombie)->TARGET::RUNAWAY) {
				(*zombie)->IgnorePlayer();

				(*zombie)->PlayScaredSound();

				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [Runaway]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [Runaway]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::RUNAWAY;
			break;

		case 9:
			if ((*zombie)->targetType != (*zombie)->TARGET::WAIT) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [HaveToWait]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [HaveToWait]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::WAIT;
			break;

		case 10:
			if ((*zombie)->targetType != (*zombie)->TARGET::LOOKINGAROUND) {
				UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [LookAround]"), tmp_path.ZombieId, *prevType);
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [LookAround]"), tmp_path.ZombieId, *prevType));
			}

			(*zombie)->targetType = (*zombie)->TARGET::LOOKINGAROUND;
			break;


		case 69:	// 서버에서 블랙보드가 클리어 됨을 알려줬을때 (애니메이션 재생 끝난 후)
			UE_LOG(LogTemp, Log, TEXT("[Q_path] Zombie %d's targetType: %s -> [BlackboardClear]"), tmp_path.ZombieId, *prevType);
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [BlackboardClear]"), tmp_path.ZombieId, *prevType));

			(*zombie)->targetType = (*zombie)->TARGET::BLACKBOARDCLEARED;
			UE_LOG(LogTemp, Warning, TEXT("Detect 패킷 리셋!(m_bPlayerInSight = false;)"));
			(*zombie)->m_bPlayerInSight = false;	// Detect 패킷 초기화 (다시 플레이어가 시야에 있는지 없는지 판단하도록)
			break;

		default:
			UE_LOG(LogTemp, Error, TEXT("[Q_path] Zombie %d's targetType: %s -> [Error: %d]"), tmp_path.ZombieId, *prevType, tmp_path.targetType);
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Q_path] Zombie %d's targetType: %s -> [Error: %d]"), tmp_path.ZombieId, *prevType, tmp_path.targetType));
			break;
		}

		bool isAnimPlaying_besideWalking = false;	// 걷기 애니메이션 말고 다른 애니메이션 (공격, 피격, 샤우팅) 재생 중인지

		// 가끔씩 이유는 모르겠는데 다른 층에서 이동하면 좀비가 바닥으로 떨어져서 여기서 크래시 발생할 때 꽤 있음;;; (특히 로컬에서 2개 동시에 켰을 시에)
		if ((*zombie)->CachedAnimInstance->Montage_IsPlaying((*zombie)->CachedAnimInstance->AttackMontage) == true
			|| (*zombie)->CachedAnimInstance->Montage_IsPlaying((*zombie)->CachedAnimInstance->BeAttackedMontage) == true
			|| (*zombie)->CachedAnimInstance->Montage_IsPlaying((*zombie)->CachedAnimInstance->ShoutingMontage) == true) {

			isAnimPlaying_besideWalking = true;
		}

		if (isAnimPlaying_besideWalking == false) {	// 공격, 피격, 샤우팅 애니메이션이 재생 중이지 않을때만

			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Update Zombie path: ZombieID=%d, PlayerID=%d"/*, NextPath=(%.2f , %.2f , %.2f)"*/), tmp_path.ZombieId, GameInstance->ClientSocketPtr->MyPlayerId));
			//UE_LOG(LogNet, Display, TEXT("Update Zombie path: ZombieID=%d, PlayerID = %d"/*, NextPath=(%.2f , %.2f , %.2f)"*/), tmp_path.ZombieId, GameInstance->ClientSocketPtr->MyPlayerId);

			// 좀비 위치 서버에서 받은 위치로 갱신
			if (tmp_path.Location.IsZero() == false) {
				FVector SmoothedLocation = FMath::VInterpTo((*zombie)->GetActorLocation(), tmp_path.Location, GetWorld()->GetDeltaSeconds(), 10.0f);     // 보간 (부드럽게 보이게)

				(*zombie)->SetActorLocation(SmoothedLocation);
			}

			// 좀비 목적지 설정
			if (tmp_path.Path1.empty() == false) {
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Update Zombie path: ZombieID=%d, PlayerID=%d, Path=(%.2f , %.2f , %.2f)"), 
				//	tmp_path.ZombieId, GameInstance->ClientSocketPtr->MyPlayerId, get<0>(*(tmp_path.Path1.begin())), get<1>(*(tmp_path.Path1.begin())), get<2>(*(tmp_path.Path1.begin()))));

				(*zombie)->NextPath[0] = *(tmp_path.Path1.begin());

				(*zombie)->afterAnim_idleDuration = 0.f;	// 서버로부터 새로운 ZombiePath를 받으면 실행 => ZombieMoveTo idle 상태 초기화 => 다시 움직이게 함!
			}
			else
				(*zombie)->NextPath[0] = std::tuple<float, float, float>();

			if (tmp_path.Path2.empty() == false)
				(*zombie)->NextPath[1] = *(tmp_path.Path2.begin());
			else
				(*zombie)->NextPath[1] = std::tuple<float, float, float>();
		}
	}

}

void APlayerCharacterController::CheckAndSendMovement(float DeltaTime)
{
	TimeSinceLastSend += DeltaTime;

	APawn* MyPawn = GetPawn();
	FVector CurrentLocation = MyPawn->GetActorLocation();
	FRotator CurrentRotation = MyPawn->GetActorRotation();

	ABaseCharacter* MyBaseCharacter = Cast<ABaseCharacter>(MyPawn);
	float hp = MyBaseCharacter->GetHP();

	bool bShouldSend = false;

	if (TimeSinceLastSend >= 0.1f || PreviousRotation != CurrentRotation || b_GetItem || PreviouHP != hp || sendRun || sendjump) {
		bShouldSend = true;
		bSentWhileStopped = false; // 뭔가 변했으니까 다시 전송 가능
	}
	else if (PreviousLocation == CurrentLocation && !bSentWhileStopped) {
		// 멈춰있는 상태지만 아직 안 보냈다면 이번에 보내기
		bShouldSend = true;
		bSentWhileStopped = true; // 한 번 보냈으니 다음 틱에 또 안 보내게
	}
	
	// 이전 위치와 현재 위치 비교 (움직임 감지)
	if (bShouldSend) {
		
		TimeSinceLastSend = 0.f;

		uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();
		uint32 MyRoomId = GameInstance->ClientSocketPtr->MyRoomId;
		MyCharacterNumber = GameInstance->GetChoicedCharacterNumber();
		std::string MyPlayerName = GameInstance->ClientSocketPtr->MyUserName;

		// Protobuf를 사용하여 TestPacket 생성
		Protocol::Character packet;
		packet.set_playerid(MyPlayerId);
		packet.set_packet_type(1); // 원하는 유형 설정
		packet.set_charactertype(MyCharacterNumber);
		packet.set_x(CurrentLocation.X);
		packet.set_y(CurrentLocation.Y);
		packet.set_z(CurrentLocation.Z);
		packet.set_pitch(CurrentRotation.Pitch);
		packet.set_yaw(CurrentRotation.Yaw);
		packet.set_roll(CurrentRotation.Roll);
		packet.set_hp(hp);
		packet.set_b_run(IsRlyRun + 1);
		packet.set_username(MyPlayerName);
		packet.set_roomnum(MyRoomId);
		packet.set_isingame(true);

		// 직렬화
		std::string serializedData;
		packet.SerializeToString(&serializedData);

		// 직렬화된 데이터를 서버로 전송
		bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());

		// 현재 위치를 이전 위치로 업데이트
		PreviousLocation = CurrentLocation;
		PreviousRotation = CurrentRotation;
		PreviouHP = hp;
		b_GetItem = false;
		sendRun = false;
	}
}

// 플레이어가 공격 서버로 전송-> 다른 클라도 해당 플레이어 공격 애니메이션 재생되게
void APlayerCharacterController::Send_Attack()
{
	if (b_attack) {
		APawn* MyPawn = GetPawn();
		ABaseCharacter* MyBaseCharacter = Cast<ABaseCharacter>(MyPawn);
		uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();

		Protocol::Character_Attack packet;
		packet.set_playerid(MyPlayerId);
		packet.set_packet_type(4);
		packet.set_attack(b_attack);
		packet.set_attacktype(m_attacktype);
		packet.set_aimoffset(MyBaseCharacter->Get_AimOffSet());

		// 직렬화
		std::string serializedData;
		packet.SerializeToString(&serializedData);

		bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());

		b_attack = false;

		//UE_LOG(LogNet, Display, TEXT("Send Attack: PlayerId=%d"), GameInstance->ClientSocketPtr->GetMyPlayerId());
	}
}

void APlayerCharacterController::Send_Equipment()
{
	if (e_NWeapon || e_TWeapon || e_BHItem || e_HItem || e_KeyItem)
	{
		APawn* MyPawn = GetPawn();
		ABaseCharacter* MyBaseCharacter = Cast<ABaseCharacter>(MyPawn);
		uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();
		uint32 ItemType; // 1 = BHItem, 2 = HItem, 3 = TWeapon, 4 = KeyItem, 5 = NWeapon

		if (e_BHItem)
		{
			ItemName = TCHAR_TO_UTF8(*(MyBaseCharacter->QuickSlot[0].Name.ToString()));
			ItemType = 1;
		}
		if (e_HItem)
		{
			ItemName = TCHAR_TO_UTF8(*(MyBaseCharacter->QuickSlot[1].Name.ToString()));
			ItemType = 2;
		}
		if (e_TWeapon)
		{
			ItemName = TCHAR_TO_UTF8(*(MyBaseCharacter->QuickSlot[2].Name.ToString()));
			ItemType = 3;
		}
		if (e_KeyItem)
		{
			ItemName = TCHAR_TO_UTF8(*(MyBaseCharacter->QuickSlot[3].Name.ToString()));
			ItemType = 4;
		}
		if (e_NWeapon)
		{
			ItemName = TCHAR_TO_UTF8(*(MyBaseCharacter->QuickSlot[4].Name.ToString()));
			ItemType = 5;
		}

		Protocol::Equip_Item packet;
		packet.set_playerid(MyPlayerId);
		packet.set_packet_type(5);
		packet.set_itemtype(ItemType);
		packet.set_itemname(ItemName);

		// 직렬화
		std::string serializedData;
		packet.SerializeToString(&serializedData);

		bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
		//UE_LOG(LogNet, Display, TEXT("Send Equip: PlayerId=%d"), GameInstance->ClientSocketPtr->GetMyPlayerId());
		e_NWeapon = false;
		e_TWeapon = false;
		e_BHItem = false; 
		e_HItem = false;
		e_KeyItem = false;
	}
}

void APlayerCharacterController::Check_run() 
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	if (b_run) {	// 뛰기 모드 ON
		basecharacter->RunWalkIcon(b_run);
		if (b_move_forward || b_move_left) {	// 플레이어 실제 이동 중

			if (basecharacter->m_bZeroStamina) {	
				// 캐릭터는 스태미너가 다 닳은 상태라 Run 상태 OFF 지만 IsRlyRun 은 아직 true 라면	-> 해당 작업을 해줘야 스태미너가 다 닳았을 때 false값 여러번 보내지 않게 막을 수 있음
				if (IsRlyRun) {
					IsRlyRun = basecharacter->IsRun();	// IsRlyRun = false;
					sendRun = true;						// 서버로 Run 상태 OFF 전송
				}
			}

			else if (basecharacter->IsRun() == false) {	// 아직 캐릭터 Run 상태 OFF 라면
				basecharacter->Run();				// 캐릭터 Run 상태 ON
				IsRlyRun = basecharacter->IsRun();	// IsRlyRun = true;
				sendRun = true;						// 서버로 Run 상태 ON 전송
			}

		}
		else {									// 플레이어 정지

			if (basecharacter->IsRun() == true) {	// 아직 캐릭터 Run 상태 ON 라면
				basecharacter->Run();				// 캐릭터 Run 상태 OFF
				IsRlyRun = basecharacter->IsRun();	// IsRlyRun = false;
				sendRun = true;						// 서버로 Run 상태 OFF 전송
			}

		}
	}
	else {			// 뛰기 모드 OFF
		basecharacter->RunWalkIcon(b_run);
		if (basecharacter->IsRun() == true) {	// 아직 캐릭터 Run 상태 ON 라면
			basecharacter->Run();				// 캐릭터 Run 상태 OFF
			IsRlyRun = basecharacter->IsRun();	// IsRlyRun = false;
			sendRun = true;						// 서버로 Run 상태 OFF 전송
		}

	}
}

//void APlayerCharacterController::Send_run() {
//	if (sendRun) {
//		uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();
//
//		Protocol::run packet;
//		packet.set_playerid(MyPlayerId);
//		packet.set_b_run(IsRlyRun);
//		packet.set_packet_type(6);
//
//		// 직렬화
//		std::string serializedData;
//		packet.SerializeToString(&serializedData);
//		bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
//
//		sendRun = false;
//	}
//}

void APlayerCharacterController::Send_jump() {
	if (sendjump) {
		uint32 MyPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();

		Protocol::jump packet;
		packet.set_playerid(MyPlayerId);
		packet.set_packet_type(7);

		// 직렬화
		std::string serializedData;
		packet.SerializeToString(&serializedData);
		bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());

		sendjump = false;
	}
}
 
void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Get the player local subsystem
	ULocalPlayer* LocalPlayer = GetLocalPlayer();

	if (LocalPlayer)
	{
		UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
		if (PlayerSubsystem)
		{            // Player subsystem is valid, continue with the setup
			PlayerSubsystem->ClearAllMappings();
			PlayerSubsystem->AddMappingContext(InputMapping, 0);


			// Bind input actions
			UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(InputComponent);
			if (PEI)
			{

				PEI->BindAction(InputActions->InputMoveForward, ETriggerEvent::Triggered, this, &APlayerCharacterController::MoveForward);
				PEI->BindAction(InputActions->InputMoveForward, ETriggerEvent::Completed, this, &APlayerCharacterController::MoveForwardStop);
				PEI->BindAction(InputActions->InputMoveLeft, ETriggerEvent::Triggered, this, &APlayerCharacterController::MoveLeft);
				PEI->BindAction(InputActions->InputMoveLeft, ETriggerEvent::Completed, this, &APlayerCharacterController::MoveLeftStop);
				PEI->BindAction(InputActions->InputTurn, ETriggerEvent::Triggered, this, &APlayerCharacterController::Turn);
				PEI->BindAction(InputActions->InputLookUp, ETriggerEvent::Triggered, this, &APlayerCharacterController::LookUp);
				PEI->BindAction(InputActions->InputRun, ETriggerEvent::Completed, this, &APlayerCharacterController::Run);
				PEI->BindAction(InputActions->InputGetItem, ETriggerEvent::Completed, this, &APlayerCharacterController::GetItem);
				PEI->BindAction(InputActions->InputLightOnOff, ETriggerEvent::Completed, this, &APlayerCharacterController::LightOnOff);
				PEI->BindAction(InputActions->InputInventoryOnOff, ETriggerEvent::Completed, this, &APlayerCharacterController::InventoryOnOff);
				PEI->BindAction(InputActions->InputJump, ETriggerEvent::Completed, this, &APlayerCharacterController::Jump);
				PEI->BindAction(InputActions->InputAttack, ETriggerEvent::Completed, this, &APlayerCharacterController::BehaviorToItem);

				PEI->BindAction(InputActions->InputQuickNWeapon, ETriggerEvent::Completed, this, &APlayerCharacterController::QuickNWeapon);
				PEI->BindAction(InputActions->InputQuickTWeapon, ETriggerEvent::Completed, this, &APlayerCharacterController::QuickTWeapon);
				PEI->BindAction(InputActions->InputQuickBHItem, ETriggerEvent::Completed, this, &APlayerCharacterController::QuickBHItem);
				PEI->BindAction(InputActions->InputQuickHItem, ETriggerEvent::Completed, this, &APlayerCharacterController::QuickHItem);
				PEI->BindAction(InputActions->InputQuickKeyItem, ETriggerEvent::Completed, this, &APlayerCharacterController::QuickKeyItem);
				PEI->BindAction(InputActions->InputChatToggle, ETriggerEvent::Completed, this, &APlayerCharacterController::ChatToggle);

				PEI->BindAction(InputActions->InputUpSTR, ETriggerEvent::Completed, this, &APlayerCharacterController::UpSTR);
				PEI->BindAction(InputActions->InputDownSTR, ETriggerEvent::Completed, this, &APlayerCharacterController::DownSTR);
				PEI->BindAction(InputActions->InputUpSpeed, ETriggerEvent::Completed, this, &APlayerCharacterController::UpSpeed);
				PEI->BindAction(InputActions->InputDownSpeed, ETriggerEvent::Completed, this, &APlayerCharacterController::DownSpeed);

				PEI->BindAction(InputActions->InputAttack_2, ETriggerEvent::Completed, this, &APlayerCharacterController::BehaviorToItem_2);
				 
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to cast InputComponent to UEnhancedInputComponent"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get EnhancedInputLocalPlayerSubsystem"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get LocalPlayer"));
	}
}

void APlayerCharacterController::MoveForward(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// 키를 여는 (애니메이션) 도중이면 플레이어의 해당 입력 막음
	if (basecharacter->IsUsingKey() == true) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Using Key - Allow no Inputs(Move_WS)")));
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// add movement 
	basecharacter->MoveForward(ForwardDirection, MovementVector.Y);

	b_move_forward = true;
}

void APlayerCharacterController::MoveForwardStop()
{
	b_move_forward = false;
}

void APlayerCharacterController::MoveLeft(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// 키를 여는 (애니메이션) 도중이면 플레이어의 해당 입력 막음
	if (basecharacter->IsUsingKey() == true) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Using Key - Allow no Inputs(Move_AD)")));
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	basecharacter->MoveLeft(RightDirection, MovementVector.Y);

	b_move_left = true;
}

void APlayerCharacterController::MoveLeftStop()
{
	b_move_left = false;
}

void APlayerCharacterController::Turn(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	basecharacter->Turn(LookAxisVector.Y);
}

void APlayerCharacterController::LookUp(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	basecharacter->LookUp(-LookAxisVector.Y);
}

void APlayerCharacterController::Run(const FInputActionValue& Value)
{
	if (!b_run)
		b_run = true;
	else
		b_run = false;
}

void APlayerCharacterController::Jump(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// 키를 여는 (애니메이션) 도중이면 플레이어의 해당 입력 막음
	if (basecharacter->IsUsingKey() == true) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Using Key - Allow no Inputs(Jump)")));
		return;
	}

	if (basecharacter->GetStamina() >= 20) {
		if (!basecharacter->GetMovementComponent()->IsFalling()) {
			basecharacter->Jump();
			basecharacter->SetStamina(basecharacter->GetStamina() - 20);
			sendjump = true;

			// 플레이어가 처음에 뛰는 키보다 jump키를 먼저 눌렀을 경우 예외상황 IF문
			if (!basecharacter->IsRun()) {

				basecharacter->HealingStamina();
			}
		}
	}
}

void APlayerCharacterController::GetItem(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	// 지혈 아이템/회복 아이템 사용 애니메이션 플레이 도중에는 아이템 먹기 입력 막기 -> 아이템 먹기로 해당 애니메이션 캔슬이 가능해서 (이렇게되면 아이템 효과가 바로 시전됨)
	if (basecharacter->m_bIsBleedHealing == true || basecharacter->m_bIsHealing == true) {
		return;
	}

	basecharacter->GetItem();
	b_GetItem = true;
}

void APlayerCharacterController::LightOnOff(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());
	basecharacter->LightOnOff();
}

void APlayerCharacterController::InventoryOnOff(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());
	basecharacter->InventoryOnOff();
}

void APlayerCharacterController::BehaviorToItem(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	if (m_bIsInputEnabled
		&& basecharacter->IsInventory() == false) {	// 인벤토리를 열고 있는 상태에서는 공격/아이템 사용 X

		m_bIsInputEnabled = false;


		// 타이머 설정 (따닥을 방지하기 위해 설정)
		GetWorld()->GetTimerManager().SetTimer(InputCoolTimeHandle, this, &APlayerCharacterController::InputCoolTime, 0.2f, false);


		if (basecharacter->IsNWHandIn() && !(basecharacter->IsAttack())) { // 아예 함수에 접근을 못하게 조건을 추가
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController Attack")));
			Attack(2);	// 가로 베기
		}
		else if (basecharacter->IsBHHandIn() && !(basecharacter->IsBHealing())) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController BH")));
			BleedHealing();
		}
		else if (basecharacter->IsHealHandIn() && !(basecharacter->IsHealing())) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController HHHHHHHHH")));

			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController HHHHHHHHH22222")));
			Healing();
		}
		else if (basecharacter->IsKeyHandIn()) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController Key")));
			PlayKey();

		}
		else if (basecharacter->IsThrowWHandIn()) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController TH")));
			Throw();
		}

	}

}

// attack_type = 1: 세로-대각 베기 / = 2: 가로 베기
void APlayerCharacterController::Attack(int attack_type)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());
	basecharacter->Attack(attack_type);
	m_attacktype = attack_type;
	b_attack = true;
	UE_LOG(LogTemp, Log, TEXT("AttackStart: %d"), GameInstance->ClientSocketPtr->GetMyPlayerId());
}

void APlayerCharacterController::Healing()
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	basecharacter->Healing();
}

void APlayerCharacterController::BleedHealing()
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	basecharacter->BleedHealing();
}

void APlayerCharacterController::PlayKey()
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	basecharacter->PlayKey();
}

void APlayerCharacterController::Throw()
{
}



void APlayerCharacterController::QuickNWeapon(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("QuickNWeapon"));

	basecharacter->QuickNWeapon();

	e_NWeapon = true;
}

void APlayerCharacterController::QuickTWeapon(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("QuickTWeapon"));

	basecharacter->QuickTWeapon();

	e_TWeapon = true;
}

void APlayerCharacterController::QuickBHItem(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("QuickBHItem"));

	basecharacter->QuickBHItem();

	e_BHItem = true;

}

void APlayerCharacterController::QuickHItem(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("QuickHItem"));

	basecharacter->QuickHItem();

	e_HItem = true;
}

void APlayerCharacterController::QuickKeyItem(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("QuickKeyItem"));

	basecharacter->QuickKeyItem();

	e_KeyItem = true;
}

void APlayerCharacterController::ChatToggle(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("ChatToggle"));

	bool bIsChatVisible = basecharacter->GameChatUIWidget->IsChatVisible();

	if (bIsChatVisible)
	{
		// 채팅창이 열려 있으면 닫기
		basecharacter->GameChatUIWidget->ToggleChatVisibility(false);
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
	else
	{
		// 채팅창이 닫혀 있으면 열기
		basecharacter->GameChatUIWidget->ToggleChatVisibility(true);
		basecharacter->GameChatUIWidget->SetKeyboardFocus();
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void APlayerCharacterController::UpSTR(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("UpSTR"));

	basecharacter->UpSTR();
}

void APlayerCharacterController::DownSTR(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("DownSTR"));

	basecharacter->DownSTR();
}

void APlayerCharacterController::UpSpeed(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("UpSpeed"));

	basecharacter->UpSpeed();
}

void APlayerCharacterController::DownSpeed(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	UE_LOG(LogTemp, Log, TEXT("DownSpeed"));

	basecharacter->DownSpeed();
}

void APlayerCharacterController::BehaviorToItem_2(const FInputActionValue& Value)
{
	ABaseCharacter* basecharacter = Cast<ABaseCharacter>(GetCharacter());

	if (m_bIsInputEnabled
		&& basecharacter->IsInventory() == false) {	// 인벤토리를 열고 있는 상태에서는 공격/아이템 사용 X) 

		m_bIsInputEnabled = false;


		// 타이머 설정 (따닥을 방지하기 위해 설정)
		GetWorld()->GetTimerManager().SetTimer(InputCoolTimeHandle, this, &APlayerCharacterController::InputCoolTime, 0.2f, false);


		if (basecharacter->IsNWHandIn() && !(basecharacter->IsAttack())) { // 아예 함수에 접근을 못하게 조건을 추가
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("PlayerController Attack")));
			Attack(1);	// 세로 베기
		}

	}

}

void APlayerCharacterController::InputCoolTime()
{
	m_bIsInputEnabled = true;
}

void APlayerCharacterController::DisabledControllerInput()
{
	DisableInput(this);
}

//void APlayerCharacterController::ServerHandleAttack()
//{
//	ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(GetCharacter());
//	if (BaseCharacter)
//	{
//		BaseCharacter->SetAttack(true, ???);
//	}
//}
