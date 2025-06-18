// Fill out your copyright notice in the Description page of Project Settings.


#include "ProCharacter/FireFighterCharacter.h"

#include "ProCharacter/PlayerCharacterAnimInstance.h"
#include "ProCharacter/PlayerCharacterController.h"
#include "ProItem/HealingItemActor.h"

AFireFighterCharacter::AFireFighterCharacter()
{
	// 위에 FObjectFinder는 기존것 밑에는 새롭게 애니메이션한 부분
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/FireFighter/FireFighter_UE.FireFighter_UE"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/CharacterAsset/JFireFighter/FireFighter.FireFighter"));
	
	if (SK_MANNEQUIN.Succeeded()) {
		GetMesh()->SetSkeletalMesh(SK_MANNEQUIN.Object);
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	//static ConstructorHelpers::FClassFinder<UAnimInstance> CHARACTER_ANIM(TEXT("/Game/CharacterAsset/Animation/BP_EmployeeAnimBlueprint.BP_EmployeeAnimBlueprint_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> CHARACTER_ANIM(TEXT("/Game/CharacterAsset/JAnimation/BP_EmployeeAnimBlueprintJ.BP_EmployeeAnimBlueprintJ_C"));

	if (CHARACTER_ANIM.Succeeded()) {
		GetMesh()->SetAnimInstanceClass(CHARACTER_ANIM.Class);
	}

	SetHP(60);
	SetMaxHP(60);
	SetBasicSpeed(5);
	SetSTR(7);
	SetStamina(100);
	SetMaxStamina(100);
	SetStaminaHealing(5);
	SetCharacterName("FireFighterCharacter");

	// 캐릭터마다 크기(scale) 재설정
	float characterScale = 1.07f;
	SetActorScale3D(FVector(characterScale, characterScale, characterScale + 0.01f));	// 키만 좀 길쭉하게
	// 기존 캡슐 크기를 가져오기
	float CurrentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CurrentHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//  캡슐 크기도 동일한 비율로 변경
	GetCapsuleComponent()->SetCapsuleSize(CurrentRadius * characterScale, CurrentHalfHeight * (characterScale + 0.01f));

	// Scale로 키 변경에 따른 공중 뜨기 방지
	FVector original_Offset = GetMesh()->GetRelativeLocation();
	RelativeHeight_ZOffset = -10.f;
	GetMesh()->SetRelativeLocation(FVector(original_Offset.X, original_Offset.Y, original_Offset.Z + RelativeHeight_ZOffset));
}

void AFireFighterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SmokeTimer();
}

void AFireFighterCharacter::SmokeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(SmokeHandle, this, &AFireFighterCharacter::LimitSmoking, 60.0f, false);	// 1분후 타이머 실행 (담배를 안 피울 시 체력 금단현상 시작)
}

void AFireFighterCharacter::LimitSmoking()
{
	if (GetPlayerId() == 99) {	// 다른 클라가 소방관이면 해당 타이머 작동 시키지 X
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("SmokeTimer ON -> My Client is a FireFighter (PlayerId: 99)")));
		//UE_LOG(LogTemp, Log, TEXT("SmokeTimer ON -> My Client is a FireFighter (PlayerId: 99)"));

		LimitSmokingIcon();
		float str = GetSTR() - 1;
		if (str <= 0)	// 마이너스 값이 되서 좀비 체력을 회복 시키지 않도록
			str = 0;
		SetSTR(str);
		float speed = GetBasicSpeed() - 1;
		if (speed <= 0)	// 마이너스 값이 되지 않도록
			speed = 0;
		SetBasicSpeed(speed);
		GetWorld()->GetTimerManager().SetTimer(DyingHandle, this, &AFireFighterCharacter::NoSmokeIsDying, 2.0f, true, 1.0f);	// 2초 간격으로 체력 빠짐
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("SmokeTimer OFF -> Other Client is the FireFighter")));
		//UE_LOG(LogTemp, Log, TEXT("SmokeTimer OFF -> Other Client is the FireFighter"));

		GetWorld()->GetTimerManager().ClearTimer(SmokeHandle);
	}
}

void AFireFighterCharacter::NoSmokeIsDying()
{
	if (GetPlayerId() == 99) {	// 다른 클라가 소방관이면 해당 타이머 작동 시키지 X
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("NoSmokeIsDyingTimer ON -> My Client is a FireFighter (PlayerId: 99)")));
		//UE_LOG(LogTemp, Log, TEXT("NoSmokeIsDyingTimer ON -> My Client is a FireFighter (PlayerId: 99)"));

		SetHP(GetHP() - 1);
		if (GetHP() <= 0) {
			if (!IsDeadPlay()) {
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("NoSmokeIsDying -> PlayDead!")));
				//UE_LOG(LogTemp, Log, TEXT("NoSmokeIsDying -> PlayDead!"));
				SetDeadPlay(true);
				GetWorld()->GetTimerManager().ClearTimer(DyingHandle);

				if (IsDeadPlay() && !IsDead()) {
					PlayDead();
				}
			}
			else if (IsDeadPlay()) {
				GetWorld()->GetTimerManager().ClearTimer(DyingHandle);
			}
		}

	}
}

void AFireFighterCharacter::Smoking(AHealingItemActor* smokeActor)
{
	// 소방관 캐릭 특성
	if (smokeActor != nullptr) {
		float originalSmoke_healingSpeed = 1.0f;
		smokeActor->m_fHealingSpeed = originalSmoke_healingSpeed + 1.6f;	// 소방관 담배로 인한 체력 회복 +160% 효력 발휘
	}

	SmokingIcon();
	GetWorld()->GetTimerManager().ClearTimer(SmokeHandle);
	GetWorld()->GetTimerManager().ClearTimer(DyingHandle);
	SetSTR(GetSTR() + 1);
	SetBasicSpeed(GetBasicSpeed() + 1);
	SmokeTimer();
}

void AFireFighterCharacter::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);
}
