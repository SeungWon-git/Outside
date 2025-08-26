// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ClientSocket.h"
#include "ProZombie/BaseZombie.h"
#include "ProCharacter/BaseCharacter.h"
#include "ShoutingZombieAIController.generated.h"

// AI 추가되면 수정될 것 같은 것!!

/**
 * 
 */
 // ShoutingZombie(소리치는 좀비)를 컨트롤하는 컨트롤러
UCLASS()
class PROJECT_API AShoutingZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AShoutingZombieAIController();

	static const FName TargetKey;
	static const FName StartLocationKey;
	static const FName PatrolLocationKey;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	void Send_Detected();

	void Send_PlayerLost();
	
	//void CheckAndSendMovement();

	//void SetStartLocationValue(FVector startlocation);
	
	//void SetPatrolLocationValue(FVector patrollocation);

	void UpdateLastKnownPositionByFootSound(FVector playerlocation);

	void ZombieMoveTo(float deltasecond, int& indx);

	void ZombieTurn(float deltasecond, int& indx);

	//void StopAI();
	//void StartAI();

	UProGameInstance* GameInstance;

private:
	//UPROPERTY(EditAnywhere)
	//class UBehaviorTree* ShoutingZombieAIBehavior;

	//UPROPERTY(EditAnywhere)

	//class UBlackboardData* ShoutingZombieBlackBoardAsset;

public:
	UPROPERTY(EditAnywhere)
	bool m_bFootSound;
	
	UPROPERTY(EditAnywhere)
	FVector m_vFootCharacterPos;
	
	UPROPERTY(EditAnywhere)
	FVector m_vShoutingPos;
	
	UPROPERTY(EditAnywhere)
	bool m_bShoutingSound;


	ZombieHP recvZombieHP;

	uint32 Zombie_Id = 9999;
	//uint32 AttackZombieId;
	//FVector NewLocation;
	//FVector PreviousLocation;
	//FRotator PreviousRotation;

	ABaseCharacter* LastSeenPlayer;

	UPROPERTY(EditAnywhere)
	ABaseZombie* OwnerZombie;

	UPROPERTY(EditAnywhere)
	uint32 attackPlayerID = 9999;	// 공격시에 바라 볼 플레이어 (공격할 대상)

	UPROPERTY(EditAnywhere)
	uint32 shoutingTo_PlayerId = 9999; // 샤우팅 시에 바라 볼 플레이어 (샤우팅에 걸린 플레이어)

	float PreviousHp = 0.f;

	float idleDuration = 0.f;
	float lookingaround_idleDuration = 0.f;

};