// Fill out your copyright notice in the Description page of Project Settings.


#include "ProCharacter/EmployeeCharacter.h"

AEmployeeCharacter::AEmployeeCharacter()
{
	// 위에 FObjectFinder는 기존것 밑에는 새롭게 애니메이션한 부분
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/Employee/EmployeeCharacter_UE.EmployeeCharacter_UE"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/JEmployee/Employee.Employee"));

	if (SK_MANNEQUIN.Succeeded()) {
		GetMesh()->SetSkeletalMesh(SK_MANNEQUIN.Object);
	}
	
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	//static ConstructorHelpers::FClassFinder<UAnimInstance> CHARACTER_ANIM(TEXT("/Game/CharacterAsset/Animation/BP_EmployeeAnimBlueprint.BP_EmployeeAnimBlueprint_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> CHARACTER_ANIM(TEXT("/Game/CharacterAsset/JAnimation/BP_EmployeeAnimBlueprintJ.BP_EmployeeAnimBlueprintJ_C"));

	if (CHARACTER_ANIM.Succeeded()) {
		GetMesh()->SetAnimInstanceClass(CHARACTER_ANIM.Class);
	}

	SetHP(50);
	SetMaxHP(50);
	SetBasicSpeed(4);
	SetSTR(4);
	SetStamina(100 + 30);
	SetMaxStamina(100 + 30);
	SetStaminaHealing(5);
	SetCharacterName("EmployeeCharacter");

	// 캐릭터마다 크기(scale) 재설정
	float characterScale = 1.065f;
	SetActorScale3D(FVector(characterScale, characterScale, characterScale));
	// 기존 캡슐 크기를 가져오기
	float CurrentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CurrentHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//  캡슐 크기도 동일한 비율로 변경
	GetCapsuleComponent()->SetCapsuleSize(CurrentRadius * characterScale, CurrentHalfHeight * characterScale);

	// Scale로 키 변경에 따른 공중 뜨기 방지
	FVector original_Offset = GetMesh()->GetRelativeLocation();
	RelativeHeight_ZOffset = -9.f;
	GetMesh()->SetRelativeLocation(FVector(original_Offset.X, original_Offset.Y, original_Offset.Z + RelativeHeight_ZOffset));
}

void AEmployeeCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 회사원 캐릭 시작시 아이템 (회사원 캐릭 특성)
	if (Inventory.Num() > 0) {
		Inventory[0].Type = EItemType::ITEM_USEABLE;
		Inventory[0].Name = "SuitCase";
		Inventory[0].ItemClassType = EItemClass::BAGITEM;
		Inventory[0].Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/InvenPng/InvenSuitCase.InvenSuitCase"));
		Inventory[0].Count = 1;
		
		GameUIUpdate();
	}	// 이니셜라이저에 넣으니 초기화 안된 메모리 엑세스 에러 발생해서 여기에서 


}

void AEmployeeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
