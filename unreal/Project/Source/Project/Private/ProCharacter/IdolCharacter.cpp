// Fill out your copyright notice in the Description page of Project Settings.


#include "ProCharacter/IdolCharacter.h"

AIdolCharacter::AIdolCharacter()
{
	// 위에 FObjectFinder는 기존것 밑에는 새롭게 애니메이션한 부분
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/Idol/IdolCharacter_UE.IdolCharacter_UE"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/JIdol/Idol.Idol"));
	
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
	SetBasicSpeed(5);
	SetSTR(5);
	SetStamina(100);
	SetMaxStamina(100);
	SetStaminaHealing(5);
	SetCharacterName("IdolCharacter");

	// 캐릭터마다 크기(scale) 재설정
	float characterScale = 1.03f;
	SetActorScale3D(FVector(characterScale, characterScale, characterScale));
	// 기존 캡슐 크기를 가져오기
	float CurrentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CurrentHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//  캡슐 크기도 동일한 비율로 변경
	GetCapsuleComponent()->SetCapsuleSize(CurrentRadius * characterScale, CurrentHalfHeight * characterScale);

	// Scale로 키 변경에 따른 공중 뜨기 방지
	FVector original_Offset = GetMesh()->GetRelativeLocation();	
	RelativeHeight_ZOffset = -5.f;
	GetMesh()->SetRelativeLocation(FVector(original_Offset.X, original_Offset.Y, original_Offset.Z + RelativeHeight_ZOffset));
}

void AIdolCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_fBleedPercent += 0.2f;	// 아이돌 캐릭 특성 - 출혈 확률: (기존)30% + (추가)20% = 50% 
}

void AIdolCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}