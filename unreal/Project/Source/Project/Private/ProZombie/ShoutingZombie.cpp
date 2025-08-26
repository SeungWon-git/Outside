// Fill out your copyright notice in the Description page of Project Settings.


#include "ProZombie/ShoutingZombie.h"
#include "ProZombie/ShoutingZombieAIController.h"
#include "ProZombie/ZombieAnimInstance.h"

AShoutingZombie::AShoutingZombie()
{

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.f), FRotator(0.f, -90.f, 0.f));

	//CopyStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopyStaticMeshS"));
	// 위에 FObjectFinder는 기존것 밑에는 새롭게 애니메이션한 부분
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/ZombieAsset/ShoutingZombie/ShoutingZombie.ShoutingZombie"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/ZombieAsset/TShoutingZombie/ShoutingZombie.ShoutingZombie"));
	if (SK_MANNEQUIN.Succeeded()) {
		GetMesh()->SetSkeletalMesh(SK_MANNEQUIN.Object);
	}

	//static ConstructorHelpers::FClassFinder<UAnimInstance> ZOMBIE_ANIM(TEXT("/Game/ZombieAsset/Animation/ShoutingAnimation/BP_ShoutingZombieAnimBlueprint.BP_ShoutingZombieAnimBlueprint_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> ZOMBIE_ANIM(TEXT("/Game/ZombieAsset/TAnimation/ShoutingAnimation/BP_ShoutingZombieAnimBlueprint.BP_ShoutingZombieAnimBlueprint_C"));
	if (ZOMBIE_ANIM.Succeeded()) {
		GetMesh()->SetAnimInstanceClass(ZOMBIE_ANIM.Class);
	}

	//ConstructorHelpers::FObjectFinder<UStaticMesh> SM_SHOUTINGSTATIC(TEXT("/Game/ZombieAsset/ShoutingZombie/ShoutingZombieStaticMesh.ShoutingZombieStaticMesh"));
	//if (SM_SHOUTINGSTATIC.Succeeded()) {
	//	CopyStaticMesh->SetStaticMesh(SM_SHOUTINGSTATIC.Object);
	//}


	SetHP(ShoutingZombieStartHP);
	m_fHP_Prev = ShoutingZombieStartHP;
	SetStartHP(ShoutingZombieStartHP);
	float speed = ShoutingZombieSpeed;
	SetSpeed(speed);
	GetCharacterMovement()->MaxWalkSpeed = ShoutingZombieSpeed;
	SetSTR(FMath::RandRange(5, 10)); 
	SetSpecialAbility(true);
	SetZombieName("ShoutingZombie");
	SetTurningSpeed(GetTurningSpeed());
	targetType = TARGET::PATROL;

	// 좀비 크기(scale) 재설정
	float zombieScale = 1.06f;
	SetActorScale3D(FVector(zombieScale, zombieScale, zombieScale));
	// 기존 캡슐 크기를 가져오기
	float CurrentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CurrentHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//  캡슐 크기도 동일한 비율로 변경
	GetCapsuleComponent()->SetCapsuleSize(CurrentRadius * zombieScale, CurrentHalfHeight * zombieScale);

	// Scale로 키 변경에 따른 공중 뜨기 방지
	FVector original_Offset = GetMesh()->GetRelativeLocation();
	RelativeHeight_ZOffset = -8.5f;
	GetMesh()->SetRelativeLocation(FVector(original_Offset.X, original_Offset.Y, original_Offset.Z + RelativeHeight_ZOffset));
}

void AShoutingZombie::BeginPlay()
{
	Super::BeginPlay();
}

void AShoutingZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//auto CharactorAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	//if (nullptr != CharactorAnimInstance) {
	//	CharactorAnimInstance->SetCurrentPawnSpeed(GetVelocity().Size());
	//}
}


void AShoutingZombie::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}