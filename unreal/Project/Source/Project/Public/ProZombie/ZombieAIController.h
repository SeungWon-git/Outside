// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ClientSocket.h"
#include "BaseZombie.h"
#include "ProCharacter/BaseCharacter.h"
#include "ZombieAIController.generated.h"



/**
 * 
 */
// AI 추가되면 수정될 것 같은 것!!
UCLASS()
class PROJECT_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieAIController();

	static const FName TargetKey;
	static const FName StartLocationKey;
	static const FName PatrolLocationKey;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void Send_Detected();

	void Send_PlayerLost();


	void SetStartLocationValue(FVector startlocation);
	void SetPatrolLocationValue(FVector patrollocation);
	

	void UpdateLastKnownPositionByFootSound(FVector playerlocation);

	void UpdateLastKnownPositionByShoutingSound(FVector playerlocation);

	
	void ZombieMoveTo(float deltasecond, int& indx);

	void ZombieTurn(float deltasecond, int& indx);

	UProGameInstance* GameInstance;

public:
	UPROPERTY(EditAnywhere)
	bool m_bFootSound;
	
	UPROPERTY(EditAnywhere)
	FVector m_vFootCharacterPos;
	
	UPROPERTY(EditAnywhere)
	FVector m_vShoutingPos;
	
	UPROPERTY(EditAnywhere)
	bool m_bShoutingSound;


	//ZombieData recvZombieData;
	//ZombiePath recvZombiePath;
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

	float PreviousHp = 0.f;

	float idleDuration = 0.f;
	float lookingaround_idleDuration = 0.f;

};
