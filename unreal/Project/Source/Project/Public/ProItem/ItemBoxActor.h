// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "ProData/ItemDataStructure.h"
#include "ItemBoxActor.generated.h"

class ANormalWeaponActor;
class ABaseCharacter;

UCLASS()	// 월드에 스폰되는 아이템 박스 클래스
class PROJECT_API AItemBoxActor : public AActor	
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBoxActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnChracterOvelapNew(ABaseCharacter* character);  // 이름 수정 필요

	void SetItemBoxId(uint32 index);

	uint32 GetItemBoxId();

	//void SetInBoxItemId(FString itemid) { m_sItemID = itemid; };
	//FString GetInBoxItemId() { return m_sItemID; }

public:
	UPROPERTY(EditAnywhere, Category = Box)
	UBoxComponent* Trigger;

	UPROPERTY(EditAnywhere, Category = Box)
	UStaticMeshComponent* Box;

	//// 수정 필요	
	//UPROPERTY(EditAnywhere, Category = Box)
	//TSubclassOf<ANormalWeaponActor> NormalWeaponItemClass;


	//UPROPERTY(EditAnywhere, Category = Item)
	//FString m_sItemID;

	// 아이템 이름
	UPROPERTY(EditAnywhere, Category = item)
	FName ItemName;

	// 아이템 종류(클래스가 뭔지)
	UPROPERTY(EditAnywhere, Category = item)
	EItemClass ItemClassType;

	// 텍스쳐
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Texture;

	// 들고있는 갯수
	UPROPERTY(EditAnywhere, Category = item)
	int Count;

	// 현재 무기 내구도
	UPROPERTY(EditAnywhere, Category = item)
	float Durability = 0;

	// 원래 무기 내구도
	UPROPERTY(EditAnywhere, Category = item)
	float Durability_Max = 0;

public:
	uint32 ItemBoxId = 0;	// 인덱스 +1 값! (패킷 통신 0 최적화 피하려고) => 그래서 패킷으로 보낼때 이걸로 보냄

};
