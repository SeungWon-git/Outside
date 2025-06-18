// Fill out your copyright notice in the Description page of Project Settings.


#include "ProZombie/RunningZombie.h"
#include "ProZombie/RunningZombieAIController.h"
#include "ProZombie/ZombieAnimInstance.h"

ARunningZombie::ARunningZombie()
{

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.f), FRotator(0.f, -90.f, 0.f));

	//CopyStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopyStaticMeshR"));
	// 위에 FObjectFinder는 기존것 밑에는 새롭게 애니메이션한 부분
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/ZombieAsset/RunningZombie/RunningZombie.RunningZombie"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANNEQUIN(TEXT("/Game/ZombieAsset/TRunningZombie/RunningZombie.RunningZombie"));
	if (SK_MANNEQUIN.Succeeded()) {
		GetMesh()->SetSkeletalMesh(SK_MANNEQUIN.Object);
	}

	//static ConstructorHelpers::FClassFinder<UAnimInstance> ZOMBIE_ANIM(TEXT("/Game/ZombieAsset/Animation/RunningZombieAnimation/BP_RunningZombieAnimBlueprint.BP_RunningZombieAnimBlueprint_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> ZOMBIE_ANIM(TEXT("/Game/ZombieAsset/TAnimation/RunningZombieAnimation/BP_RunningZombieAnimBlueprintT.BP_RunningZombieAnimBlueprintT_C"));
	if (ZOMBIE_ANIM.Succeeded()) {
		GetMesh()->SetAnimInstanceClass(ZOMBIE_ANIM.Class);
	}
	
	//ConstructorHelpers::FObjectFinder<UStaticMesh> SM_RUNNINGSTATIC(TEXT("/Game/ZombieAsset/RunningZombie/RunningZombieStaticMesh.RunningZombieStaticMesh"));
	//if (SM_RUNNINGSTATIC.Succeeded()) {
	//	CopyStaticMesh->SetStaticMesh(SM_RUNNINGSTATIC.Object);
	//}


	SetHP(RunningZombieStartHP);
	m_fHP_Prev = RunningZombieStartHP;
	SetStartHP(RunningZombieStartHP);
	float speed = RunningZombieSpeed;
	SetSpeed(speed);
	GetCharacterMovement()->MaxWalkSpeed = RunningZombieSpeed;
	SetSTR(FMath::RandRange(4, 8)); 
	SetSpecialAbility(true);
	SetZombieName("RunningZombie");
	SetTurningSpeed(GetTurningSpeed());
	targetType = TARGET::PATROL;

	// 좀비 크기(scale) 재설정
	float zombieScale = 1.02f;
	SetActorScale3D(FVector(zombieScale, zombieScale, zombieScale));
	// 기존 캡슐 크기를 가져오기
	float CurrentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CurrentHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//  캡슐 크기도 동일한 비율로 변경
	GetCapsuleComponent()->SetCapsuleSize(CurrentRadius * zombieScale, CurrentHalfHeight * zombieScale);

	// Scale로 키 변경에 따른 공중 뜨기 방지
	FVector original_Offset = GetMesh()->GetRelativeLocation();
	RelativeHeight_ZOffset = -4.f;
	GetMesh()->SetRelativeLocation(FVector(original_Offset.X, original_Offset.Y, original_Offset.Z + RelativeHeight_ZOffset));
}

void ARunningZombie::BeginPlay()
{
	Super::BeginPlay();

	CachedAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
}

void ARunningZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//auto CharactorAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	//if (nullptr != CharactorAnimInstance) {
	//	CharactorAnimInstance->SetCurrentPawnSpeed(GetVelocity().Size());
	//}
}


void ARunningZombie::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}


