// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructure.generated.h"

/**
 * 
 */

// Inventory Slot에 들어있는 속성들(구조체,Enum)이 들어있는 .h

class USlot;

class PROJECT_API ItemDataStructure
{
public:
	ItemDataStructure();
	~ItemDataStructure();
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	ITEM_NONE				UMETA(DisplayName = "None"),
	ITEM_USEABLE			UMETA(DisplayName = "Useable"),
	ITEM_EQUIPMENT			UMETA(DisplayName = "Equipment"),
	ITEM_QUICK_NONE			UMETA(DisplayName = "QuickNone"),
	ITEM_QUICK_EQUIPMENT	UMETA(DisplayName = "QuickEquipment"),
};

UENUM(BlueprintType)
enum class EItemClass : uint8
{
	NORMALWEAPON			UMETA(DisplayName = "NormalWeapon"),
	THROWINGWEAPON			UMETA(DisplayName = "ThrowingWeapon"),
	HEALINGITEM				UMETA(DisplayName = "HealingItem"),
	BLEEDINGHEALINGITEM		UMETA(DisplayName = "BleedingHealingItem"),
	KEYITEM					UMETA(DisplayName = "KeyItem"),
	BAGITEM					UMETA(DisplayName = "BagItem"),
	NONE					UMETA(DisplayName = "None"),
};

USTRUCT(Atomic, BlueprintType)		// (인벤토리)슬롯에 사용되는 구조체
struct FItemDataStructure
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) // 무슨 아이템인진 힐링인지 노말무기인지, 투척무기인지
	EItemClass ItemClassType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) // 퀵슬롯만 사용 -> 장착중인 템이 인벤토리 어디 인덱스 아이템인지 기억
	int SlotReference = -1;	// 초기값 - 슬롯 테두리 표시 때문에 초기값 설정해줘야 함 -> 안 그럼 인벤 0번은 항상 테두리 표시함 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	// 현재 무기 내구도 저장
	float Durability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	// 원래 무기 내구도 저장
	float Durability_Max = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	// 인벤에서 획득한 아이템 원래 월드에서 아이템 박스 인덱스 몇번 템이였는지 기억 (참고로 1부터 시작함 - 패킷 통신에서 0 최적화 피하려고)
	int ItemBoxOriginalId = -1;
};

UENUM(BlueprintType)
enum class ESlotType : uint8
{
	SLOT_NONE				UMETA(DisplayName = "None"),
	SLOT_ITEM				UMETA(DisplayName = "Item"),
	SLOT_QUICK				UMETA(DisplayName = "Quick"),
	SLOT_QUICK_ITEM			UMETA(DisplayName = "QuickItem"),
	SLOT_PICK_UP			UMETA(DisplayName = "PickUpItem"),
	SLOT_OTHER_PLAYER		UMETA(DisplayName = "OtherPlayerItem"),
};

