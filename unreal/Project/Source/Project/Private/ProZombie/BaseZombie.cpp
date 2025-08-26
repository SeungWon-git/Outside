// Fill out your copyright notice in the Description page of Project Settings.


#include "ProZombie/BaseZombie.h"

#include "Engine/DamageEvents.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProZombie/ZombieAIController.h"
#include "ProZombie/RunningZombieAIController.h"
#include "ProZombie/ShoutingZombieAIController.h"
#include "Engine/SkeletalMesh.h"
//#include "ProItem/NormalWeaponActor.h"

#include "ProNiagaFX/BloodNiagaEffect.h"
#include "ProNiagaFX/ShoutingNiagaEffect.h"
#include "ProNiagaFX/ResurrectNiagaEffect.h"
#include "ProNiagaFX/HealingNiagaEffect.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "ProceduralMeshComponent.h"

#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "HAL/PlatformTime.h"


// Sets default values
ABaseZombie::ABaseZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.f, 0.0f);
	GetCapsuleComponent()->SetCollisionProfileName("ZombieCol");
	GetMesh()->SetCollisionProfileName("Zombie");
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 400.f;

	ShoutingFX = nullptr;
	
	HealingFX = nullptr;

	ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("/Game/Mesh/SlicedBloodMaterial.SlicedBloodMaterial"));
	if (MaterialFinder.Succeeded())
	{
		Material_Blood = MaterialFinder.Object;
	}

	ConstructorHelpers::FObjectFinder<UMaterial> MaterialProcMeshFinder(TEXT("/Game/Mesh/wine_bottles_01_wine_red.wine_bottles_01_wine_red"));
	if (MaterialProcMeshFinder.Succeeded())
	{
		Material_ProcMeshBlood = MaterialProcMeshFinder.Object;
	}

	
	NextPath[0] = { -100000.f, -100000.f, -100000.f };	// 더미값 지정
	NextPath[1] = { -100000.f, -100000.f, -100000.f };


}

void ABaseZombie::InitializeBoneHierarchy()
{

	RootBone = MakeShared<FZBoneStructure>(TEXT("Hips"));

	// Left Leg 계열
	TSharedPtr<FZBoneStructure> LeftUpLeg = MakeShared<FZBoneStructure>(TEXT("LeftUpLeg"));
	TSharedPtr<FZBoneStructure> LeftLeg = MakeShared<FZBoneStructure>(TEXT("LeftLeg"));
	TSharedPtr<FZBoneStructure> LeftFoot = MakeShared<FZBoneStructure>(TEXT("LeftFoot"));
	TSharedPtr<FZBoneStructure> LeftToeBase = MakeShared<FZBoneStructure>(TEXT("LeftToeBase"));

	// Right Leg 계열
	TSharedPtr<FZBoneStructure> RightUpLeg = MakeShared<FZBoneStructure>(TEXT("RightUpLeg"));
	TSharedPtr<FZBoneStructure> RightLeg = MakeShared<FZBoneStructure>(TEXT("RightLeg"));
	TSharedPtr<FZBoneStructure> RightFoot = MakeShared<FZBoneStructure>(TEXT("RightFoot"));
	TSharedPtr<FZBoneStructure> RightToeBase = MakeShared<FZBoneStructure>(TEXT("RightToeBase"));

	// Spine 계열
	TSharedPtr<FZBoneStructure> Spine = MakeShared<FZBoneStructure>(TEXT("Spine"));
	TSharedPtr<FZBoneStructure> Spine1 = MakeShared<FZBoneStructure>(TEXT("Spine1"));
	TSharedPtr<FZBoneStructure> Spine2 = MakeShared<FZBoneStructure>(TEXT("Spine2"));

	// Left Arm 계열
	TSharedPtr<FZBoneStructure> LeftShoulder = MakeShared<FZBoneStructure>(TEXT("LeftShoulder"));
	TSharedPtr<FZBoneStructure> LeftArm = MakeShared<FZBoneStructure>(TEXT("LeftArm"));
	TSharedPtr<FZBoneStructure> LeftForeArm = MakeShared<FZBoneStructure>(TEXT("LeftForeArm"));
	TSharedPtr<FZBoneStructure> LeftHand = MakeShared<FZBoneStructure>(TEXT("LeftHand"));

	TSharedPtr<FZBoneStructure> LeftHandIndex1 = MakeShared<FZBoneStructure>(TEXT("LeftHandIndex1"));
	TSharedPtr<FZBoneStructure> LeftHandIndex2 = MakeShared<FZBoneStructure>(TEXT("LeftHandIndex2"));
	TSharedPtr<FZBoneStructure> LeftHandIndex3 = MakeShared<FZBoneStructure>(TEXT("LeftHandIndex3"));

	TSharedPtr<FZBoneStructure> LeftHandMiddle1 = MakeShared<FZBoneStructure>(TEXT("LeftHandMiddle1"));
	TSharedPtr<FZBoneStructure> LeftHandMiddle2 = MakeShared<FZBoneStructure>(TEXT("LeftHandMiddle2"));
	TSharedPtr<FZBoneStructure> LeftHandMiddle3 = MakeShared<FZBoneStructure>(TEXT("LeftHandMiddle3"));

	TSharedPtr<FZBoneStructure> LeftHandPinky1 = MakeShared<FZBoneStructure>(TEXT("LeftHandPinky1"));
	TSharedPtr<FZBoneStructure> LeftHandPinky2 = MakeShared<FZBoneStructure>(TEXT("LeftHandPinky2"));
	TSharedPtr<FZBoneStructure> LeftHandPinky3 = MakeShared<FZBoneStructure>(TEXT("LeftHandPinky3"));

	TSharedPtr<FZBoneStructure> LeftHandRing1 = MakeShared<FZBoneStructure>(TEXT("LeftHandRing1"));
	TSharedPtr<FZBoneStructure> LeftHandRing2 = MakeShared<FZBoneStructure>(TEXT("LeftHandRing2"));
	TSharedPtr<FZBoneStructure> LeftHandRing3 = MakeShared<FZBoneStructure>(TEXT("LeftHandRing3"));

	TSharedPtr<FZBoneStructure> LeftHandThumb1 = MakeShared<FZBoneStructure>(TEXT("LeftHandThumb1"));
	TSharedPtr<FZBoneStructure> LeftHandThumb2 = MakeShared<FZBoneStructure>(TEXT("LeftHandThumb2"));
	TSharedPtr<FZBoneStructure> LeftHandThumb3 = MakeShared<FZBoneStructure>(TEXT("LeftHandThumb3"));

	//Neck계열
	TSharedPtr<FZBoneStructure> Neck = MakeShared<FZBoneStructure>(TEXT("Neck"));
	TSharedPtr<FZBoneStructure> Head = MakeShared<FZBoneStructure>(TEXT("Head"));
	TSharedPtr<FZBoneStructure> LeftEye = MakeShared<FZBoneStructure>(TEXT("LeftEye"));
	TSharedPtr<FZBoneStructure> RightEye = MakeShared<FZBoneStructure>(TEXT("RightEye"));

	// Right Arm 계열
	TSharedPtr<FZBoneStructure> RightShoulder = MakeShared<FZBoneStructure>(TEXT("RightShoulder"));
	TSharedPtr<FZBoneStructure> RightArm = MakeShared<FZBoneStructure>(TEXT("RightArm"));
	TSharedPtr<FZBoneStructure> RightForeArm = MakeShared<FZBoneStructure>(TEXT("RightForeArm"));
	TSharedPtr<FZBoneStructure> RightHand = MakeShared<FZBoneStructure>(TEXT("RightHand"));

	TSharedPtr<FZBoneStructure> RightHandIndex1 = MakeShared<FZBoneStructure>(TEXT("RightHandIndex1"));
	TSharedPtr<FZBoneStructure> RightHandIndex2 = MakeShared<FZBoneStructure>(TEXT("RightHandIndex2"));
	TSharedPtr<FZBoneStructure> RightHandIndex3 = MakeShared<FZBoneStructure>(TEXT("RightHandIndex3"));
								
	TSharedPtr<FZBoneStructure> RightHandMiddle1 = MakeShared<FZBoneStructure>(TEXT("RightHandMiddle1"));
	TSharedPtr<FZBoneStructure> RightHandMiddle2 = MakeShared<FZBoneStructure>(TEXT("RightHandMiddle2"));
	TSharedPtr<FZBoneStructure> RightHandMiddle3 = MakeShared<FZBoneStructure>(TEXT("RightHandMiddle3"));
								
	TSharedPtr<FZBoneStructure> RightHandPinky1 = MakeShared<FZBoneStructure>(TEXT("RightHandPinky1"));
	TSharedPtr<FZBoneStructure> RightHandPinky2 = MakeShared<FZBoneStructure>(TEXT("RightHandPinky2"));
	TSharedPtr<FZBoneStructure> RightHandPinky3 = MakeShared<FZBoneStructure>(TEXT("RightHandPinky3"));
								
	TSharedPtr<FZBoneStructure> RightHandRing1 = MakeShared<FZBoneStructure>(TEXT("RightHandRing1"));
	TSharedPtr<FZBoneStructure> RightHandRing2 = MakeShared<FZBoneStructure>(TEXT("RightHandRing2"));
	TSharedPtr<FZBoneStructure> RightHandRing3 = MakeShared<FZBoneStructure>(TEXT("RightHandRing3"));
								
	TSharedPtr<FZBoneStructure> RightHandThumb1 = MakeShared<FZBoneStructure>(TEXT("RightHandThumb1"));
	TSharedPtr<FZBoneStructure> RightHandThumb2 = MakeShared<FZBoneStructure>(TEXT("RightHandThumb2"));
	TSharedPtr<FZBoneStructure> RightHandThumb3 = MakeShared<FZBoneStructure>(TEXT("RightHandThumb3"));


	// 부모-자식 관계 설정 (여기서 직접 설정)
	RootBone->Children = { Spine, LeftUpLeg, RightUpLeg};

	Spine->Parent = RootBone;
	LeftUpLeg->Parent = RootBone;
	RightUpLeg->Parent = RootBone;


	LeftUpLeg->Children = { LeftLeg };
	LeftLeg->Parent = LeftUpLeg;

	LeftLeg->Children = { LeftFoot };
	LeftFoot->Parent = LeftLeg;

	LeftFoot->Children = { LeftToeBase };
	LeftToeBase->Parent = LeftFoot;


	RightUpLeg->Children = { RightLeg };
	RightLeg->Parent = RightUpLeg;
	
	RightLeg->Children = { RightFoot };
	RightFoot->Parent = RightLeg;
	
	RightFoot->Children = { RightToeBase };
	RightToeBase->Parent = RightFoot;

	Spine->Children = { Spine1 };
	Spine1->Parent = Spine;

	Spine1->Children = { Spine2 };
	Spine2->Parent = Spine1;

	Spine2->Children = { LeftShoulder, Neck, RightShoulder };
	LeftShoulder->Parent = Spine2;
	Neck->Parent = Spine2;
	RightShoulder->Parent = Spine2;

	LeftShoulder->Children = { LeftArm };
	LeftArm->Parent = LeftShoulder;

	LeftArm->Children = { LeftForeArm };
	LeftForeArm->Parent = LeftArm;

	LeftForeArm->Children = { LeftHand };
	LeftHand->Parent = LeftForeArm;

	LeftHand->Children = { LeftHandIndex1, LeftHandMiddle1, LeftHandPinky1, LeftHandRing1, LeftHandThumb1};
	LeftHandIndex1->Parent = LeftHand;
	LeftHandMiddle1->Parent = LeftHand;
	LeftHandPinky1->Parent = LeftHand;
	LeftHandRing1->Parent = LeftHand;
	LeftHandThumb1->Parent = LeftHand;

	LeftHandIndex1->Children = { LeftHandIndex2 };
	LeftHandIndex2->Parent = LeftHandIndex1;

	LeftHandIndex2->Children = { LeftHandIndex3 };
	LeftHandIndex3->Parent = LeftHandIndex2;


	LeftHandMiddle1->Children = { LeftHandMiddle2 };
	LeftHandMiddle2->Parent = LeftHandMiddle1;

	LeftHandMiddle2->Children = { LeftHandMiddle3 };
	LeftHandMiddle3->Parent = LeftHandMiddle2;


	LeftHandPinky1->Children = { LeftHandPinky2 };
	LeftHandPinky2->Parent = LeftHandPinky1;

	LeftHandPinky2->Children = { LeftHandPinky3 };
	LeftHandPinky3->Parent = LeftHandPinky2;


	LeftHandRing1->Children = { LeftHandRing2 };
	LeftHandRing2->Parent = LeftHandRing1;
	
	LeftHandRing2->Children = { LeftHandRing3 };
	LeftHandRing3->Parent = LeftHandRing2;


	LeftHandThumb1->Children = { LeftHandThumb2 };
	LeftHandThumb2->Parent = LeftHandThumb1;
	
	LeftHandThumb2->Children = { LeftHandThumb3 };
	LeftHandThumb3->Parent = LeftHandThumb2;


	Neck->Children = { Head };
	Head->Parent = Neck;
	
	Head->Children = { LeftEye, RightEye };
	LeftEye->Parent = Head;
	RightEye->Parent = Head;


	RightShoulder->Children = { RightArm };
	RightArm->Parent = RightShoulder;

	RightArm->Children = { RightForeArm };
	RightForeArm->Parent = RightArm;

	RightForeArm->Children = { RightHand };
	RightHand->Parent = RightForeArm;

	RightHand->Children = { RightHandIndex1, RightHandMiddle1, RightHandPinky1, RightHandRing1, RightHandThumb1 };
	RightHandIndex1->Parent = RightHand;
	RightHandMiddle1->Parent = RightHand;
	RightHandPinky1->Parent = RightHand;
	RightHandRing1->Parent = RightHand;
	RightHandThumb1->Parent = RightHand;

	RightHandIndex1->Children = { RightHandIndex2 };
	RightHandIndex2->Parent = RightHandIndex1;

	RightHandIndex2->Children = { RightHandIndex3 };
	RightHandIndex3->Parent = RightHandIndex2;

	RightHandMiddle1->Children = { RightHandMiddle2 };
	RightHandMiddle2->Parent = RightHandMiddle1;

	RightHandMiddle2->Children = { RightHandMiddle3 };
	RightHandMiddle3->Parent = RightHandMiddle2;

	RightHandPinky1->Children = { RightHandPinky2 };
	RightHandPinky2->Parent = RightHandPinky1;

	RightHandPinky2->Children = { RightHandPinky3 };
	RightHandPinky3->Parent = RightHandPinky2;

	RightHandRing1->Children = { RightHandRing2 };
	RightHandRing2->Parent = RightHandRing1;

	RightHandRing2->Children = { RightHandRing3 };
	RightHandRing3->Parent = RightHandRing2;

	RightHandThumb1->Children = { RightHandThumb2 };
	RightHandThumb2->Parent = RightHandThumb1;

	RightHandThumb2->Children = { RightHandThumb3 };
	RightHandThumb3->Parent = RightHandThumb2;

}

void ABaseZombie::InitializeSpecialBoneHierarchy()
{
	SpecialRootBone = MakeShared<FZBoneStructure>(TEXT("Head"));

	TSharedPtr<FZBoneStructure> Neck = MakeShared<FZBoneStructure>(TEXT("Neck"));
	TSharedPtr<FZBoneStructure> Spine2 = MakeShared<FZBoneStructure>(TEXT("Spine2"));
	TSharedPtr<FZBoneStructure> Spine1 = MakeShared<FZBoneStructure>(TEXT("Spine1"));
	TSharedPtr<FZBoneStructure> Spine = MakeShared<FZBoneStructure>(TEXT("Spine"));
	TSharedPtr<FZBoneStructure> Hips = MakeShared<FZBoneStructure>(TEXT("Hips"));

	TSharedPtr<FZBoneStructure> LeftUpLeg = MakeShared<FZBoneStructure>(TEXT("LeftUpLeg"));
	TSharedPtr<FZBoneStructure> LeftLeg = MakeShared<FZBoneStructure>(TEXT("LeftLeg"));
	TSharedPtr<FZBoneStructure> LeftFoot = MakeShared<FZBoneStructure>(TEXT("LeftFoot"));
	TSharedPtr<FZBoneStructure> LeftToeBase = MakeShared<FZBoneStructure>(TEXT("LeftToeBase"));

	TSharedPtr<FZBoneStructure> RightUpLeg = MakeShared<FZBoneStructure>(TEXT("RightUpLeg"));
	TSharedPtr<FZBoneStructure> RightLeg = MakeShared<FZBoneStructure>(TEXT("RightLeg"));
	TSharedPtr<FZBoneStructure> RightFoot = MakeShared<FZBoneStructure>(TEXT("RightFoot"));
	TSharedPtr<FZBoneStructure> RightToeBase = MakeShared<FZBoneStructure>(TEXT("RightToeBase"));

	// 부모-자식 관계 설정 (여기서 직접 설정)
	SpecialRootBone->Children = { Neck };
	Neck->Parent = SpecialRootBone;

	Neck->Children = { Spine2 };
	Spine2->Parent = Neck;

	Spine2->Children = { Spine1 };
	Spine1->Parent = Spine2;

	Spine1->Children = { Spine };
	Spine->Parent = Spine1;

	Spine->Children = { Hips };
	Hips->Parent = Spine;

	Hips->Children = { LeftUpLeg, RightUpLeg };
	LeftUpLeg->Parent = Hips;
	RightUpLeg->Parent = Hips;

	LeftUpLeg->Children = { LeftLeg };
	LeftLeg->Parent = LeftUpLeg;

	LeftLeg->Children = { LeftFoot };
	LeftFoot->Parent = LeftLeg;

	LeftFoot->Children = { LeftToeBase };
	LeftToeBase->Parent = LeftFoot;

	RightUpLeg->Children = { RightLeg };
	RightLeg->Parent = RightUpLeg;
	
	RightLeg->Children = { RightFoot };
	RightFoot->Parent = RightLeg;
	
	RightFoot->Children = { RightToeBase };
	RightToeBase->Parent = RightFoot;

}

void ABaseZombie::PrintBoneHierarchy(TSharedPtr<FZBoneStructure> Bone, int Depth)
{
	if (!Bone.IsValid()) return;

	FString Indentation = FString::ChrN(Depth * 2, ' '); // 깊이에 따라 들여쓰기
	UE_LOG(LogTemp, Warning, TEXT("%s%s"), *Indentation, *Bone->Name.ToString());

	for (const TSharedPtr<FZBoneStructure>& Child : Bone->Children)
	{
		PrintBoneHierarchy(Child, Depth + 1);
	}
}

bool ABaseZombie::InBone(FName BoneAName, FName BoneBName, TSharedPtr<FZBoneStructure> StartBone)
{

	TSharedPtr<FZBoneStructure> BoneA = FindBoneByName(StartBone, BoneAName);
	TSharedPtr<FZBoneStructure> BoneB = FindBoneByName(StartBone, BoneBName);

	if (!BoneA.IsValid() || !BoneB.IsValid())
	{
		return false; // 이름에 해당하는 뼈대가 존재하지 않으면 false 반환
	}

	return (BoneA->Name == BoneB->Name || BoneA->IsChildOf(BoneB));

}

TSharedPtr<FZBoneStructure> ABaseZombie::FindBoneByName(TSharedPtr<FZBoneStructure> StartBone, FName BoneName)
{
	if (!StartBone.IsValid())
		return nullptr;

	// 현재 뼈대가 이름과 일치하는지 확인
	if (StartBone->Name == BoneName)
	{
		return StartBone;
	}

	// 자식 뼈대들 탐색
	for (const TSharedPtr<FZBoneStructure>& Child : StartBone->Children)
	{
		TSharedPtr<FZBoneStructure> FoundBone = FindBoneByName(Child, BoneName);
		if (FoundBone.IsValid())
		{
			return FoundBone;
		}
	}

	return nullptr; // 찾지 못한 경우
}


// Called when the game starts or when spawned
void ABaseZombie::BeginPlay()
{
	Super::BeginPlay();

	CachedAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());

	GameInstance = Cast<UProGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseZombie::OnZombieHit);

	InitializeBoneHierarchy();
	InitializeSpecialBoneHierarchy();
	//PrintBoneHierarchy(RootBone);
	//PrintBoneHierarchy(SpecialRootBone);

	GrowlSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sound/ZombieGrowl.ZombieGrowl"));
	if (!GrowlSound)
	{
		UE_LOG(LogTemp, Error, TEXT("GrowlSound failed to load in BeginPlay!"));
	}

	//BeAttackedSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sound/ZombieBeAttacked.ZombieBeAttacked"));
	//if (!BeAttackedSound)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("BeAttackedSound failed to load in BeginPlay!"));
	//}

	DeathSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sound/zombie-death.zombie-death"));
	if (!DeathSound)
	{
		UE_LOG(LogTemp, Error, TEXT("DeathSound failed to load in BeginPlay!"));
	}

	ScaredSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sound/zombie-scared.zombie-scared"));
	if (!ScaredSound)
	{
		UE_LOG(LogTemp, Error, TEXT("ScaredSound failed to load in BeginPlay!"));
	}

	// 5초 후에 EnablePlayerDetection() 호출
	GetWorld()->GetTimerManager().SetTimer(DetectionTimerHandle, [this]() {
		//UE_LOG(LogTemp, Log, TEXT("5초 지남, 탐지 활성화"));
		bCanDetectPlayer = true;  // 플레이어 탐지 가능 상태로 변경
		} , 5.0f, false);
}

// Called every frame
void ABaseZombie::Tick(float DeltaTime)
{
	// 다른 층에서 좀비가 죽었고 부활 도중에 해당 층으로 로컬 플레이어가 움직였을때 좀비 부활에 문제가 있어서 => 좀비 절단 사망은 다른 어느 층에 있던지 간에 동기화 시키고 렌더링, 물리 처리를 해준다!
	if (doAction_setIsCuttingDead_onTick == false && m_bIsCuttingDead == false
		&& doAction_setIsNormalDead_onTick == false) {	// 노말 사망도 부활 시점을 맞춰줘야 해서
		if (MyChar == nullptr)	// 크래시 방지
			return;

		// 클라 - 로컬 클라에게 보이지 않는 좀비 (같은 층에 없는) 최적화 작업
		if (MyChar->floor != floor) {
			SetActorHiddenInGame(true);	// 렌더링 중지
			UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
			if (MeshComponent)
			{
				MeshComponent->SetSimulatePhysics(false); // 물리 시뮬레이션 중지
			}
			return;
		}
		else {
			SetActorHiddenInGame(false);	// 렌더링 다시 시작
			UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
			if (MeshComponent)
			{
				MeshComponent->SetSimulatePhysics(true); // 물리 시뮬레이션 다시 시작
			}
		}
	}
	// doAction_setIsCuttingDead_onTick == true || m_bIsCuttingDead == true || doAction_setIsNormalDead_onTick == true
	else {
		// 다른 층에 있던 좀비에 경우를 위하여 
		if (doAction_setIsCuttingDead_onTick == true || m_bIsCuttingDead == true) {	// 노멀 데드는 렌더링, 물리 처리 다시 켜줄 이유 없음
			SetActorHiddenInGame(false);	// 렌더링 다시 시작
			UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
			if (MeshComponent)
			{
				MeshComponent->SetSimulatePhysics(true); // 물리 시뮬레이션 다시 시작
			}
		}
	}


	// CutProceduralMesh Impulse 적용
	if (CutProceduralMesh_1) {
		FVector WeaponForward;
		float min_weight = 10.f;

		if (procMesh_AddImpulse_1 == false) {
			if (PlayerWeapon)
				WeaponForward = PlayerWeapon->GetActorRotation().Vector();

			//UE_LOG(LogTemp, Log, TEXT("(CutProcedural_1)"));

			float weight = CutProceduralMesh_1->CalculateMass();
			if (weight < min_weight) {
				weight = min_weight;
				CutProceduralMesh_1->SetMassOverrideInKg(NAME_None, weight, true);
			}
			// 너무 얇게 짤려 컨벡스 메시 자체가 생성 안되었을 경우!
			int32 ConvexElementCount = CutProceduralMesh_1->GetBodySetup()->AggGeom.ConvexElems.Num();
			if (ConvexElementCount == 0) {
				CreateAndApplyBoundingBox(CutProceduralMesh_1);
				CutProceduralMesh_1->SetMassOverrideInKg(NAME_None, weight, true);
			}
			UE_LOG(LogTemp, Log, TEXT("Weight: %f"), weight);

			float x_baseImpulse = 3500.0f; //FMath::RandRange(2000.0f, 5000.0f);
			float x_impulse = SetImpulseByWeight(weight, x_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("X Impulse: %f"), x_impulse);

			float y_baseImpulse = 3500.0f; //FMath::RandRange(2000.0f, 5000.0f);
			float y_impulse = SetImpulseByWeight(weight, y_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("Y Impulse: %f"), y_impulse);

			float z_baseImpulse = 12500.0f; //FMath::RandRange(7000.0f, 10000.0f);
			float z_impulse = SetImpulseByWeight(weight, z_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("Z Impulse: %f"), z_impulse);

			if (doAction_addImpulseOnProcMesh_onTick == false) {
				UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Weapon Foward Vector: %s"), *WeaponForward.ToString());

				CutProceduralMesh_1->AddImpulseAtLocation(FVector(WeaponForward.X * x_impulse, WeaponForward.Y * y_impulse, WeaponForward.Z * z_impulse), CutProceduralMesh_1->K2_GetComponentLocation());
				procMesh_AddImpulse_1 = true;
			}
			else if (doAction_addImpulseOnProcMesh_onTick == true) {
				UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Sync Impulse (Weapon) Vector: %s"), *sync_cutImpulse.ToString());

				CutProceduralMesh_1->AddImpulseAtLocation(FVector(sync_cutImpulse.X * x_impulse, sync_cutImpulse.Y * y_impulse, sync_cutImpulse.Z * z_impulse), CutProceduralMesh_1->K2_GetComponentLocation());
				procMesh_AddImpulse_1 = true;
			}
		}
	}

	if (CutProceduralMesh_2) {
		FVector WeaponForward;
		float min_weight = 10.f;

		if (procMesh_AddImpulse_2 == false) {
			if (PlayerWeapon)
				WeaponForward = PlayerWeapon->GetActorRotation().Vector();

			//UE_LOG(LogTemp, Log, TEXT("(CutProcedural_2)"));

			float weight = CutProceduralMesh_2->CalculateMass();
			if (weight < min_weight) {
				weight = min_weight;
				CutProceduralMesh_2->SetMassOverrideInKg(NAME_None, weight, true);
			}
			// 너무 얇게 짤려 컨벡스 메시 자체가 생성 안되었을 경우!
			int32 ConvexElementCount = CutProceduralMesh_2->GetBodySetup()->AggGeom.ConvexElems.Num();
			if (ConvexElementCount == 0) {
				CreateAndApplyBoundingBox(CutProceduralMesh_2);
				CutProceduralMesh_2->SetMassOverrideInKg(NAME_None, weight, true);
			}
			UE_LOG(LogTemp, Log, TEXT("Weight: %f"), weight);

			float x_baseImpulse = 10000.0f; //FMath::RandRange(8000.0f, 12000.0f);
			float x_impulse = SetImpulseByWeight(weight, x_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("X Impulse: %f"), x_impulse);

			float y_baseImpulse = 10000.0f; //FMath::RandRange(8000.0f, 12000.0f);
			float y_impulse = SetImpulseByWeight(weight, y_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("Y Impulse: %f"), y_impulse);

			float z_baseImpulse = 3000.0f; //FMath::RandRange(2000.0f, 4000.0f);
			float z_impulse = SetImpulseByWeight(weight, z_baseImpulse);
			//UE_LOG(LogTemp, Log, TEXT("Z Impulse: %f"), z_impulse);

			if (doAction_addImpulseOnProcMesh_onTick == false) {
				UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Weapon Backward Vector: %s"), *(-WeaponForward).ToString());

				CutProceduralMesh_2->AddImpulseAtLocation(FVector(-WeaponForward.X * x_impulse, -WeaponForward.Y * y_impulse, z_impulse), CutProceduralMesh_2->K2_GetComponentLocation());
				procMesh_AddImpulse_2 = true;
			}
			else if (doAction_addImpulseOnProcMesh_onTick == true) {
				UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Sync Impulse (Weapon) Backward Vector : % s"), *(-sync_cutImpulse).ToString());

				CutProceduralMesh_2->AddImpulseAtLocation(FVector(-sync_cutImpulse.X * x_impulse, -sync_cutImpulse.Y * y_impulse, z_impulse), CutProceduralMesh_2->K2_GetComponentLocation());
				procMesh_AddImpulse_2 = true;
			}
		}
	}
	int32 i = 0;
	// for 루프에서 순회하면서 각 ProcMesh에 대해 처리
	for (UProceduralMeshComponent* ProcMesh : ProceduralMeshes) {
		if (ProcMesh) {

			FVector WeaponForward;
			float min_weight = 10.f;

			// 상태값을 TMap에서 가져옴
			bool& bAddImpulse = ProcMeshImpulseStates.FindOrAdd(ProcMesh, false); // 기본값은 false

			// 임펄스를 추가할 때, 이전에 추가하지 않았다면 추가
			if (bAddImpulse == false) {
				if (PlayerWeapon) {
					WeaponForward = PlayerWeapon->GetActorRotation().Vector();
				}

				// 메시의 질량을 계산하고, 최소 값이 적용되도록 함
				float weight = ProcMesh->CalculateMass();
				if (weight < min_weight) {
					weight = min_weight;
					ProcMesh->SetMassOverrideInKg(NAME_None, weight, true);
				}

				// 너무 얇게 짤려 컨벡스 메시 자체가 생성 안되었을 경우!
				int32 ConvexElementCount = ProcMesh->GetBodySetup()->AggGeom.ConvexElems.Num();
				if (ConvexElementCount == 0) {
					CreateAndApplyBoundingBoxByNewProcMesh(ProcMesh);
					ProcMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
					ProcMesh->SetCollisionProfileName(TEXT("Ragdoll"));
					ProcMesh->SetSimulatePhysics(true);

					ProcMesh->SetMassOverrideInKg(NAME_None, weight, true);
				}


				UE_LOG(LogTemp, Log, TEXT("Weight: %f"), weight);

				// 임펄스를 설정
				float x_baseImpulse = 1000.0f;
				float x_impulse = SetImpulseByWeight(weight, x_baseImpulse);

				float y_baseImpulse = 1000.0f;
				float y_impulse = SetImpulseByWeight(weight, y_baseImpulse);

				float z_baseImpulse = 300.0f;
				float z_impulse = SetImpulseByWeight(weight, z_baseImpulse);

				// 임펄스를 추가할 때 실행 여부 결정
				if (doAction_addImpulseOnProcMesh_onTick == false) {
					UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Weapon Backward Vector: %s, ProcMeshIndex : %d"), *(-WeaponForward).ToString(), i);

					ProcMesh->AddImpulseAtLocation(FVector(-WeaponForward.X * x_impulse, -WeaponForward.Y * y_impulse, z_impulse), ProcMesh->K2_GetComponentLocation());

					// 상태값을 true로 변경하여 다음에는 임펄스를 추가하지 않도록 설정
					bAddImpulse = true;
				}
				else if (doAction_addImpulseOnProcMesh_onTick == true) {
					UE_LOG(LogTemp, Log, TEXT("[Add Impulse on Tick] Sync Impulse (Weapon) Backward Vector : % s, ProcMeshIndex : %d"), *(-sync_cutImpulse).ToString(), i);

					ProcMesh->AddImpulseAtLocation(FVector(-sync_cutImpulse.X * x_impulse, -sync_cutImpulse.Y * y_impulse, z_impulse), ProcMesh->K2_GetComponentLocation());

					// 상태값을 true로 변경하여 다음에는 임펄스를 추가하지 않도록 설정
					bAddImpulse = true;
				}
			}
			++i;
		}
	}


	float bloodspawn_z_offset = 80.f;

	// 좀비 피격시 클라 동기화
	if (m_fHP_Prev != m_fHP && GetHP() > 0 && m_bBeAttacked == false && doAction_takeDamage_onTick == true) {

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("좀비 피격 클라 동기화 작업실행!")));	// 직접 때리는 클라에서는 해당 메세지 보이면 안 됨!
		UE_LOG(LogTemp, Log, TEXT("좀비 피격 동기화 작업실행! - 좀비 아이디: %d"), ZombieId);

		doAction_takeDamage_onTick = false;	// 딱 한번만 실행 시키도록

		BeAttacked();
		m_fHP_Prev = m_fHP;

		BloodFX.Empty();

		FRotator RandRotate;

		RandRotate.Yaw = FMath::FRandRange(0.f, 1.f);
		RandRotate.Roll = FMath::FRandRange(0.f, 1.f);
		RandRotate.Pitch = FMath::FRandRange(0.f, 1.f);

		ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), GetActorLocation() + FVector(0, 0, bloodspawn_z_offset), RandRotate);

		if (NewBloodFX) {
			//int32 bmin = 0;
			//int32 bmax = 0;
			//
			//if (Weapon->WeaponName == "ButchersKnife" || Weapon->WeaponName == "FireAxe" || Weapon->WeaponName == "SashimiKnife") {	// 날붙이 무기
			//	bmin = NewBloodFX->blood_spawncount_cuthit_min;
			//	bmax = NewBloodFX->blood_spawncount_cuthit_max;
			//}
			//else {	// 그 외 타격무기 (둔기)
			//	bmin = NewBloodFX->blood_spawncount_normalhit_min;
			//	bmax = NewBloodFX->blood_spawncount_normalhit_max;
			//}	
			// ===> 동기화에서는 피격 당시에 좀비를 때린 무기를 알수가 없네;;

			int32 bmin = NewBloodFX->blood_spawncount_awaysynchit_min;
			int32 bmax = NewBloodFX->blood_spawncount_awaysynchit_max;

			NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
			NewBloodFX->spawn_flag = true;

			BloodFX.Add(NewBloodFX);
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
			UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
		}
	}


	// 좀비 사망처리 클라 동기화 - 애니메이션 재생, 피 이펙트 생성

	// normal dead 동기화
	if (GetHP() <= 0 && m_bIsNormalDead == false && doAction_setIsNormalDead_onTick == true) {

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("좀비 사망 클라 동기화 작업실행!")));
		UE_LOG(LogTemp, Log, TEXT("좀비 사망 동기화 작업실행! - (normal dead) 좀비 아이디: %d"), ZombieId);

		m_bIsNormalDead = true;
		auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
		if (nullptr != CharacterAnimInstance) {
			CharacterAnimInstance->SetIsNormalDead(m_bIsNormalDead);
		}
		GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

		doAction_setIsNormalDead_onTick = false;	// 딱 한번만 실행 시키도록

		StartResurrectionTimer();

		BloodFX.Empty();

		FRotator RandRotate;

		RandRotate.Yaw = FMath::FRandRange(0.f, 1.f);
		RandRotate.Roll = FMath::FRandRange(0.f, 1.f);
		RandRotate.Pitch = FMath::FRandRange(0.f, 1.f);

		ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), GetActorLocation() + FVector(0, 0, bloodspawn_z_offset), RandRotate);

		int bmin = NewBloodFX->blood_spawncount_normaldead_min;
		int bmax = NewBloodFX->blood_spawncount_normaldead_max;

		if (NewBloodFX) {
			NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
			//NewBloodFX->blood_spawnloop = true;
			NewBloodFX->spawn_flag = true;

			BloodFX.Add(NewBloodFX);
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
			UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
		}
	}

	// cut dead 동기화
	if (GetHP() <= 0 && m_bIsCuttingDead == false && doAction_setIsCuttingDead_onTick == true) {

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("좀비 절단 사망 클라 동기화 작업실행!")));
		UE_LOG(LogTemp, Log, TEXT("좀비 사망 동기화 작업실행! - (cut dead) 좀비 아이디: %d"), ZombieId);

		doAction_setIsCuttingDead_onTick = false;	// 딱 한번만 실행 시키도록

		doAction_addImpulseOnProcMesh_onTick = true;

		m_bIsCuttingDead = true;
		m_bIsStanding = true;
		auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
		if (nullptr != CharacterAnimInstance) {
			CharacterAnimInstance->SetIsCuttingDead(m_bIsCuttingDead);
			CharacterAnimInstance->SetIsStanding(m_bIsStanding);	// 누운 상태로 시작하도록 (결합 후 부활때 한틱 죽는 애니메이션 재생 안되도록)
			GetMesh()->GlobalAnimRateScale = 0.1f;	// 일단 전체 애니메이션 정지 시켜 놓기 (배속을 느리게해서 정지와 비슷하게 -> 완전 정지시켜 놓으면 어차피 다시 키면서 순간 강제 전환 때문에 깜빡임)
			// 0.01배속이 딱임 => 초반 애니메이션 강제 전환시에 깜빡임을 넘겨주고 동시에 보이지 않는 상태(백그라운드)에서 쓰러지는 애니메이션 느리게 재생되면서 딱 누워 있는 상태일때가 10초후! 그럼 이제 다시 보이게 하면 딱 맞음
		}
		GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

		PlayDeathSound();

		StartResurrectionTimer();

		BloodFX.Empty();

		// 피 생성 방향 동기화
		FRotator SyncRotate = sync_cutImpulse.Rotation();	// sync_cutImpulse 에는 FVector WeaponForward = GetActorRotation().Vector() 가 담겨 있음 (NormalWeaponActor에서 패킷 설정)

		ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), GetActorLocation(), SyncRotate);

		int bmin = NewBloodFX->blood_spawncount_cutdead_min;
		int bmax = NewBloodFX->blood_spawncount_cutdead_max;

		if (NewBloodFX) {
			NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
			//NewBloodFX->blood_spawnloop = true;
			NewBloodFX->spawn_flag = true;

			BloodFX.Add(NewBloodFX);
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
			UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
		}

		CutZombie(sync_cutPlane, sync_cutNormal, false);
	}

	if (bIsMergingInProgress)
	{
		double StartTime = FPlatformTime::Seconds();


		ElapsedTime += DeltaTime;

		// 10초 동안 이동
		float TimeRatio = FMath::Clamp(ElapsedTime / TotalTimeToMove, 0.0f, 1.0f); // 0~1 사이로 고정

		if (CutProceduralMesh_1) {
			//CutPro_1을 이동
			FVector CutPro_1Location = CutPro_1StartLocation + (CutPro_1Distance * TimeRatio);
			CutProceduralMesh_1->SetWorldLocation(CutPro_1Location);

			FRotator NewRotation = FMath::Lerp(CutPro_1StartRotation, CutPro_1TargetRotation, TimeRatio);
			CutProceduralMesh_1->SetWorldRotation(NewRotation);
		}


		if (CutProceduralMesh_2) {
			//CutPro_2을 이동
			FVector CutPro_2Location = CutPro_2StartLocation + (CutPro_2Distance * TimeRatio);
			CutProceduralMesh_2->SetWorldLocation(CutPro_2Location);

			FRotator NewRotation = FMath::Lerp(CutPro_2StartRotation, CutPro_2TargetRotation, TimeRatio);
			CutProceduralMesh_2->SetWorldRotation(NewRotation);
		}

		if (m_bIsCutProceduralMesh_2Visibility) {

			// 10초 동안 이동한 후, 완료되었으면 머지 진행을 종료
			if (ElapsedTime >= TotalTimeToMove)
			{
				bIsMergingInProgress = false;
				UE_LOG(LogTemp, Log, TEXT("Merging Completed!"));
				//GetMesh()->SetHiddenInGame(false);

				if (CutProceduralMesh_1) {
					CutProceduralMesh_1->DestroyComponent();
					CutProceduralMesh_1 = nullptr;
				}
				if (CutProceduralMesh_2) {
					CutProceduralMesh_2->DestroyComponent();
					CutProceduralMesh_2 = nullptr;
				}


				//GetMesh()->SetVisibility(true);
			}
		}
		else {
			// 10초 동안 이동
			for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
			{
				UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
				if (!ProcMesh) continue;

				// 초기 위치 저장 (메쉬의 원래 위치)
				FVector StartLocation = ProcMeshMergeStartLocation[MeshIndex];

				if (MeshIndex < ProcMeshMergeTargetLocation.Num())
				{

					// 이동해야 할 총 거리
					FVector TotalMoveDistance = ProcMeshDistance[MeshIndex];

					FVector TargetLo = StartLocation + (TotalMoveDistance * TimeRatio);

					// 10초 동안 이동 (프로시저 메쉬 자체를 이동)
					ProcMesh->SetWorldLocation(TargetLo);

					FRotator NewRotation = FMath::Lerp(ProcMeshMergeStartRotation[MeshIndex], ProcMeshMergeTargetRotation[MeshIndex], TimeRatio);

					ProcMesh->SetWorldRotation(NewRotation);
				}

			}

			// 10초 동안 이동한 후, 완료되었으면 머지 진행을 종료
			if (ElapsedTime >= TotalTimeToMove)
			{
				bIsMergingInProgress = false;
				UE_LOG(LogTemp, Log, TEXT("Merging Completed!"));

				//GetMesh()->SetHiddenInGame(false);
				
				// 절단 메시 삭제
				for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
				{
					UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
					if (!ProcMesh) continue;
					ProcMesh->DestroyComponent();
				}

				if (CutProceduralMesh_1) {
					CutProceduralMesh_1->DestroyComponent();
					CutProceduralMesh_1 = nullptr;
				}

				if (CutProceduralMesh_2) {
					CutProceduralMesh_2->DestroyComponent();
					CutProceduralMesh_2 = nullptr;
				}

				ProceduralMeshes.Empty();
				ProcMeshImpulseStates.Empty();

				//GetMesh()->SetVisibility(true); 
			}
		}


		//UE_LOG(LogTemp, Warning, TEXT("Merged DeltaTime %f "), DeltaTime);
		//UE_LOG(LogTemp, Warning, TEXT("Merging!!!!!!!!!!"));
		//++m_iMergeFlag;


		double EndTime = FPlatformTime::Seconds();
		//UE_LOG(LogTemp, Warning, TEXT("bIsMergingInProgressMesh took: %f seconds"), EndTime - StartTime);
	}

	// 좀비 도망가기 체력회복 이펙트 생성/파괴
	if (targetType == RUNAWAY && IsRunaway == false) {
		IsRunaway = true;
		Runaway();
	}
	else if ((IsRunaway == true && (targetType != RUNAWAY && targetType != BLACKBOARDCLEARED)) || GetHP() <= 0.f) {
		IsRunaway = false;
		if (HealingFX != NULL) {
			HealingFX->EndPlay(EEndPlayReason::Destroyed);
		}
	}


	Super::Tick(DeltaTime);

}

void ABaseZombie::OnZombieHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != nullptr && OtherActor != this) {
		//UE_LOG(LogTemp, Display, TEXT("Zombie collided with %s"), *OtherActor->GetName());
	}

	Protocol::patrol_hit Packet;
	Packet.set_zombieid(ZombieId);
	Packet.set_packet_type(14);

	std::string serializedData;
	Packet.SerializeToString(&serializedData);

	bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());

}

void ABaseZombie::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 변경 필요 각각의 animinstance가 필요할 것 같다.
	auto AnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());

	AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseZombie::AttackMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseZombie::ShoutingMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &ABaseZombie::BeAttackedMontageEnded);

	AnimInstance->OnAttackStartCheck.AddLambda([this]() -> void {
		AttackCheck();
		});

}

void ABaseZombie::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

// 100kg 기준 baseImpulse 만큼의 Impulse 적용
float ABaseZombie::SetImpulseByWeight(float targetWeight, float baseImpulse)
{
	float baseWeight = 100.0f;

	float targetImpulse = baseImpulse * (targetWeight / baseWeight);

	return targetImpulse;
}



// 좀비가 공격을 받았을 때
float ABaseZombie::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("HP %f"), GetHP()));

	doAction_takeDamage_onTick = false;
	doAction_setIsNormalDead_onTick = false;
	doAction_setIsCuttingDead_onTick = false;

	m_fHP_Prev = GetHP();	// 동기화 될 경우 이거는 업데이트를 따로 안 해줘서
	float hp = GetHP() - Damage;
	if (hp <= 0)	// 체력이 음수가 되지는 않도록
		hp = 0;
	SetHP(hp);
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("HP %f"), GetHP()));
	UE_LOG(LogTemp, Log, TEXT("[Zombie#%u] TakeDamage - HP: %f -> %f"), GetZombieId(), m_fHP_Prev, hp);

	m_fHP_Prev = m_fHP;

	ANormalWeaponActor* Weapon = Cast<ANormalWeaponActor>(DamageCauser);

	if (Weapon == nullptr) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[ERROR] TakeDamage - Weapon is nullptr!!! No Damage!")));
		UE_LOG(LogTemp, Error, TEXT("TakeDamage - Weapon is nullptr!!! No Damage!"));
		return 0;
	}

	//UE_LOG(LogTemp, Log, TEXT("Weapon Location: %s"), *Weapon->GetActorLocation().ToString());
	//UE_LOG(LogTemp, Log, TEXT("Weapon Rotation: %s"), *Weapon->GetActorRotation().ToString());
	//UE_LOG(LogTemp, Log, TEXT("Weapon Rotation Vector: %s"), *Weapon->GetActorRotation().Vector().ToString());

	BloodFX.Empty();

	if (GetHP() <= 0) {	// 죽을때(절단 될 때)는 피가 더 많이 튀도록 /*& 절단 된 부분들에서 따로 피 생성되도록*/
		if (Weapon->WeaponName == "ButchersKnife" || Weapon->WeaponName == "FireAxe" || Weapon->WeaponName == "SashimiKnife") {	// 날붙이 무기
			ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), Weapon->GetActorLocation(), Weapon->GetActorRotation()); // 무기가 닿은 위치에서 무기가 바라보는 방향으로 피 이펙트 생성

			int bmin = NewBloodFX->blood_spawncount_cutdead_min;
			int bmax = NewBloodFX->blood_spawncount_cutdead_max;

			if (NewBloodFX) {
				NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
				//NewBloodFX->blood_spawnloop = true;
				NewBloodFX->spawn_flag = true;

				BloodFX.Add(NewBloodFX);
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
				UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
			}
		}
		else {	// 그 외 타격무기 (둔기)
			ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), Weapon->GetActorLocation(), Weapon->GetActorRotation()); // 무기가 닿은 위치에서 무기가 바라보는 방향으로 피 이펙트 생성

			int bmin = NewBloodFX->blood_spawncount_normaldead_min;
			int bmax = NewBloodFX->blood_spawncount_normaldead_max;

			if (NewBloodFX) {
				NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
				//NewBloodFX->blood_spawnloop = true;
				NewBloodFX->spawn_flag = true;

				BloodFX.Add(NewBloodFX);
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
				UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
			}
		}
	}
	else {	// 그냥 피격 당했을때
		ABloodNiagaEffect* NewBloodFX = GetWorld()->SpawnActor<ABloodNiagaEffect>(ABloodNiagaEffect::StaticClass(), Weapon->GetActorLocation(), Weapon->GetActorRotation()); // 무기가 닿은 위치에서 무기가 바라보는 방향으로 피 이펙트 생성
		
		if (NewBloodFX) {
			int32 bmin = 0;
			int32 bmax = 0;

			if (Weapon->WeaponName == "ButchersKnife" || Weapon->WeaponName == "FireAxe" || Weapon->WeaponName == "SashimiKnife") {	// 날붙이 무기
				bmin = NewBloodFX->blood_spawncount_cuthit_min;
				bmax = NewBloodFX->blood_spawncount_cuthit_max;
			}
			else {	// 그 외 타격무기 (둔기)
				bmin = NewBloodFX->blood_spawncount_normalhit_min;
				bmax = NewBloodFX->blood_spawncount_normalhit_max;
			}

			NewBloodFX->blood_spawncount = FMath::RandRange(bmin, bmax);
			NewBloodFX->spawn_flag = true;

			BloodFX.Add(NewBloodFX);
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX spawn failed!!!")));
			UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX spawn failed!!!"));
		}

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("좀비 피격 클라 직접 실행!")));
		UE_LOG(LogTemp, Log, TEXT("좀비 피격 직접 실행! - 좀비 아이디: %d"), ZombieId);

		BeAttacked();
	}


	return Damage;
}

void ABaseZombie::SetNormalDeadWithAnim()
{ 
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("좀비 사망 직접 실행! - normal dead")));
	UE_LOG(LogTemp, Log, TEXT("좀비 사망 직접 실행! - (normal dead) 좀비 아이디: %d"), ZombieId);

	m_bIsNormalDead = true;
	auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	if (nullptr != CharacterAnimInstance) {
		CharacterAnimInstance->SetIsNormalDead(m_bIsNormalDead);
	}
	GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

	StartResurrectionTimer();
}

void ABaseZombie::SetCuttingDeadWithAnim()
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("좀비 사망 직접 실행! - cut dead")));
	UE_LOG(LogTemp, Log, TEXT("좀비 사망 직접 실행! - (cut dead) 좀비 아이디: %d"), ZombieId);

	m_bIsCuttingDead = true;
	m_bIsStanding = true;
	auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	if (nullptr != CharacterAnimInstance) {
		CharacterAnimInstance->SetIsCuttingDead(m_bIsCuttingDead);
		CharacterAnimInstance->SetIsStanding(m_bIsStanding);	// 누운 상태로 시작하도록 (결합 후 부활때 한틱 죽는 애니메이션 재생 안되도록)
		GetMesh()->GlobalAnimRateScale = 0.1f;	// 일단 전체 애니메이션 정지 시켜 놓기 (배속을 느리게해서 정지와 비슷하게 -> 완전 정지시켜 놓으면 어차피 다시 키면서 순간 강제 전환 때문에 깜빡임)
		// 0.01배속이 딱임 => 초반 애니메이션 강제 전환시에 깜빡임을 넘겨주고 동시에 보이지 않는 상태(백그라운드)에서 쓰러지는 애니메이션 느리게 재생되면서 딱 누워 있는 상태일때가 10초후! 그럼 이제 다시 보이게 하면 딱 맞음
	}
	GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

	PlayDeathSound();	// 커팅데드는 애니메이션에 묶어놓은 사운드 큐가 재생 안되어 따로 재생시켜줌

	StartResurrectionTimer();
}

void ABaseZombie::CutZombie(FVector planeposition, FVector planenormal, bool do_self)
{
	// 절단 디버깅 용 - 화면 출력
	//========================================

	//FVector Center;
	//FVector Right;  // 평면의 오른쪽 방향
	//FVector Forward;  // 평면의 앞쪽 방향
	//FVector Scale;  // 평면의 스케일
	//float Weapon_Scale = 0.f;	// 무기별 스케일 조정
	//
	//if (do_self == true && PlayerWeapon) {
	//	Center = PlayerWeapon->PlaneComponent->GetComponentLocation();
	//	Right = PlayerWeapon->PlaneComponent->GetRightVector();  // 평면의 오른쪽 방향
	//	Forward = PlayerWeapon->PlaneComponent->GetForwardVector();  // 평면의 앞쪽 방향
	//	Scale = PlayerWeapon->PlaneComponent->GetComponentScale();  // 평면의 스케일
	//
	//
	//	if (PlayerWeapon->WeaponName == "ButchersKnife") { Weapon_Scale = 45.f; }
	//	else if (PlayerWeapon->WeaponName == "FireAxe") { Weapon_Scale = 50.f; }
	//	else if (PlayerWeapon->WeaponName == "SashimiKnife") { Weapon_Scale = 40.f; }
	//
	//	float HalfWidth = Weapon_Scale * Scale.X;  // 평면의 폭
	//	float HalfHeight = Weapon_Scale * Scale.Y; // 평면의 높이
	//
	//	// 꼭짓점 계산
	//	FVector TopLeft = Center - Right * HalfWidth + Forward * HalfHeight;
	//	FVector TopRight = Center + Right * HalfWidth + Forward * HalfHeight;
	//	FVector BottomLeft = Center - Right * HalfWidth - Forward * HalfHeight;
	//	FVector BottomRight = Center + Right * HalfWidth - Forward * HalfHeight;
	//
	//	// 꼭짓점을 리스트에 추가
	//	TArray PlaneVertexs = { TopLeft, TopRight, BottomLeft, BottomRight };
	//
	//	if (PlaneVertexs.Num() >= 4) {
	//		float displaceTime = 30.f;
	//
	//		// 히트 지점에 평면의 선 그리기
	//		DrawDebugLine(
	//			GetWorld(),
	//			PlaneVertexs[0],			// 시작 위치
	//			PlaneVertexs[1],			// 히트 지점
	//			FColor::Green,				// 선 색상
	//			false,						// 지속 여부
	//			displaceTime,				// 지속 시간
	//			0,							// 깊이 우선 여부
	//			1.0f						// 선 두께
	//		);
	//
	//		DrawDebugLine(
	//			GetWorld(),
	//			PlaneVertexs[0],
	//			PlaneVertexs[2],
	//			FColor::Green,
	//			false,
	//			displaceTime,
	//			0,
	//			1.0f
	//		);
	//
	//		DrawDebugLine(
	//			GetWorld(),
	//			PlaneVertexs[2],
	//			PlaneVertexs[3],
	//			FColor::Green,
	//			false,
	//			displaceTime,
	//			0,
	//			1.0f
	//		);
	//
	//		DrawDebugLine(
	//			GetWorld(),
	//			PlaneVertexs[1],
	//			PlaneVertexs[3],
	//			FColor::Green,
	//			false,
	//			displaceTime,
	//			0,
	//			1.0f
	//		);
	//
	//		FVector planeposition_center = Center;	//(PlaneVertexs[0] + PlaneVertexs[1] + PlaneVertexs[2] + PlaneVertexs[3]) / 4.0f;
	//
	//		DrawDebugPoint(
	//			GetWorld(),
	//			planeposition_center,
	//			5.0f,
	//			FColor::Yellow,
	//			false,
	//			displaceTime,
	//			0
	//		);
	//
	//		FVector planenormal = FVector::CrossProduct(PlaneVertexs[3] - PlaneVertexs[0], PlaneVertexs[1] - PlaneVertexs[2]).GetSafeNormal();
	//
	//		DrawDebugLine(
	//			GetWorld(),
	//			planeposition_center,
	//			planeposition_center + planenormal * 20.0f,
	//			FColor::Yellow,
	//			false,
	//			displaceTime,
	//			0,
	//			1.0f
	//		);
	//	}
	//}
	
	//========================================

	USkeletalMeshComponent* Skeleton = GetMesh();
	if (Skeleton) {

		// ProceduralMeshComponent 생성 및 설정
		CutProceduralMesh_1 = NewObject<UProceduralMeshComponent>(this);

		if (!CutProceduralMesh_1) return;

		CreativeProceduralMesh(planeposition, planenormal);

	}
}



// 프로시저럴 메쉬 생성되는 부분
void ABaseZombie::CreativeProceduralMesh(FVector planeposition, FVector planenormal)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("CreativeProceduralMesh")));

	USkeletalMeshComponent* Skeleton = GetMesh();

	FSkeletalMeshRenderData* SkMeshRenderData = Skeleton->GetSkeletalMeshRenderData();
	const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[0];
	FSkinWeightVertexBuffer& SkinWeights = *Skeleton->GetSkinWeightBuffer(0);

	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	TArray<FVector> Vertices_Convex;
	int interval_Convex = 100;			// Convex 에서 사용할 vertices 개수 조절 (원래 vertices의 1/? 개)

	//UE_LOG(LogTemp, Log, TEXT("RenderSections.Num (int) %d"), DataArray.RenderSections.Num());

	for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
	{
		const int32 NumSourceVertices = DataArray.RenderSections[j].NumVertices;
		const int32 BaseVertexIndex = DataArray.RenderSections[j].BaseVertexIndex;

		//UE_LOG(LogTemp, Log, TEXT("RenderSections[%d] - NumSourceVertices.Num (int) %d"), j, DataArray.RenderSections[j].NumVertices);

		for (int32 i = 0; i < NumSourceVertices; i++)
		{
			const int32 VertexIndex = i + BaseVertexIndex;
			const FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(Skeleton, VertexIndex, DataArray, SkinWeights);

			Vertices.Add(FVector(SkinnedVectorPos.X, SkinnedVectorPos.Y, SkinnedVectorPos.Z));

			if (i % interval_Convex == 0) {
				Vertices_Convex.Add(FVector(SkinnedVectorPos.X, SkinnedVectorPos.Y, SkinnedVectorPos.Z));
			}

			const FVector3f ZTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
			const FVector3f XTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);

			Normals.Add(FVector(ZTangentStatic.X, ZTangentStatic.Y, ZTangentStatic.Z));
			Tangents.Add(FProcMeshTangent(FVector(XTangentStatic.X, XTangentStatic.Y, XTangentStatic.Z), false));

			const FVector2f SourceUVs = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0);
			FVector2d ResUVs;

			ResUVs.X = SourceUVs.X;
			ResUVs.Y = SourceUVs.Y;

			UV.Add(ResUVs);

			Colors.Add(FColor(0.0, 0.0, 0.0, 255));
		}
	}

	// ProceduralMesh에 Convex Collision 추가하기 위해서
	CutProceduralMesh_1->bUseComplexAsSimpleCollision = false;

	FMultiSizeIndexContainerData IndicesData;
	DataArray.MultiSizeIndexContainer.GetIndexBuffer(IndicesData.Indices);

	for (int32 j = 0; j < DataArray.RenderSections.Num(); j++)
	{
		TArray<int32> Triangles;

		const int32 SectionNumTriangles = DataArray.RenderSections[j].NumTriangles;
		const int32 SectionBaseIndex = DataArray.RenderSections[j].BaseIndex;

		for (int32 i = 0; i < SectionNumTriangles; i++)
		{
			int32 TriVertexIndex = i * 3 + SectionBaseIndex;
			Triangles.Add(IndicesData.Indices[TriVertexIndex]);
			Triangles.Add(IndicesData.Indices[TriVertexIndex + 1]);
			Triangles.Add(IndicesData.Indices[TriVertexIndex + 2]);
		}


		FTransform SkeletonTransform = Skeleton->GetComponentTransform();
		CutProceduralMesh_1->SetWorldTransform(SkeletonTransform);

		CutProceduralMesh_1->CreateMeshSection(j, Vertices, Triangles, Normals, UV, Colors, Tangents, true);

		for (int n = 0; n < Skeleton->GetNumMaterials(); n++) {
			CutProceduralMesh_1->SetMaterial(n, GetMesh()->GetMaterial(n));
		}
	}


	CutProceduralMesh_1->SetVisibility(true);
	CutProceduralMesh_1->SetHiddenInGame(false);
	CutProceduralMesh_1->RegisterComponent();

	// ProceduralMesh에 Convex Collision 추가
	CutProceduralMesh_1->AddCollisionConvexMesh(Vertices_Convex);

	CutProceduralMesh_1->SetSimulatePhysics(true);
	CutProceduralMesh_1->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CutProceduralMesh_1->SetCollisionProfileName(TEXT("Ragdoll"));


	// 절단 부위 proceduralmesh 생성
	SliceProceduralmesh(planeposition, planenormal);

	//UE_LOG(LogTemp, Log, TEXT("IsPhysicsStateCreated: %s"), CutProceduralMesh_1->IsPhysicsStateCreated() ? TEXT("true") : TEXT("false"));
	//UE_LOG(LogTemp, Log, TEXT("ShouldCreatePhysicsState: %s"), CutProceduralMesh_1->ShouldCreatePhysicsState() ? TEXT("true") : TEXT("false"));
	//UE_LOG(LogTemp, Log, TEXT("IsSimulatingPhysics: %s"), CutProceduralMesh_1->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"));

	GetMesh()->SetCollisionProfileName("NoCollision");		// 생각해보니 이거 궂이 필요한...가? (+ 초기값은 "Zombie")
															// 어차피 플레이어는 콜리전 프리셋 설정이 메시는 NoCollision, 캡슐은 Pawn 이라서 좀비 메시랑은 항상 충돌 무시되어 왔음 (초기값 Zombie가 Pawn을 무시함)
	GetMesh()->SetGenerateOverlapEvents(false);
	// 기존 SkeletalMesh 안 보이게 설정
	//GetMesh()->SetVisibility(false, true);
	GetMesh()->SetVisibility(false);
	//GetMesh()->SetHiddenInGame(true);


	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("CreativeProceduralMesh END")));

}

void ABaseZombie::SliceProceduralmesh(FVector planeposition, FVector planenormal)
{
	if (CutProceduralMesh_1)
	{

		UProceduralMeshComponent* procHit = Cast<UProceduralMeshComponent>(CutProceduralMesh_1);

		// 이전 CutProceduralMesh_2 초기화 => 다시 자를때 Array Out of Bounds 크래시 에러 발생 방지
		if (CutProceduralMesh_2) {
			CutProceduralMesh_2->DestroyComponent();
			CutProceduralMesh_2 = nullptr;
		}

		UKismetProceduralMeshLibrary::SliceProceduralMesh(
			procHit,
			planeposition,
			planenormal,
			true,
			CutProceduralMesh_2,
			EProcMeshSliceCapOption::CreateNewSectionForCap,
			Material_Blood
		);





		if (CutProceduralMesh_2)
		{
			CutProceduralMesh_2->RegisterComponent(); // 컴포넌트 등록

			// 절단 부위 material 설정
			CutProceduralMesh_2->SetMaterial(CutProceduralMesh_2->GetNumMaterials() - 1, Material_Blood);

			CutProceduralMesh_2->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			CutProceduralMesh_2->SetCollisionProfileName(TEXT("Ragdoll"));
			CutProceduralMesh_2->SetSimulatePhysics(true);

			TArray<FVector> CutSectionVertices;  // 절단된 단면 버텍스 저장


			// 만약 미래에 절단 된 부분들에서 따로 피 생성되도록 -> 피 이펙트를 여러개 생성 할 생각이라면... => 이 코드 사용!
			//bool spawned = false;
			//
			//for (int num = 0; num < BloodFX.Num(); num++) {
			//	//BloodFX[num]->ProcMesh = CutProceduralMesh_?;	
			//	BloodFX[num]->blood_spawnloop = true;
			//	BloodFX[num]->spawn_flag = true;
			//	spawned = true;
			//}
			//
			//if (spawned == false) {
			//	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] BloodFX.Num() < 0 => BloodFX spawn failed!!!")));
			//	UE_LOG(LogTemp, Error, TEXT("[Error] BloodFX.Num() < 0 => BloodFX spawn failed!!!"));
			//}


			//병합부분! ----------------------------------
			//CutPro1 단면 bone 구하는 부분
			//FVector CutPro_1ClusterCenter(0, 0, 0);

			//FProcMeshSection* CutSection1 = CutProceduralMesh_1->GetProcMeshSection(CutProceduralMesh_1->GetNumSections() - 1);
			//if (CutSection1)
			//{
			//	CutPro_1ClusterCenter = FVector(0, 0, 0);
			//	for (const FProcMeshVertex& Vertex : CutSection1->ProcVertexBuffer)
			//	{
			//		CutPro_1ClusterCenter += Vertex.Position;
			//	}

			//	CutPro_1ClusterCenter /= CutSection1->ProcVertexBuffer.Num();

			//	UE_LOG(LogTemp, Warning, TEXT("CutPro_1ClusterCenter : X : %f, Y : %f, Z : %f "), CutPro_1ClusterCenter.X, CutPro_1ClusterCenter.Y, CutPro_1ClusterCenter.Z);
			//}

			//  ----------------------------------
			double StartTime = FPlatformTime::Seconds();
			double EndTime = FPlatformTime::Seconds();

			if (CutProceduralMesh_1) {
				CutPro_1TargetLocation = CutProceduralMesh_1->GetComponentLocation();
				//CutPro_1TargetRotation = CutProceduralMesh_1->GetComponentRotation();

				//FVector ForwardVector = CutProceduralMesh_1->GetForwardVector();  // 현재 바라보는 방향
				FVector ForwardVector = GetActorForwardVector();  // 현재 바라보는 방향
				FVector UpVector = FVector::UpVector;             // 목표 방향 (0,0,1)

				// 현재 바라보는 방향(ForwardVector)을 (0,0,1)로 만들기 위한 회전값 계산
				FRotator NewRotation = FRotationMatrix::MakeFromXZ(-UpVector, ForwardVector).Rotator();
				NewRotation.Yaw += 180.0f;
				//CutPro_1TargetRotation = CutProceduralMesh_1->GetComponentRotation() + NewRotation;
				CutPro_1TargetRotation = NewRotation;

				CutPro_1TargetLocation += ForwardVector * 50.f;
				CutPro_1TargetLocation.Z += 20.f;

			}

			EndTime = FPlatformTime::Seconds();
			UE_LOG(LogTemp, Warning, TEXT("MergeTime took: %f seconds"), EndTime - StartTime);

			TMap<FVector, FVertexBoneData> VertexBoneMap;
			TMap<FName, FVector> BoneAveragePos;
			InitVertexBoneMap(VertexBoneMap, BoneAveragePos);


			TArray<FVector> CutSectionVertices2;

			FProcMeshSection* CutSection2 = CutProceduralMesh_2->GetProcMeshSection(CutProceduralMesh_2->GetNumSections() - 1);
			if (CutSection2)
			{
				for (const FProcMeshVertex& Vertex : CutSection2->ProcVertexBuffer)
				{
					CutSectionVertices2.Add(Vertex.Position);
					//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->CutPlane : X : %f, Y : %f, Z : %f "), Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
				}
				//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->CutPlaneNUM : %d "), CutSectionVertices2.Num());
				
			}

			TArray<int> Labels2;
			int MinPts2 = 3;     // 최소 점 개수
			TMap<int, TArray<TPair<int, FVector>>> ClusteredVertices;
			TMap<int, FVector> ClusterCenters;


			double ALLStartTime = FPlatformTime::Seconds();

			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("SliceProceduralmesh START")));


			StartTime = FPlatformTime::Seconds();

			DBSCANWithAverageDistance(CutSectionVertices2, MinPts2, Labels2, ClusteredVertices);

			EndTime = FPlatformTime::Seconds();

			UE_LOG(LogTemp, Warning, TEXT("DBSCANWithAverageDistance took: %f seconds"), EndTime - StartTime);

			TArray<TArray<TPair<int, FVector>>> RefinedClusters;


			for (TPair<int, TArray<TPair<int, FVector>>>& Cluster : ClusteredVertices)
			{
				TArray<TArray<TPair<int, FVector>>> SubClusters;
				StartTime = FPlatformTime::Seconds();
				// 밀도 기반 그래프 분석 (연결 관계 확인)
				RefineClusterUsingGraph(Cluster.Value, SubClusters);
				EndTime = FPlatformTime::Seconds();

				UE_LOG(LogTemp, Warning, TEXT("RefineClusterUsingGraph took: %f seconds"), EndTime - StartTime);

			
				// 만약 그래프 기반 분리로 충분하지 않다면, K-Means 시도
				if (SubClusters.Num() == 1)
				{
					StartTime = FPlatformTime::Seconds();
					KMeansSplitCluster(Cluster.Value, SubClusters);
					EndTime = FPlatformTime::Seconds();
					UE_LOG(LogTemp, Warning, TEXT("KMeansSplitCluster took: %f seconds"), EndTime - StartTime);
				}

				StartTime = FPlatformTime::Seconds();

				// 분리된 클러스터를 최종 저장 (중복 클러스터 추가 방지)
				for (const auto& SubCluster : SubClusters)
				{
					bool bAlreadyAdded = false;

					// 중복된 클러스터가 있으면 추가하지 않음
					for (const auto& ExistingCluster : RefinedClusters)
					{
						if (ExistingCluster == SubCluster)
						{
							bAlreadyAdded = true;
							break;
						}
					}

					if (!bAlreadyAdded)
					{
						RefinedClusters.Add(SubCluster);
					}
				}


				EndTime = FPlatformTime::Seconds();
				UE_LOG(LogTemp, Warning, TEXT("bAlreadyAdded took: %f seconds"), EndTime - StartTime);
			}

			// 최종적으로 중복되지 않은 클러스터들로 ClusteredVertices 업데이트
			ClusteredVertices.Empty();
			int NewClusterId = 0;
			for (const auto& SubCluster : RefinedClusters)
			{
				ClusteredVertices.Add(NewClusterId++, SubCluster);
			}


			//StartTime = FPlatformTime::Seconds();

			GetVerticesByCluster(ClusteredVertices, ClusterCenters);


			//EndTime = FPlatformTime::Seconds();

			//UE_LOG(LogTemp, Warning, TEXT("GetVerticesByCluster took: %f seconds"), EndTime - StartTime);



			//StartTime = FPlatformTime::Seconds();

			MergeClustersBasedOnBoneName(ClusteredVertices, ClusterCenters, BoneAveragePos);

			//EndTime = FPlatformTime::Seconds();

			//UE_LOG(LogTemp, Warning, TEXT("MergeClustersBasedOnBoneName took: %f seconds"), EndTime - StartTime);


			ClusterCenters.Empty();
			GetVerticesByCluster(ClusteredVertices, ClusterCenters);

			// 섹션
			//TMap<FVector, FVertexBoneData> VertexBoneMap;
			//TMap<FName, FVector> BoneAveragePos;
			TMap<int32, TMap<int32, FName>> SectionVertexBoneMap;
			StartTime = FPlatformTime::Seconds();
			if (ClusterCenters.Num() <= 1) {
				//병합부분!
				m_bIsCutProceduralMesh_2Visibility = true;

				if (CutProceduralMesh_2) {
					CutPro_2TargetLocation = CutProceduralMesh_2->GetComponentLocation();
					//CutPro_2TargetRotation = CutProceduralMesh_2->GetComponentRotation();



					////FVector ForwardVector = CutProceduralMesh_2->GetForwardVector();  // 현재 바라보는 방향
					FVector ForwardVector = GetActorForwardVector();  // 현재 바라보는 방향
					FVector UpVector = FVector::UpVector;             // 목표 방향 (0,0,1)

					// 현재 바라보는 방향(ForwardVector)을 (0,0,1)로 만들기 위한 회전값 계산
					FRotator NewRotation = FRotationMatrix::MakeFromXZ(-UpVector, ForwardVector).Rotator();
					NewRotation.Yaw += 180.0f;
					CutPro_2TargetRotation = NewRotation;
					//CutPro_2TargetRotation = CutProceduralMesh_2->GetComponentRotation() + NewRotation;

				
					CutPro_2TargetLocation += ForwardVector * 50.f;
					CutPro_2TargetLocation.Z += 20.f;

				}


				GetWorld()->GetTimerManager().SetTimer(ZombieMergeWattingHandle, this, &ABaseZombie::StartMergiingTimer, 5.f, false);
				
				return; // CutProceduralMesh_2 그대로 사용
			}
			EndTime = FPlatformTime::Seconds();

			UE_LOG(LogTemp, Warning, TEXT("MergeTime2 took: %f seconds"), EndTime - StartTime);


			//StartTime = FPlatformTime::Seconds();
			//InitVertexBoneMap(VertexBoneMap, BoneAveragePos);

			//EndTime = FPlatformTime::Seconds();
			//UE_LOG(LogTemp, Warning, TEXT("InitVertexBoneMap took: %f seconds"), EndTime - StartTime);

			//StartTime = FPlatformTime::Seconds();
			FillSectionVertexBoneMap(VertexBoneMap, SectionVertexBoneMap, BoneAveragePos);

			//EndTime = FPlatformTime::Seconds();
			//UE_LOG(LogTemp, Warning, TEXT("FillSectionVertexBoneMap took: %f seconds"), EndTime - StartTime);

			//병합부분! ----------------------------------

			//CutPro_1PlaneBoneName = FindNearstBoneName(BoneAveragePos, CutPro_1ClusterCenter);

			//  ----------------------------------

			// 기존 ProceduralMesh가 있으면 삭제
			for (UProceduralMeshComponent* MeshComp : ProceduralMeshes)
			{
				if (MeshComp)
				{
					MeshComp->DestroyComponent(); // 안전하게 제거
				}
			}
			ProceduralMeshes.Empty(); // 배열 초기화
			ProcMeshImpulseStates.Empty();

			int32 CutPlaneIndex = 0;

			int32 ProceduralMeshNewIndex = 2;


			if (ClusterCenters.Num() > 1) { // CutProceduralMesh_2 분해
				for (const TPair<int, FVector>& Cluster : ClusterCenters) {

					TMap<FVector, int32> GlobalVertexMap;

					TArray<FVector> Vertices;
					//TArray<int32> Triangles;
					TArray<FVector> Normals;
					TArray<FVector2D> UVs;
					TArray<FColor> Colors;
					TArray<FProcMeshTangent> Tangents;

					StartTime = FPlatformTime::Seconds();
					FName CutPlaneBoneName = FindNearstBoneName(BoneAveragePos ,Cluster.Value);
					
					//병합부분! ----------------------------------

					//ProcMeshBone.Add(CutPlaneBoneName);

					//  ----------------------------------

					EndTime = FPlatformTime::Seconds();
					UE_LOG(LogTemp, Warning, TEXT("CutPlaneBoneName FindNearstBoneName took: %f seconds"), EndTime - StartTime);

					//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->CutPlaneBoneName : %s "), *CutPlaneBoneName.ToString());

					// 새로운 ProceduralMeshComponent 생성
					UProceduralMeshComponent* NewProcMesh = NewObject<UProceduralMeshComponent>(this);
					NewProcMesh->bUseComplexAsSimpleCollision = false;

					TArray<FVector> Vertices_Convex;
					int interval_Convex = 10;
					int interval_index = 0;
					if (CutPlaneBoneName == "Head" ||
						CutPlaneBoneName == "Neck" ||
						CutPlaneBoneName == "Spine2" ||
						CutPlaneBoneName == "Spine1" ||
						CutPlaneBoneName == "Spine" ||
						CutPlaneBoneName == "Hips") { // SpecialRootBone

						for (int32 SectionIndex = 0; SectionIndex < CutProceduralMesh_2->GetNumSections(); ++SectionIndex)
						{
							FProcMeshSection* Section = CutProceduralMesh_2->GetProcMeshSection(SectionIndex);
							if (!Section) continue;

							TArray<FProcMeshVertex>& SectionVertices = Section->ProcVertexBuffer;
							TArray<uint32>& SectionTriangles = Section->ProcIndexBuffer;
							
							//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->CutPlaneBoneName : %s, SectionIndex : %d "), *CutPlaneBoneName.ToString(), SectionIndex);
							//UE_LOG(LogTemp, Warning, TEXT("SectionVertices : %d "), SectionVertices.Num());
							//UE_LOG(LogTemp, Warning, TEXT("SectionTriangles: %d "), SectionTriangles.Num());

							TMap<int32, int32> LocalToGlobalVertexMap; // 섹션 내 로컬 인덱스 → 글로벌 인덱스 매핑

							if (SectionIndex == CutProceduralMesh_2->GetNumSections() - 1) {
								int32 ClusterVerticesNum = 0;
								
								if (ClusteredVertices.Contains(Cluster.Key))
								{
									StartTime = FPlatformTime::Seconds();
									TArray<TPair<int, FVector>>& ClusterVertices = ClusteredVertices[Cluster.Key]; // TPair<int, FVector> 기반으로 수정
						
									ClusterVerticesNum = ClusterVertices.Num();

									//UE_LOG(LogTemp, Warning, TEXT("Cluster.Key : %d, ClusterVerticesNum : %d "), Cluster.Key,  ClusterVertices.Num());
									//UE_LOG(LogTemp, Warning, TEXT("Cluster.Key : %d, SectionVertices.Num(): %d "), Cluster.Key, SectionVertices.Num());

									for (const TPair<int, FVector>& VertexData : ClusterVertices) {
										int32 OriginalIndex = VertexData.Key;  // 원본 인덱스 가져오기
										FVector VertexPos = SectionVertices[OriginalIndex].Position; // 기존 `i + CutPlaneIndex` 대신 원본 인덱스로 접근

										int32 GlobalIndex;
										if (GlobalVertexMap.Contains(VertexPos)) {
											GlobalIndex = GlobalVertexMap[VertexPos];
										}
										else {
											// 새로운 정점이면 추가 후 인덱스 저장
											GlobalIndex = Vertices.Add(VertexPos);
											GlobalVertexMap.Add(VertexPos, GlobalIndex);

											if (interval_index % interval_Convex == 0) {
												Vertices_Convex.Add(VertexPos);
											}	
											++interval_index;

											// 나머지 속성도 추가
											Normals.Add(SectionVertices[OriginalIndex].Normal);
											UVs.Add(SectionVertices[OriginalIndex].UV0);
											Colors.Add(FColor(0.0, 0.0, 0.0, 255));
											Tangents.Add(SectionVertices[OriginalIndex].Tangent);
										}

										// 로컬 인덱스 -> 글로벌 인덱스 매핑
										LocalToGlobalVertexMap.Add(OriginalIndex, GlobalIndex);
									}

									TArray<int32> Triangles;
									// 삼각형 인덱스 변환
									for (int32 i = 0; i < SectionTriangles.Num(); i += 3)
									{
										if (LocalToGlobalVertexMap.Contains(SectionTriangles[i]) &&
											LocalToGlobalVertexMap.Contains(SectionTriangles[i + 1]) &&
											LocalToGlobalVertexMap.Contains(SectionTriangles[i + 2]))
										{
											int32 GlobalIdx0 = LocalToGlobalVertexMap[SectionTriangles[i]];
											int32 GlobalIdx1 = LocalToGlobalVertexMap[SectionTriangles[i + 1]];
											int32 GlobalIdx2 = LocalToGlobalVertexMap[SectionTriangles[i + 2]];

											Triangles.Add(GlobalIdx0);
											Triangles.Add(GlobalIdx1);
											Triangles.Add(GlobalIdx2);
										}
									}


									//UE_LOG(LogTemp, Warning, TEXT("Triangles Count: %d"), Triangles.Num());
									//for (int32 i = 0; i < Triangles.Num(); i += 3)
									//{
									//	//UE_LOG(LogTemp, Warning, TEXT("Triangle %d: %d, %d, %d"), i / 3, Triangles[i], Triangles[i + 1], Triangles[i + 2]);
									//}

									CutPlaneIndex += ClusterVerticesNum;
									//UE_LOG(LogTemp, Warning, TEXT("ClusterVerticesNum : %d "), CutPlaneIndex);


									FTransform SkeletonTransform = CutProceduralMesh_2->GetComponentTransform();
									NewProcMesh->SetWorldTransform(SkeletonTransform);
									NewProcMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
									// 절단 부위 material 설정
									NewProcMesh->SetMaterial(CutProceduralMesh_2->GetNumMaterials() - 1, Material_ProcMeshBlood);

									Vertices.Empty();
									Normals.Empty();
									UVs.Empty();
									Colors.Empty();
									Tangents.Empty();

									EndTime = FPlatformTime::Seconds();
									UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->GetNumSections() - 1 took: %f seconds"), EndTime - StartTime);
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("Cluster %d has no associated vertices."), Cluster.Key);
								}


							}
							else {
								StartTime = FPlatformTime::Seconds();
								for (int32 i = 0; i < SectionVertices.Num(); i++)
								{
									FVector VertexPos = SectionVertices[i].Position;

									// 해당 버텍스의 본 이름 가져오기
									FName BoneName = SectionVertexBoneMap[SectionIndex][i];
									//UE_LOG(LogTemp, Warning, TEXT("BoneName ::: %s"), *BoneName.ToString());
									//UE_LOG(LogTemp, Warning, TEXT("InBone ::: %d"), InBone(BoneName, CutPlaneBoneName, SpecialRootBone));

									// 특정 본(Bone)의 자식인지 확인
									if (InBone(BoneName, CutPlaneBoneName, SpecialRootBone))
									{
										int32 GlobalIndex;
										if (GlobalVertexMap.Contains(VertexPos))
										{
											GlobalIndex = GlobalVertexMap[VertexPos];
										}
										else
										{
											// 새로운 정점이면 추가 후 인덱스 저장
											GlobalIndex = Vertices.Add(VertexPos);
											GlobalVertexMap.Add(VertexPos, GlobalIndex);

											if (interval_index % interval_Convex == 0) {
												Vertices_Convex.Add(VertexPos);
											}
											++interval_index;

											// 나머지 속성도 추가
											Normals.Add(SectionVertices[i].Normal);
											UVs.Add(SectionVertices[i].UV0);
											Colors.Add(FColor(0.0, 0.0, 0.0, 255));
											Tangents.Add(SectionVertices[i].Tangent);
										}

										LocalToGlobalVertexMap.Add(i, GlobalIndex);
									}
								}
								TArray<int32> Triangles;
								// 삼각형 인덱스 변환
								for (int32 i = 0; i < SectionTriangles.Num(); i += 3)
								{
									if (LocalToGlobalVertexMap.Contains(SectionTriangles[i]) &&
										LocalToGlobalVertexMap.Contains(SectionTriangles[i + 1]) &&
										LocalToGlobalVertexMap.Contains(SectionTriangles[i + 2]))
									{
										int32 GlobalIdx0 = LocalToGlobalVertexMap[SectionTriangles[i]];
										int32 GlobalIdx1 = LocalToGlobalVertexMap[SectionTriangles[i + 1]];
										int32 GlobalIdx2 = LocalToGlobalVertexMap[SectionTriangles[i + 2]];

										Triangles.Add(GlobalIdx0);
										Triangles.Add(GlobalIdx1);
										Triangles.Add(GlobalIdx2);
									}
								}

								//UE_LOG(LogTemp, Warning, TEXT("Triangles Count: %d"), Triangles.Num());

								FTransform SkeletonTransform = CutProceduralMesh_2->GetComponentTransform();
								NewProcMesh->SetWorldTransform(SkeletonTransform);
								NewProcMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
								// 절단 부위 material 설정
								for (int n = 0; n < CutProceduralMesh_2->GetNumMaterials() - 1; n++) {
									NewProcMesh->SetMaterial(n, CutProceduralMesh_2->GetMaterial(n));
								}

								Vertices.Empty();
								Normals.Empty();
								UVs.Empty();
								Colors.Empty();
								Tangents.Empty();

								EndTime = FPlatformTime::Seconds();
								UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->GetNumSections() took: %f seconds"), EndTime - StartTime);
							}

						}



						NewProcMesh->RegisterComponent();

						// 언리얼 에디터에서 보이게 설정
						NewProcMesh->SetVisibility(true);
						NewProcMesh->SetHiddenInGame(false);


						NewProcMesh->AddCollisionConvexMesh(Vertices_Convex);
						Vertices_Convex.Empty();
						interval_index = 0;

						NewProcMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
						NewProcMesh->SetCollisionProfileName(TEXT("Ragdoll"));
						NewProcMesh->SetSimulatePhysics(true);

						// 배열에 추가
						ProceduralMeshes.Add(NewProcMesh);
						ProcMeshImpulseStates.Add(NewProcMesh, false);
						ProceduralMeshNewIndex++;
						//UE_LOG(LogTemp, Warning, TEXT("ProceduralMeshNewIndex : %d"), ProceduralMeshNewIndex);

					}
					else {  // RootBone

						///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

						for (int32 SectionIndex = 0; SectionIndex < CutProceduralMesh_2->GetNumSections(); ++SectionIndex)
						{
							FProcMeshSection* Section = CutProceduralMesh_2->GetProcMeshSection(SectionIndex);
							if (!Section) continue;

							TArray<FProcMeshVertex>& SectionVertices = Section->ProcVertexBuffer;
							TArray<uint32>& SectionTriangles = Section->ProcIndexBuffer;

							//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->CutPlaneBoneName : %s, SectionIndex : %d "), *CutPlaneBoneName.ToString(), SectionIndex);
							//UE_LOG(LogTemp, Warning, TEXT("SectionVertices : %d "), SectionVertices.Num());
							//UE_LOG(LogTemp, Warning, TEXT("SectionTriangles: %d "), SectionTriangles.Num());

							TMap<int32, int32> LocalToGlobalVertexMap; // 섹션 내 로컬 인덱스 → 글로벌 인덱스 매핑

							if (SectionIndex == CutProceduralMesh_2->GetNumSections() - 1) {
								int32 ClusterVerticesNum = 0;
								if (ClusteredVertices.Contains(Cluster.Key))
								{
									//TArray<FVector>& ClusterVertices = ClusteredVertices[Cluster.Key];
									TArray<TPair<int, FVector>>& ClusterVertices = ClusteredVertices[Cluster.Key]; // TPair<int, FVector> 기반으로 수정

									ClusterVerticesNum = ClusterVertices.Num();

									//UE_LOG(LogTemp, Warning, TEXT("Cluster.Key : %d, ClusterVerticesNum : %d "), Cluster.Key, ClusterVertices.Num());
									//UE_LOG(LogTemp, Warning, TEXT("Cluster.Key : %d, SectionVertices.Num(): %d "), Cluster.Key, SectionVertices.Num());
									for (const TPair<int, FVector>& VertexData : ClusterVertices) {
										int32 OriginalIndex = VertexData.Key;  // 원본 인덱스 가져오기
										FVector VertexPos = SectionVertices[OriginalIndex].Position; // 기존 `i + CutPlaneIndex` 대신 원본 인덱스로 접근

										int32 GlobalIndex;
										if (GlobalVertexMap.Contains(VertexPos)) {
											GlobalIndex = GlobalVertexMap[VertexPos];
										}
										else {
											// 새로운 정점이면 추가 후 인덱스 저장
											GlobalIndex = Vertices.Add(VertexPos);
											GlobalVertexMap.Add(VertexPos, GlobalIndex);

											if (interval_index % interval_Convex == 0) {
												Vertices_Convex.Add(VertexPos);
											}
											++interval_index;

											// 나머지 속성도 추가
											Normals.Add(SectionVertices[OriginalIndex].Normal);
											UVs.Add(SectionVertices[OriginalIndex].UV0);
											Colors.Add(FColor(0.0, 0.0, 0.0, 255));
											Tangents.Add(SectionVertices[OriginalIndex].Tangent);
										}

										// 로컬 인덱스 -> 글로벌 인덱스 매핑
										LocalToGlobalVertexMap.Add(OriginalIndex, GlobalIndex);
									}

									TArray<int32> Triangles;
									// 삼각형 인덱스 변환
									for (int32 i = 0; i < SectionTriangles.Num(); i += 3)
									{
										if (LocalToGlobalVertexMap.Contains(SectionTriangles[i]) &&
											LocalToGlobalVertexMap.Contains(SectionTriangles[i + 1]) &&
											LocalToGlobalVertexMap.Contains(SectionTriangles[i + 2]))
										{
											int32 GlobalIdx0 = LocalToGlobalVertexMap[SectionTriangles[i]];
											int32 GlobalIdx1 = LocalToGlobalVertexMap[SectionTriangles[i + 1]];
											int32 GlobalIdx2 = LocalToGlobalVertexMap[SectionTriangles[i + 2]];

											Triangles.Add(GlobalIdx0);
											Triangles.Add(GlobalIdx1);
											Triangles.Add(GlobalIdx2);
										}
									}

									//UE_LOG(LogTemp, Warning, TEXT("Triangles Count: %d"), Triangles.Num());

									CutPlaneIndex += ClusterVerticesNum;
									//UE_LOG(LogTemp, Warning, TEXT("ClusterVerticesNum : %d "), CutPlaneIndex);


									FTransform SkeletonTransform = CutProceduralMesh_2->GetComponentTransform();
									NewProcMesh->SetWorldTransform(SkeletonTransform);
									NewProcMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
									// 절단 부위 material 설정
									NewProcMesh->SetMaterial(CutProceduralMesh_2->GetNumMaterials() - 1, Material_ProcMeshBlood);

									Vertices.Empty();
									Normals.Empty();
									UVs.Empty();
									Colors.Empty();
									Tangents.Empty();
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("Cluster %d has no associated vertices."), Cluster.Key);
								}


							}
							else {

								for (int32 i = 0; i < SectionVertices.Num(); i++)
								{
									FVector VertexPos = SectionVertices[i].Position;



									// 해당 버텍스의 본 이름 가져오기
									FName BoneName = SectionVertexBoneMap[SectionIndex][i];

									//UE_LOG(LogTemp, Warning, TEXT("BoneName ::: %s"), *BoneName.ToString());
									//UE_LOG(LogTemp, Warning, TEXT("InBone ::: %d"), InBone(BoneName, CutPlaneBoneName, RootBone));

									// 특정 본(Bone)의 자식인지 확인
									if (InBone(BoneName, CutPlaneBoneName, RootBone))
									{
										int32 GlobalIndex;
										if (GlobalVertexMap.Contains(VertexPos))
										{
											GlobalIndex = GlobalVertexMap[VertexPos];
										}
										else
										{
											// 새로운 정점이면 추가 후 인덱스 저장
											GlobalIndex = Vertices.Add(VertexPos);
											GlobalVertexMap.Add(VertexPos, GlobalIndex);

											if (interval_index % interval_Convex == 0) {
												Vertices_Convex.Add(VertexPos);
											}
											++interval_index;

											// 나머지 속성도 추가
											Normals.Add(SectionVertices[i].Normal);
											UVs.Add(SectionVertices[i].UV0);
											Colors.Add(FColor(0.0, 0.0, 0.0, 255));
											Tangents.Add(SectionVertices[i].Tangent);
										}

										LocalToGlobalVertexMap.Add(i, GlobalIndex);
									}
								}
								TArray<int32> Triangles;
								// 삼각형 인덱스 변환
								for (int32 i = 0; i < SectionTriangles.Num(); i += 3)
								{
									if (LocalToGlobalVertexMap.Contains(SectionTriangles[i]) &&
										LocalToGlobalVertexMap.Contains(SectionTriangles[i + 1]) &&
										LocalToGlobalVertexMap.Contains(SectionTriangles[i + 2]))
									{
										int32 GlobalIdx0 = LocalToGlobalVertexMap[SectionTriangles[i]];
										int32 GlobalIdx1 = LocalToGlobalVertexMap[SectionTriangles[i + 1]];
										int32 GlobalIdx2 = LocalToGlobalVertexMap[SectionTriangles[i + 2]];

										Triangles.Add(GlobalIdx0);
										Triangles.Add(GlobalIdx1);
										Triangles.Add(GlobalIdx2);
									}
								}


								//UE_LOG(LogTemp, Warning, TEXT("Triangles Count: %d"), Triangles.Num());
								//for (int32 i = 0; i < Triangles.Num(); i += 3)
								//{
								//	//UE_LOG(LogTemp, Warning, TEXT("Triangle %d: %d, %d, %d"), i / 3, Triangles[i], Triangles[i + 1], Triangles[i + 2]);
								//}


								FTransform SkeletonTransform = CutProceduralMesh_2->GetComponentTransform();
								NewProcMesh->SetWorldTransform(SkeletonTransform);
								NewProcMesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
								// 절단 부위 material 설정
								for (int n = 0; n < CutProceduralMesh_2->GetNumMaterials() - 1; n++) {
									NewProcMesh->SetMaterial(n, CutProceduralMesh_2->GetMaterial(n));
								}
								Vertices.Empty();
								Normals.Empty();
								UVs.Empty();
								Colors.Empty();
								Tangents.Empty();
							}

						}


						// 새로운 ProceduralMeshComponent 생성
						//UProceduralMeshComponent* NewProcMesh = NewObject<UProceduralMeshComponent>(this);

						//NewProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);

						NewProcMesh->RegisterComponent();

						NewProcMesh->AddCollisionConvexMesh(Vertices_Convex);
						Vertices_Convex.Empty();
						interval_index = 0;

						NewProcMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
						NewProcMesh->SetCollisionProfileName(TEXT("Ragdoll"));
						NewProcMesh->SetSimulatePhysics(true);

						// 언리얼 에디터에서 보이게 설정
						NewProcMesh->SetVisibility(true);
						NewProcMesh->SetHiddenInGame(false);

						// 배열에 추가
						ProceduralMeshes.Add(NewProcMesh);
						ProcMeshImpulseStates.Add(NewProcMesh, false);

						ProceduralMeshNewIndex++;
						//UE_LOG(LogTemp, Warning, TEXT("ProceduralMeshNewIndex : %d"), ProceduralMeshNewIndex);
					}

				}
			}
			CutProceduralMesh_2->SetVisibility(false);
			CutProceduralMesh_2->DestroyComponent();
			CutProceduralMesh_2 = nullptr;
			//CutProceduralMesh_2->SetHiddenInGame(true);
			//CutProceduralMesh_2->bUseComplexAsSimpleCollision = false;

			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("SliceProceduralmesh END")));


			double AllEndTime = FPlatformTime::Seconds();
			UE_LOG(LogTemp, Warning, TEXT("NSlice took: %f seconds"), AllEndTime - ALLStartTime);


			//병합부분! ----------------------------------
			StartTime = FPlatformTime::Seconds();

			m_bIsCutProceduralMesh_2Visibility = false;
			ProcMeshMergeTargetLocation.SetNum(ProceduralMeshes.Num());
			ProcMeshMergeTargetRotation.SetNum(ProceduralMeshes.Num());

			for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
			{
				UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
				if (!ProcMesh) continue;
				FVector ProcForard = ProcMesh->GetForwardVector();
				//UE_LOG(LogTemp, Warning, TEXT("ProcForardVector : X: %f , Y : %f, Z: %f"), ProcForard.X, ProcForard.Y, ProcForard.Z);

				ProcMeshMergeTargetLocation[MeshIndex] = ProcMesh->GetComponentLocation();
				//ProcMeshMergeTargetRotation[MeshIndex] = ProcMesh->GetComponentRotation();


				////FVector ForwardVector = ProcMesh->GetForwardVector();  // 현재 바라보는 방향
				FVector ForwardVector = GetActorForwardVector();  // 현재 바라보는 방향
				FVector UpVector = FVector::UpVector;             // 목표 방향 (0,0,1)

				// 현재 바라보는 방향(ForwardVector)을 (0,0,1)로 만들기 위한 회전값 계산
				FRotator NewRotation = FRotationMatrix::MakeFromXZ(-UpVector, ForwardVector).Rotator();
				NewRotation.Yaw += 180.0f;
				ProcMeshMergeTargetRotation[MeshIndex] = NewRotation;
				//ProcMeshMergeTargetRotation[MeshIndex] = ProcMesh->GetComponentRotation() + NewRotation;

				ProcMeshMergeTargetLocation[MeshIndex] += ForwardVector * 50.f;
				ProcMeshMergeTargetLocation[MeshIndex].Z += 20.f;

			}
			EndTime = FPlatformTime::Seconds();
			UE_LOG(LogTemp, Warning, TEXT("MergeTimeProc took: %f seconds"), EndTime - StartTime);

			//StartTime = FPlatformTime::Seconds();
		

			//// vertexpos - sectionindex, vertexindex    promesh들 for문 돌면서 TArray<TMap<ZombieMeshData(SectionIndex,Vertexindex), VertexIndex>>에 기록 
			//// Cut1도 해줘야 한다.
			//ReportProVertexMap(VertexBoneMap); 

			//EndTime = FPlatformTime::Seconds();
			//UE_LOG(LogTemp, Warning, TEXT("CutProceduralMesh_2->ReportProVertexMap took: %f seconds"), EndTime - StartTime);


			//// 좀비 머지 5초후 시작
			GetWorld()->GetTimerManager().SetTimer(ZombieMergeWattingHandle, this, &ABaseZombie::StartMergiingTimer, 5.f, false);

			//  ----------------------------------
		}


	}

	// 생각해보니까 CutPro2가 있을때는 다른 FTimer와 함수를 사용해야 할 것 같다

	// 메쉬 병합 방법 정리1
	// 절단 시 5초간 기다리기 합칠 때 10초 시간
	// 프로시저럴 메쉬 섹션인덱스 버텍스 인덱스 맵으로 기록해두기
	// 스켈레탈 메쉬 섹션인덱스 버텍스 인덱스 맵으로 기록하고 프로시저럴 메쉬 버텍스랑 스켈레탈 메쉬 버텍스 둘이 거리 기록
	// 문제는 단면 버텍스랑 똑같은 위치가 없는 버텍스가 문제이긴 한데 이 부분 해결 필요
	// 거리 시간에 따라 다가오게 한 다음 시간 다 되면 프로시저럴 메쉬 전체 삭제 후 스켈레탈 메쉬 보이게 하기
	// 초기화 필요한 부분들 초기화 하기


	// 메쉬 병합 방법 정리2
	// 프로시저럴 메쉬 bonename 저장 해놓기
	// bonename 저장해놓은 것을 이용해서 bone 월드 좌표랑 회전 좌표 계산
	// 도착하면 프로시저럴 메쉬 없애고 스켈레탈 메쉬 부활 시키기
}


void ABaseZombie::InitVertexBoneMap(TMap<FVector, FVertexBoneData>& VertexBoneMap, TMap<FName, FVector>& BoneAveragePos)
{
	VertexBoneMap.Empty(); // 기존 데이터 초기화
	BoneAveragePos.Empty();

	USkeletalMeshComponent* Skeleton = GetMesh();
	if (!Skeleton) return;

	FSkeletalMeshRenderData* SkMeshRenderData = Skeleton->GetSkeletalMeshRenderData();
	if (!SkMeshRenderData || SkMeshRenderData->LODRenderData.Num() == 0) return;

	const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[0]; // LOD 0 기준
	FSkinWeightVertexBuffer& SkinWeights = *Skeleton->GetSkinWeightBuffer(0);

	TMap<FName, FVector> BonePositionSum;  
	TMap<FName, int32> BoneVertexCount;    

	// 모든 섹션 순회
	for (int32 SectionIndex = 0; SectionIndex < DataArray.RenderSections.Num(); SectionIndex++)
	{
		const FSkelMeshRenderSection& Section = DataArray.RenderSections[SectionIndex];
		const int32 NumSourceVertices = Section.NumVertices;
		const int32 BaseVertexIndex = Section.BaseVertexIndex;

		for (int32 i = 0; i < NumSourceVertices; i++)
		{
			const int32 VertexIndex = i + BaseVertexIndex;
			const FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(Skeleton, VertexIndex, DataArray, SkinWeights);
			FVector VertexPosition = FVector(SkinnedVectorPos);

			uint32 k = uint32(i);

			// 본 인덱스 가져오기
			if (k >= SkinWeights.GetNumVertices()) continue;
			int32 BoneIndex = SkinWeights.GetBoneIndex(k, 0);

			if (BoneIndex >= Section.BoneMap.Num()) continue;
			int32 ActualBone = Section.BoneMap[BoneIndex];

			// 본 이름 가져오기
			FName BoneName = Skeleton->GetBoneName(ActualBone);

			// 맵에 저장 (버텍스 위치 -> {섹션 인덱스, 버텍스 인덱스, 본 이름})
			VertexBoneMap.Add(VertexPosition, FVertexBoneData(SectionIndex, i, BoneName));

			// 본 위치 합과 개수 업데이트
			if (BonePositionSum.Contains(BoneName))
			{
				BonePositionSum[BoneName] += VertexPosition;
				BoneVertexCount[BoneName]++;
			}
			else
			{
				BonePositionSum.Add(BoneName, VertexPosition);
				BoneVertexCount.Add(BoneName, 1);
			}
		}
	}

	// 본 이름별 평균 위치 계산
	for (const TPair<FName, FVector>& Pair : BonePositionSum)
	{
		FName BoneName = Pair.Key;
		FVector PositionSum = Pair.Value;
		int32 Count = BoneVertexCount[BoneName];

		if (Count > 0)
		{
			BoneAveragePos.Add(BoneName, PositionSum / Count);
		}
	}


	//UE_LOG(LogTemp, Log, TEXT("VertexBoneMap initialized with %d vertices"), VertexBoneMap.Num());
}

void ABaseZombie::InitVertexIndexPosMapForMerge(TMap<FZombieMeshData, FVector>& VertexIndexPosMap)
{
	VertexIndexPosMap.Empty(); // 기존 데이터 초기화

	USkeletalMeshComponent* Skeleton = GetMesh();
	if (!Skeleton) return;

	FSkeletalMeshRenderData* SkMeshRenderData = Skeleton->GetSkeletalMeshRenderData();
	if (!SkMeshRenderData || SkMeshRenderData->LODRenderData.Num() == 0) return;

	const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[0]; // LOD 0 기준
	FSkinWeightVertexBuffer& SkinWeights = *Skeleton->GetSkinWeightBuffer(0);

	TMap<FName, FVector> BonePositionSum;
	TMap<FName, int32> BoneVertexCount;

	// 모든 섹션 순회
	for (int32 SectionIndex = 0; SectionIndex < DataArray.RenderSections.Num(); SectionIndex++)
	{
		const FSkelMeshRenderSection& Section = DataArray.RenderSections[SectionIndex];
		const int32 NumSourceVertices = Section.NumVertices;
		const int32 BaseVertexIndex = Section.BaseVertexIndex;

		for (int32 i = 0; i < NumSourceVertices; i++)
		{
			const int32 VertexIndex = i + BaseVertexIndex;
			const FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(Skeleton, VertexIndex, DataArray, SkinWeights);
			FVector VertexPosition = FVector(SkinnedVectorPos);
			FVector WorldPosition = Skeleton->GetComponentTransform().TransformPosition(VertexPosition);
			uint32 k = uint32(i);

			// 본 인덱스 가져오기
			if (k >= SkinWeights.GetNumVertices()) continue;


			// 맵에 저장 (버텍스 위치 -> {섹션 인덱스, 버텍스 인덱스, 본 이름})
			VertexIndexPosMap.Add(FZombieMeshData(SectionIndex, i), WorldPosition);

		}
	}



	//UE_LOG(LogTemp, Log, TEXT("VertexBoneMap initialized with %d vertices"), VertexBoneMap.Num());
}

void ABaseZombie::FillSectionVertexBoneMap(const TMap<FVector, FVertexBoneData>& VertexBoneMap, TMap<int32, TMap<int32, FName>>& SectionVertexBoneMap, TMap<FName, FVector>& BoneAveragePos)
{
	SectionVertexBoneMap.Empty(); // 초기화

	if (!CutProceduralMesh_2) return;

	for (int32 SectionIndex = 0; SectionIndex < CutProceduralMesh_2->GetNumSections() - 1; ++SectionIndex)
	{
		FProcMeshSection* Section = CutProceduralMesh_2->GetProcMeshSection(SectionIndex);
		if (!Section) continue;

		TArray<FProcMeshVertex>& SectionVertices = Section->ProcVertexBuffer;

		for (int32 i = 0; i < SectionVertices.Num(); i++)
		{
			// 버텍스 위치로 검색
			if (const FVertexBoneData* BoneData = VertexBoneMap.Find(SectionVertices[i].Position))
			{
				// 맵에 저장 (섹션 인덱스 -> 버텍스 인덱스 -> 본 이름)
				SectionVertexBoneMap.FindOrAdd(SectionIndex).Add(i, BoneData->BoneName);

				//UE_LOG(LogTemp, Warning, TEXT("Section: %d, Vertex: %d, Bone: %s"),
				//	SectionIndex, i, *BoneData->BoneName.ToString());
			}
			else {

				FName ClosestBoneName = FindNearstBoneName(BoneAveragePos, SectionVertices[i].Position);
				SectionVertexBoneMap.FindOrAdd(SectionIndex).Add(i, ClosestBoneName);
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("SectionIndex : %d , CutProceduralMesh_2->VertexNum : %d "), SectionIndex, SectionVertices.Num());
	}
}

FName ABaseZombie::FindNearstBoneName(TMap<FName, FVector>& BoneAveragePos, const FVector& TargetPosition)
{
	FName ClosestBoneName;
	float MinDistanceSq = FLT_MAX;

	for (const auto& Pair : BoneAveragePos)
	{
		float DistanceSq = FVector::DistSquared(Pair.Value, TargetPosition);
		if (DistanceSq < MinDistanceSq)
		{
			MinDistanceSq = DistanceSq;
			ClosestBoneName = Pair.Key;
		}
	}

	return ClosestBoneName;
}


float ABaseZombie::CalculateEuclideanDistance(const FVector& Point1, const FVector& Point2)
{
	return FVector::Dist(Point1, Point2);  // FVector의 Dist 함수 사용
}

float ABaseZombie::CalculateAverageDistance(const TArray<FVector>& Vertices)
{
	float TotalDistance = 0.0f;
	int Count = 0;
	for (int i = 0; i < Vertices.Num(); ++i)
	{
		for (int j = i + 1; j < Vertices.Num(); ++j)
		{
			TotalDistance += FVector::Dist(Vertices[i], Vertices[j]);
			Count++;
		}
	}
	return TotalDistance / Count;
}

float ABaseZombie::CalculateDynamicEps(const TArray<FVector>& Vertices, int K)
{
	TArray<float> Distances;

	for (int i = 0; i < Vertices.Num(); ++i)
	{
		TArray<float> NeighborDistances;

		for (int j = 0; j < Vertices.Num(); ++j)
		{
			if (i != j)
			{
				NeighborDistances.Add(CalculateEuclideanDistance(Vertices[i], Vertices[j]));
			}
		}

		NeighborDistances.Sort();
		if (NeighborDistances.Num() >= K)
		{
			Distances.Add(NeighborDistances[K - 1]); // K번째 최근접 이웃 거리 추가
		}
	}

	// 거리의 중앙값을 Eps로 사용
	Distances.Sort();
	return (Distances.Num() > 0) ? Distances[Distances.Num() / 2] : 0.0f;
}

void ABaseZombie::DBSCANWithAverageDistance(const TArray<FVector>& Vertices, int MinPts, TArray<int>& Labels, TMap<int, TArray<TPair<int, FVector>>>& ClusteredVertices)
{
	int ClusterId = 0;
	Labels.SetNumUninitialized(Vertices.Num());
	for (int& Label : Labels)
	{
		Label = -1;
	}

	// Eps 계산 및 제한
	float Eps = CalculateDynamicEps(Vertices, MinPts) * 3.f;
	float EpsSquared = Eps * Eps;
	//UE_LOG(LogTemp, Warning, TEXT("DBSCANWithAverageDistance - Eps: %f"), Eps);

	for (int i = 0; i < Vertices.Num(); ++i)
	{
		if (Labels[i] != -1) continue;

		// 이웃 탐색 (DistSquared 사용)
		TArray<int> Neighbors;
		for (int j = 0; j < Vertices.Num(); ++j)
		{
			if (i != j && FVector::DistSquared(Vertices[i], Vertices[j]) <= EpsSquared)
			{
				Neighbors.Add(j);
			}
		}

		if (Neighbors.Num() >= MinPts)
		{
			ClusterId++;
			Labels[i] = ClusterId;
			ClusteredVertices.FindOrAdd(ClusterId).Add(TPair<int, FVector>(i, Vertices[i]));

			// TSet 사용하여 SeedSet 관리
			TSet<int> Visited;
			TArray<int> SeedSet = Neighbors;
			Visited.Append(Neighbors);

			int Index = 0;
			while (Index < SeedSet.Num())
			{
				int CurrentIdx = SeedSet[Index];

				if (Labels[CurrentIdx] == -1)
				{
					Labels[CurrentIdx] = ClusterId;
					ClusteredVertices.FindOrAdd(ClusterId).Add(TPair<int, FVector>(CurrentIdx, Vertices[CurrentIdx]));
				}

				// 새로운 이웃 탐색
				TArray<int> NewNeighbors;
				for (int j = 0; j < Vertices.Num(); ++j)
				{
					if (CurrentIdx != j && FVector::DistSquared(Vertices[CurrentIdx], Vertices[j]) <= EpsSquared)
					{
						NewNeighbors.Add(j);
					}
				}

				if (NewNeighbors.Num() >= MinPts)
				{
					for (int NewIdx : NewNeighbors)
					{
						if (!Visited.Contains(NewIdx))
						{
							SeedSet.Add(NewIdx);
							Visited.Add(NewIdx);
						}
					}
				}

				Index++;
			}
		}
	}
}

void ABaseZombie::MergeClustersBasedOnBoneName(TMap<int, TArray<TPair<int, FVector>>>& ClusteredVertices, TMap<int, FVector>& ClusterCenters, TMap<FName, FVector>& BoneAveragePos)
{
	// FName들을 비교하기 위해서 ClusterCenters를 TMap<int, FName>로 변환
	TMap<int, FName> ClusterBoneNames;

	// 각 클러스터의 중심을 기준으로 CutPlane의 BoneName을 구하고 저장
	for (const TPair<int, FVector>& Cluster : ClusterCenters)
	{
		FName CutPlaneBoneName = FindNearstBoneName(BoneAveragePos, Cluster.Value);
		ClusterBoneNames.Add(Cluster.Key, CutPlaneBoneName);
	}

	TMap<FName, TArray<int>> BoneNameToClusterIds;

	for (const TPair<int, FName>& Cluster : ClusterBoneNames)
	{
		if (Cluster.Value.IsNone())  // BoneName이 비어 있는 경우 방어 코드 추가
		{
			UE_LOG(LogTemp, Error, TEXT("ClusterBoneNames contains an empty BoneName for ClusterID: %d"), Cluster.Key);
			continue;
		}

		if (!BoneNameToClusterIds.Contains(Cluster.Value))
		{
			BoneNameToClusterIds.Add(Cluster.Value, TArray<int>());
		}

		TArray<int>& ClusterList = BoneNameToClusterIds.FindOrAdd(Cluster.Value);
		ClusterList.Add(Cluster.Key);

		//UE_LOG(LogTemp, Warning, TEXT("Added ClusterID %d to BoneName %s"), Cluster.Key, *Cluster.Value.ToString());
	}

	// 동일한 FName을 가진 클러스터들끼리 합침 (우선순위 고려 X)
	for (const TPair<FName, TArray<int>>& BoneClusterPair : BoneNameToClusterIds)
	{
		const FName& BoneName = BoneClusterPair.Key;
		const TArray<int>& ClusterIds = BoneClusterPair.Value;

		// 먼저 같은 FName을 가진 클러스터들을 합침
		if (ClusterIds.Num() > 1)
		{
			TArray<TPair<int, FVector>> MergedVertices;

			// 클러스터들을 합침
			for (int ClusterId : ClusterIds)
			{
				if (ClusteredVertices.Contains(ClusterId))
				{
					const TArray<TPair<int, FVector>>& VerticesInCluster = ClusteredVertices[ClusterId];
					MergedVertices.Append(VerticesInCluster);  // 버텍스 합침
				}
			}

			// 합쳐진 버텍스를 새 클러스터로 저장
			int FirstClusterId = ClusterIds[0];
			ClusteredVertices[FirstClusterId] = MergedVertices;

			// 나머지 클러스터는 삭제
			for (int i = 1; i < ClusterIds.Num(); ++i)
			{
				ClusteredVertices.Remove(ClusterIds[i]);
			}

			// 합쳐진 클러스터의 중심 재계산
			FVector NewClusterCenter(0, 0, 0);
			for (const TPair<int, FVector>& Vertex : MergedVertices)
			{
				NewClusterCenter += Vertex.Value;
			}
			if (MergedVertices.Num() > 0)
			{
				NewClusterCenter /= MergedVertices.Num();
			}

			// 클러스터 중심 정보 갱신
			ClusterCenters[FirstClusterId] = NewClusterCenter;

			// **병합된 클러스터의 새로운 BoneName을 계산하여 업데이트**
			FName NewBoneName = FindNearstBoneName(BoneAveragePos, NewClusterCenter);
			ClusterBoneNames[FirstClusterId] = NewBoneName;
		}
	}

	// 병합 우선순위 그룹들 정의
	TArray<TArray<FName>> MergeConditionBoneGroups = {
		{ FName(TEXT("Head")), FName(TEXT("Neck")), FName(TEXT("Spine2")), FName(TEXT("Spine1")), FName(TEXT("Spine")), FName(TEXT("Hips")) },
		{ FName(TEXT("LeftShoulder")), FName(TEXT("LeftArm")), FName(TEXT("LeftForeArm")), FName(TEXT("LeftHand")) },
		{ FName(TEXT("RightShoulder")), FName(TEXT("RightArm")), FName(TEXT("RightForeArm")), FName(TEXT("RightHand")) },
		{ FName(TEXT("LeftUpLeg")), FName(TEXT("LeftLeg")), FName(TEXT("LeftFoot")), FName(TEXT("LeftToeBase")) },
		{ FName(TEXT("RightUpLeg")), FName(TEXT("RightLeg")), FName(TEXT("RightFoot")), FName(TEXT("RightToeBase")) }
	};

	// === 1차 병합 ===
	for (const TArray<FName>& MergeConditionBoneNames : MergeConditionBoneGroups)
	{
		FName HighestPriorityBoneName;
		int HighestPriorityIndex = MergeConditionBoneNames.Num();
		int TargetClusterId = -1;

		// 가장 높은 우선순위를 가진 클러스터 찾기
		for (const TPair<int, FName>& Cluster : ClusterBoneNames)
		{
			int Index = MergeConditionBoneNames.IndexOfByKey(Cluster.Value);
			if (Index != INDEX_NONE && Index < HighestPriorityIndex)
			{
				HighestPriorityBoneName = Cluster.Value;
				HighestPriorityIndex = Index;
				TargetClusterId = Cluster.Key;
			}
		}

		// 병합 대상 확인
		if (!HighestPriorityBoneName.IsNone() && TargetClusterId != -1)
		{
			TArray<int> ClustersToMerge;
			TArray<TPair<int, FVector>> MergedVertices;

			for (const TPair<int, FName>& Cluster : ClusterBoneNames)
			{
				int Index = MergeConditionBoneNames.IndexOfByKey(Cluster.Value);
				if (Index != INDEX_NONE && Index >= HighestPriorityIndex)
				{
					ClustersToMerge.Add(Cluster.Key);
				}
			}

			// 병합 수행
			for (int ClusterId : ClustersToMerge)
			{
				if (ClusteredVertices.Contains(ClusterId))
				{
					const TArray<TPair<int, FVector>>& VerticesInCluster = ClusteredVertices[ClusterId];
					MergedVertices.Append(VerticesInCluster);
				}
			}

			// 병합 결과 반영
			if (TargetClusterId != -1 && MergedVertices.Num() > 0)
			{
				if (ClusteredVertices.Contains(TargetClusterId))
				{
					ClusteredVertices[TargetClusterId] = MergedVertices;

					for (int ClusterId : ClustersToMerge)
					{
						if (ClusterId != TargetClusterId)
						{
							ClusteredVertices.Remove(ClusterId);
							ClusterBoneNames.Remove(ClusterId);
						}
					}

					FVector NewClusterCenter(0, 0, 0);
					for (const TPair<int, FVector>& Vertex : MergedVertices)
					{
						NewClusterCenter += Vertex.Value;
					}
					ClusterCenters[TargetClusterId] = NewClusterCenter / MergedVertices.Num();
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid TargetClusterId %d"), TargetClusterId);
				}
			}
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("==== Cluster Bone Names After Merge ===="));
	//for (const TPair<int, FName>& Cluster : ClusterBoneNames)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Cluster ID1: %d, Bone Name: %s"), Cluster.Key, *Cluster.Value.ToString());
	//}

	// === 2차 병합 (Hips 기준 추가 병합) ===
	TArray<int> ClustersToMerge2;

	for (const TPair<int, FName>& Cluster : ClusterBoneNames)
	{
		FName ClBoneName = Cluster.Value;
		if (ClBoneName == FName(TEXT("LeftUpLeg")) || ClBoneName == FName(TEXT("RightUpLeg")))
		{
			int LegClusterId = Cluster.Key;

			// 병합할 Hips와 Spine1, Spine 클러스터 찾기
			TArray<int> PotentialHipsSpineClusterIds;

			for (const TPair<int, FName>& InnerCluster : ClusterBoneNames)
			{
				FName InnerClBoneName = InnerCluster.Value;
				if (InnerClBoneName == FName(TEXT("Spine1")) || InnerClBoneName == FName(TEXT("Spine")) || InnerClBoneName == FName(TEXT("Hips")))
				{
					PotentialHipsSpineClusterIds.Add(InnerCluster.Key);
				}
			}

			// 병합할 준비가 되었는지 확인
			for (int HipsSpineClusterId : PotentialHipsSpineClusterIds)
			{
				if (ClusteredVertices.Contains(LegClusterId) && ClusteredVertices.Contains(HipsSpineClusterId))
				{
					// 병합할 클러스터들 기록
					ClustersToMerge2.Add(HipsSpineClusterId);
				}
			}
		}
	}

	// 병합할 클러스터들 처리
	for (int HipsSpineClusterId : ClustersToMerge2)
	{
		int LegClusterId = -1;

		// Leg 클러스터 찾기 (Hips/Spine과 병합)
		for (const TPair<int, FName>& Cluster : ClusterBoneNames)
		{
			if (Cluster.Value == FName(TEXT("LeftUpLeg")) || Cluster.Value == FName(TEXT("RightUpLeg")))
			{
				LegClusterId = Cluster.Key;
				break;
			}
		}

		// 병합 작업 수행
		if (LegClusterId != -1 && ClusteredVertices.Contains(LegClusterId) && ClusteredVertices.Contains(HipsSpineClusterId))
		{
			// Hips 또는 Spine 클러스터에 Leg 클러스터의 버텍스 추가
			TArray<TPair<int, FVector>>& TargetVertices = ClusteredVertices[HipsSpineClusterId];
			const TArray<TPair<int, FVector>>& LegVertices = ClusteredVertices[LegClusterId];

			// Leg 클러스터의 모든 버텍스를 Target(Hips/Spine) 클러스터에 추가
			TargetVertices.Append(LegVertices);

			// 병합된 Leg 클러스터 삭제
			ClusteredVertices.Remove(LegClusterId);
			ClusterBoneNames.Remove(LegClusterId);

			// 새로운 중심점 계산
			FVector NewCenter(0, 0, 0);
			for (const TPair<int, FVector>& Vertex : TargetVertices)
			{
				NewCenter += Vertex.Value;
			}

			// 새로운 중심점 재계산 (버텍스 개수로 나눠서 평균 계산)
			if (TargetVertices.Num() > 0)
			{
				NewCenter /= TargetVertices.Num();
			}

			// 새로운 중심점 저장
			ClusterCenters[HipsSpineClusterId] = NewCenter;

			//UE_LOG(LogTemp, Log, TEXT("Merged Leg Cluster %d into Cluster %d (Hips/Spine)"), LegClusterId, HipsSpineClusterId);
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("==== Cluster Bone Names After Merge ===="));
	//for (const TPair<int, FName>& Cluster : ClusterBoneNames)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Cluster ID2: %d, Bone Name: %s"), Cluster.Key, *Cluster.Value.ToString());
	//}
}

void ABaseZombie::RefineClusterUsingGraph(const TArray<TPair<int, FVector>>& ClusterPoints, TArray<TArray<TPair<int, FVector>>>& SeparatedClusters)
{
	TMap<int, TArray<int>> Graph;
	TMap<int, int> GlobalToLocalIndexMap; // 전역 인덱스 → 로컬 인덱스 매핑
	float Threshold = 2.0f; // 거리 임계값

	// 전역 인덱스와 로컬 인덱스를 매핑
	for (int i = 0; i < ClusterPoints.Num(); i++)
	{
		GlobalToLocalIndexMap.Add(ClusterPoints[i].Key, i);
	}

	// 그래프 생성 (전역 인덱스 사용)
	for (int i = 0; i < ClusterPoints.Num(); i++)
	{
		for (int j = i + 1; j < ClusterPoints.Num(); j++)
		{
			if (FVector::Dist(ClusterPoints[i].Value, ClusterPoints[j].Value) < Threshold)
			{
				int GlobalKeyI = ClusterPoints[i].Key;
				int GlobalKeyJ = ClusterPoints[j].Key;

				Graph.FindOrAdd(GlobalKeyI).Add(GlobalKeyJ);
				Graph.FindOrAdd(GlobalKeyJ).Add(GlobalKeyI);
			}
		}
	}

	TArray<TPair<int, FVector>> OrphanedVertices;
	TSet<int> Visited;

	for (const auto& Point : ClusterPoints)
	{
		if (Visited.Contains(Point.Key)) continue;

		TArray<TPair<int, FVector>> SubCluster;
		TArray<int> Stack = { Point.Key };

		while (Stack.Num() > 0)
		{
			int CurrentGlobalKey = Stack.Pop();

			// 방문한 노드는 다시 추가하지 않음
			if (Visited.Contains(CurrentGlobalKey)) continue;
			Visited.Add(CurrentGlobalKey);

			// GlobalKey를 로컬 인덱스로 변환
			int* LocalIndexPtr = GlobalToLocalIndexMap.Find(CurrentGlobalKey);
			if (!LocalIndexPtr)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid GlobalKey access: %d"), CurrentGlobalKey);
				continue;
			}
			int LocalIndex = *LocalIndexPtr;

			// 중복 추가 방지
			if (!SubCluster.ContainsByPredicate([&](const TPair<int, FVector>& Existing) {
				return Existing.Key == CurrentGlobalKey;
				}))
			{
				SubCluster.Add(TPair<int, FVector>(CurrentGlobalKey, ClusterPoints[LocalIndex].Value));
			}

			if (Graph.Contains(CurrentGlobalKey))
			{
				for (int Neighbor : Graph[CurrentGlobalKey])
				{
					if (!Visited.Contains(Neighbor))
						Stack.Add(Neighbor);
				}
			}
		}

		if (SubCluster.Num() > 0)
		{
			SeparatedClusters.Add(SubCluster);
		}
		else
		{
			OrphanedVertices.Add(Point);
		}
	}

	// OrphanedVertices 병합 (고립된 버텍스를 모든 클러스터의 버텍스와 비교)
	for (auto& Orphaned : OrphanedVertices)
	{
		bool bAdded = false;

		for (auto& ExistingCluster : SeparatedClusters)
		{
			for (const auto& Vertex : ExistingCluster)
			{
				if (FVector::Dist(Vertex.Value, Orphaned.Value) < Threshold)
				{
					ExistingCluster.Add(Orphaned);
					bAdded = true;
					break; // 버텍스를 추가했으면 해당 클러스터 검사는 종료
				}
			}

			if (bAdded) break; // 이미 병합된 경우, 다음 클러스터 검사 생략
		}

		// 가장 가까운 클러스터가 없다면 새로운 클러스터로 추가
		if (!bAdded)
		{
			SeparatedClusters.Add({ Orphaned });
		}
	}

	//// OrphanedVertices 병합 (고립된 버텍스를 기존 클러스터에 추가)
	//for (auto& Orphaned : OrphanedVertices)
	//{
	//	bool bAdded = false;

	//	for (auto& ExistingCluster : SeparatedClusters)
	//	{

	//		if (FVector::Dist(ExistingCluster[0].Value, Orphaned.Value) < Threshold)
	//		{
	//			ExistingCluster.Add(Orphaned);
	//			bAdded = true;
	//			break;
	//		}
	//	}

	//	// 가장 가까운 클러스터가 없다면 새로운 클러스터로 추가
	//	if (!bAdded)
	//	{
	//		SeparatedClusters.Add({ Orphaned });
	//	}
	//}

	// 최종 클러스터 개수 출력
	//UE_LOG(LogTemp, Warning, TEXT("Total Clusters: %d"), SeparatedClusters.Num());
}

void ABaseZombie::KMeansSplitCluster(TArray<TPair<int, FVector>>& ClusterPoints,TArray<TArray<TPair<int, FVector>>>& SeparatedClusters)
{
	if (ClusterPoints.Num() < 5)
	{
		SeparatedClusters.Add(ClusterPoints);
		return;
	}

	// K-Means를 2개 클러스터로 적용
	TArray<FVector> Centroids = { ClusterPoints[0].Value, ClusterPoints.Last().Value };
	TMap<int, TArray<TPair<int, FVector>>> NewClusters;
	const float MinCentroidDistance = 1.0f;  // 센트로이드 최소 거리

	for (int Iter = 0; Iter < 5; Iter++)
	{
		NewClusters.Empty();

		// 각 점을 가까운 중심에 할당
		for (const TPair<int, FVector>& Point : ClusterPoints)
		{
			float Dist0 = CalculateEuclideanDistance(Point.Value, Centroids[0]);
			float Dist1 = CalculateEuclideanDistance(Point.Value, Centroids[1]);

			if (Dist0 < Dist1)
				NewClusters.FindOrAdd(0).Add(Point);
			else
				NewClusters.FindOrAdd(1).Add(Point);
		}

		// 새 중심 계산
		for (int i = 0; i < 2; i++)
		{
			if (!NewClusters.Contains(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("NewClusters does not contain index %d"), i);
				continue;
			}


			FVector NewCentroid(0, 0, 0);
			for (const auto& P : NewClusters[i])
				NewCentroid += P.Value;

			if (NewClusters[i].Num() > 0)
				NewCentroid /= NewClusters[i].Num();

			// 두 센트로이드 간의 거리가 너무 가깝다면, 더 이상 분리하지 않음
			if (CalculateEuclideanDistance(Centroids[i], NewCentroid) < MinCentroidDistance)
			{
				// 센트로이드가 너무 가까우면, 이 둘을 하나로 합침
				SeparatedClusters.Add(NewClusters[i]);
				return;
			}

			Centroids[i] = NewCentroid;
		}
	}

	// 분리된 클러스터 저장
	for (const auto& Cluster : NewClusters)
	{
		SeparatedClusters.Add(Cluster.Value);
	}
}

void ABaseZombie::GetVerticesByCluster(const TMap<int, TArray<TPair<int, FVector>>>& ClusteredVerticesMap, TMap<int, FVector>& ClusterCenters)
{
	for (const TPair<int, TArray<TPair<int, FVector>>>& Cluster : ClusteredVerticesMap)
	{
		const TArray<TPair<int, FVector>>& VerticesInCluster = Cluster.Value;
		FVector ClusterCenter(0, 0, 0);

		for (const TPair<int, FVector>& VertexPair : VerticesInCluster)
		{
			ClusterCenter += VertexPair.Value;
		}

		if (VerticesInCluster.Num() > 0)
		{
			ClusterCenter /= VerticesInCluster.Num();
		}

		ClusterCenters.Add(Cluster.Key, ClusterCenter);

		//UE_LOG(LogTemp, Warning, TEXT("Cluster %d has %d vertices"), Cluster.Key, VerticesInCluster.Num());
		//UE_LOG(LogTemp, Warning, TEXT("Cluster %d Center: %s"), Cluster.Key, *ClusterCenter.ToString());
	}
}


void ABaseZombie::CreateAndApplyBoundingBox(UProceduralMeshComponent* ProceduralMesh)
{
	// 1. 메시의 바운딩 박스를 가져옴
	FBox BoundingBox = ProceduralMesh->CalcBounds(ProceduralMesh->GetComponentTransform()).GetBox();

	// 2. 바운딩 박스의 크기와 중심 계산
	FVector BoxExtent = BoundingBox.GetExtent();  // 반경 (반쪽 크기)
	FVector BoxCenter = BoundingBox.GetCenter(); // 중심 위치

	// 디버깅용: 바운딩 박스 정보를 출력
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
	//	FString::Printf(TEXT("ProceduralMesh has NO ConvexMesh!! ====> BoundingBox Created!!!\nBoundingBox Extent: %s, Center: %s"),
	//		*BoxExtent.ToString(), *BoxCenter.ToString()));
	UE_LOG(LogTemp, Log, TEXT("ProceduralMesh has NO ConvexMesh!! ====> BoundingBox Created!!!\nBoundingBox Extent: %s, Center: %s"),
		*BoxExtent.ToString(), *BoxCenter.ToString());

	// 3. 충돌 설정 - 기존 Convex Mesh를 대체
	ProceduralMesh->ClearCollisionConvexMeshes(); // 기존 충돌 제거

	// 4. 바운딩 박스 크기를 기반으로 충돌 형상 생성
	TArray<FVector> ConvexVerts;

	// 바운딩 박스의 8개 꼭짓점 계산
	ConvexVerts.Add(BoundingBox.Min);
	ConvexVerts.Add(FVector(BoundingBox.Max.X, BoundingBox.Min.Y, BoundingBox.Min.Z));
	ConvexVerts.Add(FVector(BoundingBox.Min.X, BoundingBox.Max.Y, BoundingBox.Min.Z));
	ConvexVerts.Add(FVector(BoundingBox.Max.X, BoundingBox.Max.Y, BoundingBox.Min.Z));
	ConvexVerts.Add(FVector(BoundingBox.Min.X, BoundingBox.Min.Y, BoundingBox.Max.Z));
	ConvexVerts.Add(FVector(BoundingBox.Max.X, BoundingBox.Min.Y, BoundingBox.Max.Z));
	ConvexVerts.Add(FVector(BoundingBox.Min.X, BoundingBox.Max.Y, BoundingBox.Max.Z));
	ConvexVerts.Add(BoundingBox.Max);

	// 각 벡터 값 출력
	for (int32 i = 0; i < ConvexVerts.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("ConvexMesh Convex Vert %d: %s"), i, *ConvexVerts[i].ToString());
	}

	// 새로운 Convex Mesh를 적용
	ProceduralMesh->AddCollisionConvexMesh(ConvexVerts);

	// 5. 충돌 다시 활성화
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProceduralMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

	// 6. 물리 시뮬레이션 활성화
	ProceduralMesh->SetSimulatePhysics(true);
}

void ABaseZombie::CreateAndApplyBoundingBoxByNewProcMesh(UProceduralMeshComponent* ProceduralMesh)
{
	// 1. 메시의 바운딩 박스를 가져옴
	FBox BoundingBox = ProceduralMesh->CalcBounds(ProceduralMesh->GetComponentTransform()).GetBox();

	// 2. 바운딩 박스의 크기와 중심 계산
	FVector BoxExtent = BoundingBox.GetExtent();  // 반경 (반쪽 크기)
	FVector BoxCenter = BoundingBox.GetCenter(); // 중심 위치

	/*UE_LOG(LogTemp, Log, TEXT("ProceduralMesh has NO ConvexMesh!! ====> BoundingBox Created!!!\nBoundingBox Extent: %s, Center: %s"),
		*BoxExtent.ToString(), *BoxCenter.ToString());*/

	// 3. 충돌 설정 - 기존 Convex Mesh를 대체
	ProceduralMesh->ClearCollisionConvexMeshes(); // 기존 충돌 제거

	// 4. 바운딩 박스 크기를 기반으로 충돌 형상 생성
	TArray<FVector> ConvexVerts;

	// 바운딩 박스의 8개 꼭짓점 계산
	FVector Min = BoundingBox.Min;
	FVector Max = BoundingBox.Max;

	// 월드 좌표를 로컬 좌표로 변환
	FVector LocalMin = ProceduralMesh->GetComponentTransform().Inverse().TransformPosition(Min);
	FVector LocalMax = ProceduralMesh->GetComponentTransform().Inverse().TransformPosition(Max);

	// 로컬 좌표 시스템에서 ConvexVerts 생성
	ConvexVerts.Add(LocalMin);  // 0
	ConvexVerts.Add(FVector(LocalMax.X, LocalMin.Y, LocalMin.Z));  // 1
	ConvexVerts.Add(FVector(LocalMin.X, LocalMax.Y, LocalMin.Z));  // 2
	ConvexVerts.Add(FVector(LocalMax.X, LocalMax.Y, LocalMin.Z));  // 3
	ConvexVerts.Add(FVector(LocalMin.X, LocalMin.Y, LocalMax.Z));  // 4
	ConvexVerts.Add(FVector(LocalMax.X, LocalMin.Y, LocalMax.Z));  // 5
	ConvexVerts.Add(FVector(LocalMin.X, LocalMax.Y, LocalMax.Z));  // 6
	ConvexVerts.Add(LocalMax);  // 7

	//// 각 벡터 값 출력
	//for (int32 i = 0; i < ConvexVerts.Num(); ++i)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("ConvexMesh Convex Vert %d: %s"), i, *ConvexVerts[i].ToString());
	//}

	// 새로운 Convex Mesh를 적용
	ProceduralMesh->AddCollisionConvexMesh(ConvexVerts);

	// 5. 충돌 다시 활성화
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProceduralMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

	// 6. 물리 시뮬레이션 활성화
	ProceduralMesh->SetSimulatePhysics(true);
}

// vertexpos - sectionindex, vertexindex    promesh들 for문 돌면서 TArray<TMap<ZombieMeshData(SectionIndex,Vertexindex), VertexIndex>>에 기록 
void ABaseZombie::ReportProVertexMap(TMap<FVector, FVertexBoneData>& VertexBoneMap)
{
	ProMeshIndexArray.Empty();
	int32 i = 0;
	for (UProceduralMeshComponent* ProcMesh : ProceduralMeshes)
	{
		TMap<FZombieMeshData, FZombieMeshData> SectionVertexMap;
		
		// 각 섹션별 버텍스 가져오기
		int32 NumSections = ProcMesh->GetNumSections();
		for (int32 SectionIndex = 0; SectionIndex < NumSections - 1; ++SectionIndex)
		{
			FProcMeshSection* MeshSection = ProcMesh->GetProcMeshSection(SectionIndex);
			if (!MeshSection) continue;

			TArray<FProcMeshVertex>& SectionVertices = MeshSection->ProcVertexBuffer;

			// 버텍스 순회
			for (int32 VertexIndex = 0; VertexIndex < SectionVertices.Num(); ++VertexIndex)
			{
				const FVector& VertexPos = SectionVertices[VertexIndex].Position;

				// VertexBoneMap에 해당 버텍스가 존재하면 저장
				if (FVertexBoneData* BoneData = VertexBoneMap.Find(VertexPos))
				{
					// SectionIndex와 VertexIndex를 Key로 사용
					FZombieMeshData SectionVertexKey(BoneData->SectionIndex, BoneData->VertexIndex);
					FZombieMeshData ProIndexVertexKey(i, VertexIndex);
					SectionVertexMap.Add(SectionVertexKey, ProIndexVertexKey);
				}
				else { // 존재안할때 여기서 뭔가를 해줘야 하는가 아니면 5초뒤에 할때 해줘야 하는가..
					
				}
			}
		}

		// 이 메쉬의 결과 추가
		ProMeshIndexArray.Add(SectionVertexMap);
		++i;
	}

	UE_LOG(LogTemp, Log, TEXT("Processed %d Procedural Meshes"), ProceduralMeshes.Num());
}

void ABaseZombie::RotateFromCutProc1MeshToSkel()
{
	if (!CutProceduralMesh_1)
	{
		return;
	}

	CutProceduralMesh_1->SetSimulatePhysics(false);

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	if (!SkeletalMeshComp) return;

	CutPro_1StartRotation = CutProceduralMesh_1->GetComponentRotation();

	//CutProceduralMesh_1->SetWorldRotation(CutPro_1TargetRotation);


	CutPro_1StartLocation = CutProceduralMesh_1->GetComponentLocation();
	CutPro_1Distance = CutPro_1TargetLocation - CutPro_1StartLocation;


	CutProceduralMesh_1->SetWorldRotation(CutPro_1StartRotation);
}

void ABaseZombie::RotateFromCutProc2MeshToSkel()
{
	if (!CutProceduralMesh_2)
	{
		return;
	}

	CutProceduralMesh_2->SetSimulatePhysics(false);

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	if (!SkeletalMeshComp) return;

	CutPro_2StartRotation = CutProceduralMesh_2->GetComponentRotation();

	//CutProceduralMesh_2->SetWorldRotation(CutPro_2TargetRotation);


	CutPro_2StartLocation = CutProceduralMesh_2->GetComponentLocation();
	CutPro_2Distance = CutPro_2TargetLocation - CutPro_2StartLocation;
}

void ABaseZombie::RotateFromProcMeshToSkel()
{
	ProcMeshMergeStartLocation.SetNum(ProceduralMeshes.Num());
	ProcMeshDistance.SetNum(ProceduralMeshes.Num());
	ProcMeshMergeStartRotation.SetNum(ProceduralMeshes.Num());

	for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
	{
		UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
		if (!ProcMesh) continue;

		ProcMeshMergeStartRotation[MeshIndex] = ProcMesh->GetComponentRotation();

		//ProcMesh->SetWorldRotation(ProcMeshMergeTargetRotation[MeshIndex]);
		//ProcMesh->ClearCollisionConvexMeshes();
		ProcMesh->SetSimulatePhysics(false);

		FVector StartLocation = ProcMesh->GetComponentLocation();
		ProcMeshMergeStartLocation[MeshIndex] = StartLocation;

		if (MeshIndex < ProcMeshMergeTargetLocation.Num())
		{
			ProcMeshDistance[MeshIndex] = ProcMeshMergeTargetLocation[MeshIndex] - StartLocation;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ProcMeshMergeTargetLocation is out of bounds for MeshIndex %d"), MeshIndex);
		}
	}
}

void ABaseZombie::RotateFromCutProc1MeshToSkelBone()
{
	if (!CutProceduralMesh_1) 
	{
		return;
	}

	CutProceduralMesh_1->SetSimulatePhysics(false);

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	if (!SkeletalMeshComp) return;

	// 본의 월드 트랜스폼(위치, 회전, 스케일) 가져오기
	FTransform BoneTransform = SkeletalMeshComp->GetBoneTransform(SkeletalMeshComp->GetBoneIndex(CutPro_1PlaneBoneName));


	// 컴포넌트 트랜스폼 가져오기 (스켈레탈 메쉬의 월드 트랜스폼)
	//FTransform ComponentTransform = SkeletalMeshComp->GetComponentTransform();

	// 월드 트랜스폼 계산
	//FTransform BoneWorldTransform = BoneTransform * ComponentTransform;

	// 본의 월드 회전값
	FRotator BoneWorldRotation = BoneTransform.GetRotation().Rotator();
	//FRotator BoneWorldRotation = BoneWorldTransform.GetRotation().Rotator();

	// Roll(roll) 값 100도 추가
	BoneWorldRotation.Roll += 280.0f;

	CutProceduralMesh_1->SetWorldRotation(BoneWorldRotation);

	//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(ProcMeshBone[MeshIndex]).ToString(), *BoneWorldRotation1.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(CutPro_1PlaneBoneName).ToString(), *BoneWorldRotation.ToString());

	// 본의 월드 위치 구하기
	FVector BoneWorldLocation = SkeletalMeshComp->GetBoneLocation(CutPro_1PlaneBoneName, EBoneSpaces::WorldSpace);
	CutPro_1StartLocation = CutProceduralMesh_1->GetComponentLocation();
	CutPro_1Distance = BoneWorldLocation - CutPro_1StartLocation;
}

void ABaseZombie::RotateFromCutProc2MeshToSkelBone()
{

	if (!CutProceduralMesh_2)
	{
		return;
	}

	CutProceduralMesh_2->SetSimulatePhysics(false);

	USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
	if (!SkeletalMeshComp) return;

	// 본의 월드 트랜스폼(위치, 회전, 스케일) 가져오기
	FTransform BoneTransform = SkeletalMeshComp->GetBoneTransform(SkeletalMeshComp->GetBoneIndex(CutPro_2PlaneBoneName));


	// 컴포넌트 트랜스폼 가져오기 (스켈레탈 메쉬의 월드 트랜스폼)
	//FTransform ComponentTransform = SkeletalMeshComp->GetComponentTransform();

	// 월드 트랜스폼 계산
	//FTransform BoneWorldTransform = BoneTransform * ComponentTransform;

	// 본의 월드 회전값
	FRotator BoneWorldRotation = BoneTransform.GetRotation().Rotator();
	//FRotator BoneWorldRotation = BoneWorldTransform.GetRotation().Rotator();

	// Roll(roll) 값 100도 추가
	BoneWorldRotation.Roll += 280.0f;

	CutProceduralMesh_2->SetWorldRotation(BoneWorldRotation);

	//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(ProcMeshBone[MeshIndex]).ToString(), *BoneWorldRotation1.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(CutPro_2PlaneBoneName).ToString(), *BoneWorldRotation.ToString());

	// 본의 월드 위치 구하기
	FVector BoneWorldLocation = SkeletalMeshComp->GetBoneLocation(CutPro_2PlaneBoneName, EBoneSpaces::WorldSpace);
	CutPro_2StartLocation = CutProceduralMesh_2->GetComponentLocation();
	CutPro_2Distance = BoneWorldLocation - CutPro_2StartLocation;
}

void ABaseZombie::RotateFromProcMeshToSkelBone()
{
	for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
	{
		UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
		if (!ProcMesh) continue;

		USkeletalMeshComponent* SkeletalMeshComp = GetMesh();
		if (!SkeletalMeshComp) return;

		// 본의 월드 트랜스폼(위치, 회전, 스케일) 가져오기
		FTransform BoneTransform = SkeletalMeshComp->GetBoneTransform(SkeletalMeshComp->GetBoneIndex(ProcMeshBone[MeshIndex]));
		
		
		// 컴포넌트 트랜스폼 가져오기 (스켈레탈 메쉬의 월드 트랜스폼)
		//FTransform ComponentTransform = SkeletalMeshComp->GetComponentTransform();

		// 월드 트랜스폼 계산
		//FTransform BoneWorldTransform = BoneTransform * ComponentTransform;
		
		// 본의 월드 회전값
		FRotator BoneWorldRotation = BoneTransform.GetRotation().Rotator();
		//FRotator BoneWorldRotation = BoneWorldTransform.GetRotation().Rotator();

		// Roll(roll) 값 100도 추가
		BoneWorldRotation.Roll += 280.0f;

		ProcMesh->SetWorldRotation(BoneWorldRotation);

		//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(ProcMeshBone[MeshIndex]).ToString(), *BoneWorldRotation1.ToString());
		//UE_LOG(LogTemp, Warning, TEXT("Bone %s World Rotation: %s"), *(ProcMeshBone[MeshIndex]).ToString(), *BoneWorldRotation.ToString());
	}
}

void ABaseZombie::SetCutProc1MeshPivotToCenter()
{

	if (!CutProceduralMesh_1) return;

	int32 NumSections = CutProceduralMesh_1->GetNumSections();
	if (NumSections == 0) return;

	// 마지막 섹션 가져오기
	FProcMeshSection* LastSection = CutProceduralMesh_1->GetProcMeshSection(NumSections - 1);
	if (!LastSection) return;

	FVector Center = FVector::ZeroVector;
	int32 VertexCount = 0;

	// 마지막 섹션의 버텍스 평균 계산
	for (const FProcMeshVertex& Vertex : LastSection->ProcVertexBuffer)
	{
		Center += Vertex.Position;
		VertexCount++;
	}

	// 버텍스의 평균 = 중심점
	if (VertexCount > 0)
	{
		Center /= VertexCount;

		// 각 섹션 버텍스들을 중심점 기준으로 이동
		for (int32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
		{
			FProcMeshSection* MeshSection = CutProceduralMesh_1->GetProcMeshSection(SectionIndex);
			if (!MeshSection) continue;

			TArray<FVector> AdjustedVertices;
			TArray<FVector> Normals;
			TArray<FVector2D> UVs;
			TArray<FColor> Colors;
			TArray<FProcMeshTangent> Tangents;

			for (const FProcMeshVertex& Vertex : MeshSection->ProcVertexBuffer)
			{
				AdjustedVertices.Add(Vertex.Position - Center); // 피봇 보정
				Normals.Add(Vertex.Normal);
				UVs.Add(Vertex.UV0);
				Colors.Add(Vertex.Color);
				Tangents.Add(Vertex.Tangent);
			}

			// 이동된 버텍스로 메쉬 갱신
			CutProceduralMesh_1->UpdateMeshSection(SectionIndex, AdjustedVertices, Normals, UVs, Colors, Tangents);
		}

		// 컴포넌트 자체를 중심점으로 이동 (피봇 보정)
		CutProceduralMesh_1->SetWorldLocation(CutProceduralMesh_1->GetComponentLocation() + Center);
	}
}

void ABaseZombie::SetCutProc2MeshPivotToCenter()
{
	if (!CutProceduralMesh_2) return;

	int32 NumSections = CutProceduralMesh_2->GetNumSections();
	if (NumSections == 0) return;

	// 마지막 섹션 가져오기
	FProcMeshSection* LastSection = CutProceduralMesh_2->GetProcMeshSection(NumSections - 1);
	if (!LastSection) return;

	FVector Center = FVector::ZeroVector;
	int32 VertexCount = 0;

	// 마지막 섹션의 버텍스 평균 계산
	for (const FProcMeshVertex& Vertex : LastSection->ProcVertexBuffer)
	{
		Center += Vertex.Position;
		VertexCount++;
	}

	// 버텍스의 평균 = 중심점
	if (VertexCount > 0)
	{
		Center /= VertexCount;

		// 각 섹션 버텍스들을 중심점 기준으로 이동
		for (int32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
		{
			FProcMeshSection* MeshSection = CutProceduralMesh_2->GetProcMeshSection(SectionIndex);
			if (!MeshSection) continue;

			TArray<FVector> AdjustedVertices;
			TArray<FVector> Normals;
			TArray<FVector2D> UVs;
			TArray<FColor> Colors;
			TArray<FProcMeshTangent> Tangents;

			for (const FProcMeshVertex& Vertex : MeshSection->ProcVertexBuffer)
			{
				AdjustedVertices.Add(Vertex.Position - Center); // 피봇 보정
				Normals.Add(Vertex.Normal);
				UVs.Add(Vertex.UV0);
				Colors.Add(Vertex.Color);
				Tangents.Add(Vertex.Tangent);
			}

			// 이동된 버텍스로 메쉬 갱신
			CutProceduralMesh_2->UpdateMeshSection(SectionIndex, AdjustedVertices, Normals, UVs, Colors, Tangents);
		}

		// 컴포넌트 자체를 중심점으로 이동 (피봇 보정)
		CutProceduralMesh_2->SetWorldLocation(CutProceduralMesh_2->GetComponentLocation() + Center);
	}
}

void ABaseZombie::SetProceduralMeshPivotToCenter() // 피봇을 절단 부분으로 옮기는데는 성공했으나 여전히 회전이 이상, 그리고 충돌박스도 옮겨지지 않음(이건 옮겨야하나 고민) 
{
	for (int32 MeshIndex = 0; MeshIndex < ProceduralMeshes.Num(); ++MeshIndex)
	{
		UProceduralMeshComponent* ProcMesh = ProceduralMeshes[MeshIndex];
		if (!ProcMesh) return;

		int32 NumSections = ProcMesh->GetNumSections();
		if (NumSections == 0) return;

		// 마지막 섹션 가져오기
		FProcMeshSection* LastSection = ProcMesh->GetProcMeshSection(NumSections - 1);
		if (!LastSection) return;

		FVector Center = FVector::ZeroVector;
		int32 VertexCount = 0;

		// 마지막 섹션의 버텍스 평균 계산
		for (const FProcMeshVertex& Vertex : LastSection->ProcVertexBuffer)
		{
			Center += Vertex.Position;
			VertexCount++;
		}

		// 버텍스의 평균 = 중심점
		if (VertexCount > 0)
		{
			Center /= VertexCount;

			// 각 섹션 버텍스들을 중심점 기준으로 이동
			for (int32 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
			{
				FProcMeshSection* MeshSection = ProcMesh->GetProcMeshSection(SectionIndex);
				if (!MeshSection) continue;

				TArray<FVector> AdjustedVertices;
				TArray<FVector> Normals;
				TArray<FVector2D> UVs;
				TArray<FColor> Colors;
				TArray<FProcMeshTangent> Tangents;

				for (const FProcMeshVertex& Vertex : MeshSection->ProcVertexBuffer)
				{
					AdjustedVertices.Add(Vertex.Position - Center); // 피봇 보정
					Normals.Add(Vertex.Normal);
					UVs.Add(Vertex.UV0);
					Colors.Add(Vertex.Color);
					Tangents.Add(Vertex.Tangent);
				}

				// 이동된 버텍스로 메쉬 갱신
				ProcMesh->UpdateMeshSection(SectionIndex, AdjustedVertices, Normals, UVs, Colors, Tangents);
			}

			// 컴포넌트 자체를 중심점으로 이동 (피봇 보정)
			ProcMesh->SetWorldLocation(ProcMesh->GetComponentLocation() + Center);
		}
	}
}

void ABaseZombie::StartMergiingTimer()
{
	if (m_bIsCutProceduralMesh_2Visibility) {
		RotateFromCutProc1MeshToSkel();
		RotateFromCutProc2MeshToSkel();
	}
	else {
		RotateFromCutProc1MeshToSkel();
		RotateFromProcMeshToSkel();
	}
	bIsMergingInProgress = true;
	ElapsedTime = 0.0; // 시간 초기화

	// 부활 이펙트 생성
	UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Script/Niagara.NiagaraSystem'/Game/Basic_VFX/Niagara/NS_Basic_5.NS_Basic_5'"));
	if (NiagaraSystem)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, "Found Resurrect FX!");
		UE_LOG(LogTemp, Log, TEXT("Found Cutting Dead Resurrect FX!"));

		ResurrectFX = GetWorld()->SpawnActor<AResurrectNiagaEffect>(AResurrectNiagaEffect::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);

		ResurrectFX->ResurrectFXSystem = NiagaraSystem;

		ResurrectFX->spawn_flag = true;
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, "Couldn't Find Resurrect FX!");
		UE_LOG(LogTemp, Error, TEXT("Couldn't Find Resurrect FX!"));
	}

}

void ABaseZombie::MergingTimerElapsed()
{
}

void ABaseZombie::Attack(uint32 PlayerId)
{
	if (m_bIsAttacking) {
		return;
	}

	if (m_bBeAttacked) {
		return;
	}

	auto AnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());

	AnimInstance->PlayAttackMontage();

	m_bIsAttacking = true;

	if (GetZombieName() == "NormalZombie") {
		AZombieAIController* NormalZombieAIController = Cast<AZombieAIController>(GetController());

		NormalZombieAIController->attackPlayerID = PlayerId;
	}
	else if (GetZombieName() == "RunningZombie") {
		ARunningZombieAIController* RunningZombieAIController = Cast<ARunningZombieAIController>(GetController());

		RunningZombieAIController->attackPlayerID = PlayerId;
	}
	else if (GetZombieName() == "ShoutingZombie") {
		AShoutingZombieAIController* ShoutingZombieAIController = Cast<AShoutingZombieAIController>(GetController());

		ShoutingZombieAIController->attackPlayerID = PlayerId;
	}
}

void ABaseZombie::AttackMontageEnded(UAnimMontage* Montage, bool interrup)
{
	m_bIsAttacking = false;

	afterAnim_idleDuration = afterAnim_idleInterpol;

	m_DAttackEnd.Broadcast();
}

// 좀비가 플레이어 공격할 때 충돌체크
void ABaseZombie::AttackCheck()
{
	// 이미 죽은 상태일때는 때리지 못하도록
	if (GetHP() <= 0)
		return;

	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);
	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * m_fAttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(m_fAttackRadius),
		Params
	);

	// debug 용(충돌 범위 확인 용)
	/*FVector TraceVec = GetActorForwardVector() * m_fAttackRange;
	FVector Center = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight = m_fAttackRange * 0.5f + 50.f;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		m_fAttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime);*/

	if (bResult) {
		ABaseCharacter* player = Cast<ABaseCharacter>(HitResult.GetActor());
			
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Hit Actor")));
		UE_LOG(LogNet, Display, TEXT("Player #%d got hit - current HP: %f"), player->GetPlayerId(), player->GetHP());
		FDamageEvent DamageEvent;
		HitResult.GetActor()->TakeDamage(GetSTR(), DamageEvent, GetController(), this);
	}
}

void ABaseZombie::Shouting()
{
	if (m_bIsShouting) {
		UE_LOG(LogTemp, Error, TEXT("[Shouting] 실행X -> 이미 샤우팅 중입니다!"));

		return;
	}
	if(m_bIsShouted){ 
		UE_LOG(LogTemp, Error, TEXT("[Shouting] 실행X -> 이미 샤우팅 했습니다!"));

		return;
	}

	// 샤우팅 이펙트 생성
	ShoutingFX = GetWorld()->SpawnActor<AShoutingNiagaEffect>(AShoutingNiagaEffect::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);

	auto AnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());

	AnimInstance->PlayShoutingMontage();

	AnimInstance->SetCurrentPawnSpeed(0);

	m_bIsShouting = true;


	UE_LOG(LogTemp, Log, TEXT("IsShouted?: %s"), m_bIsShouted ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("PLAY SHOUTINHG!!!!!"));
}

void ABaseZombie::ShoutingMontageEnded(UAnimMontage* Montage, bool interrup)
{
	//if (m_bIsShouting)
	//	UE_LOG(LogTemp, Log, TEXT("bIsShouted true"));
	
	m_bIsShouting = false;
	m_bIsShouted = true;

	afterAnim_idleDuration = afterAnim_idleInterpol;

	m_DShoutingEnd.Broadcast();

	if (ShoutingFX != NULL) {
		ShoutingFX->EndPlay(EEndPlayReason::Destroyed);
	}

}

void ABaseZombie::BeAttacked()
{
	if (m_bBeAttacked || GetHP() <= 0) {
		return;
	}

	auto AnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());

	if (m_bIsAttacking || m_bIsShouting) {	// 다른 애니메이션 재생 중이였다면 멈추기
		AnimInstance->Montage_Stop(0.5f);	// 0.5초의 애니메이션 blend time
	}

	AnimInstance->PlayBeAttackedMontage();

	GetCharacterMovement()->MaxWalkSpeed = 1.f;
	m_bBeAttacked = true;
}

void ABaseZombie::BeAttackedMontageEnded(UAnimMontage* Montage, bool interrup)
{
	m_bBeAttacked = false;
	GetCharacterMovement()->MaxWalkSpeed = GetSpeed() * 100.f;

	doAction_takeDamage_onTick = false;

	afterAnim_idleDuration = afterAnim_idleInterpol;

	//if (BloodFX != NULL) {
	//	BloodFX->EndPlay(EEndPlayReason::Destroyed);
	//}
}

void ABaseZombie::SetZombieId(uint32 NewZombieId)
{
	ZombieId = NewZombieId;
}

uint32 ABaseZombie::GetZombieId() const
{
	return ZombieId;
}

void ABaseZombie::UpdateZombieData(FVector Location)
{
	NewLocation = Location;
}

// 사망 후 되살아나기 딜레이 타이머
void ABaseZombie::StartResurrectionTimer()
{
	// 타이머 중복 방지 (기존 타이머 제거) => 두번째 부활때 바로 일어나는 애니메이션 재생되는 거 방지
	GetWorld()->GetTimerManager().ClearTimer(ResurrectionHandle);

	if (m_bIsNormalDead) {
		GetWorld()->GetTimerManager().SetTimer(ResurrectionHandle, this, &ABaseZombie::ResurrectionTimerElapsed, 10.0f, false);		// 10초 후 다시 일어나기 애니메이션 재생 시작 (기존에는 30초)

		GetMesh()->SetCollisionProfileName("NoCollision");		
		GetMesh()->SetGenerateOverlapEvents(false);
	}
	else if (m_bIsCuttingDead) {
		GetWorld()->GetTimerManager().SetTimer(ResurrectionHandle, this, &ABaseZombie::ResurrectionTimerElapsed, 10.0f, false);		// 10초 후 다시 일어나기 애니메이션 재생 시작 (기존에는 60초)

		GetMesh()->SetCollisionProfileName("NoCollision");		
		GetMesh()->SetGenerateOverlapEvents(false);
	}
}

// 다시 일어나기 애니메이션 재생
void ABaseZombie::ResurrectionTimerElapsed()
{
	//UE_LOG(LogTemp, Warning, TEXT("ResurrectionTimerElapsed 실행됨"));

	// 혹시 남아 있는 타이머가 있으면 제거
	GetWorld()->GetTimerManager().ClearTimer(ResurrectionHandle);

	m_bIsStanding = true;	
	auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	if (nullptr != CharacterAnimInstance) {
		CharacterAnimInstance->SetIsStanding(m_bIsStanding);	// 되살아나는 - 일어나는 애니메이션 재생 (블루프린트가 해당 플래그 값을 통해 자동 전환해줌)
	}

	// 다시 기존 SkeletalMesh 보이도록 설정
	GetMesh()->SetVisibility(true);

	GetMesh()->GlobalAnimRateScale = 1.0f;	// 정지 시켜놨던 애니메이션 다시 재생 시작

	StartWatiingTimer();

	// 부활 이펙트 생성
	UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Script/Niagara.NiagaraSystem'/Game/Basic_VFX/Niagara/NS_Basic_4.NS_Basic_4'"));
	if (NiagaraSystem)
	{
		if (m_bIsCuttingDead == false && m_bIsNormalDead == true) {	// 이펙트 중복 생성 방지
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, "Found Resurrect FX!");
			UE_LOG(LogTemp, Log, TEXT("Found Normal Dead Resurrect FX!"));

			ResurrectFX = GetWorld()->SpawnActor<AResurrectNiagaEffect>(AResurrectNiagaEffect::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);

			ResurrectFX->ResurrectFXSystem = NiagaraSystem;

			ResurrectFX->spawn_flag = true;
		}
	}
	else {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, "Couldn't Find Resurrect FX!");
		UE_LOG(LogTemp, Error, TEXT("Couldn't Find Resurrect FX!"));
	}

	m_bIsCuttingDead = false;
	m_bIsNormalDead = false;
}

// 되살아나기 애니메이션 워이팅 타이머
void ABaseZombie::StartWatiingTimer()
{
	// 기존 타이머가 남아 있을 경우 초기화
	GetWorld()->GetTimerManager().ClearTimer(WattingHandle);

	GetWorld()->GetTimerManager().SetTimer(WattingHandle, this, &ABaseZombie::WaittingTimerElapsed, 5.f, false);	// 5초(다시 일어나기 애니메이션 재생 시간) 이후 완전히 다시 살아남 => * 좀비 부활 관련 초기화 작업은 서버에서 부활 동기화 통신 받으면 따로 진행 
}

// 일어나는 애니메이션 waiting 
void ABaseZombie::WaittingTimerElapsed()
{
	//UE_LOG(LogTemp, Warning, TEXT("WaittingTimerElapsed 실행됨"));

	// 혹시 남아 있는 타이머가 있으면 제거
	GetWorld()->GetTimerManager().ClearTimer(ResurrectionHandle);

	// 애니메이션 관련 플래그 값들 모두 다시 초기화 ===> 일단 다시 부활하면 기본 서있는 자세에서 시작하도록
	m_bIsAttacking = false;
	m_bBeAttacked = false;
	m_bIsShouting = false;
	
	m_bIsNormalDead = false;
	//m_bIsCuttingOverlapOn = false;
	m_bIsCuttingDead = false;
	m_bIsStanding = false;

	//auto CharacterAnimInstance = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
	//if (nullptr != CharacterAnimInstance) {
	//	CharacterAnimInstance->SetIsNormalDead(m_bIsNormalDead);
	//	CharacterAnimInstance->SetIsCuttingDead(m_bIsCuttingDead);
	//	CharacterAnimInstance->SetIsStanding(m_bIsStanding);
	//} // 이거 그냥 CachedAnimInstance 쓰면 됐는데;;

	// 부활 직후 다른 애니메이션 재생 방지
	CachedAnimInstance->SetIsNormalDead(m_bIsNormalDead);
	CachedAnimInstance->SetIsCuttingDead(m_bIsCuttingDead);
	CachedAnimInstance->SetIsStanding(m_bIsStanding);
	CachedAnimInstance->SetCurrentPawnSpeed(0);	// 애니메이션은 idle로 전환
	CachedAnimInstance->SetIsStandingStill(false);	// 숨고르기 애니메이션도 초기화
}

void ABaseZombie::Resurrect()
{
	// 다시 충돌 설정 ON
	GetCapsuleComponent()->SetCollisionProfileName("ZombieCol");
	GetMesh()->SetCollisionProfileName("Zombie");
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCharacterMovement()->MaxWalkSpeed = GetSpeed();
	SetHP(GetStartHP());
	SetDie(false);

	doAction_takeDamage_onTick = false;
	doAction_setIsNormalDead_onTick = false;	
	doAction_setIsCuttingDead_onTick = false;
	doAction_addImpulseOnProcMesh_onTick = false;

	m_bIsShouting = false;	// 샤우팅 좀비 소리치는 중인지
	m_bIsShouted = false;	// 샤우팅 좀비 소리쳤는지

	IsGrowlSoundPlaying = false;	// 부활한 직후에 바로 눈 앞에 플레이어가 있으면 곧바로 소리치도록 
	GetWorld()->GetTimerManager().ClearTimer(GrowlSoundTimerHandle);	// 타이머도 초기화시켜주고

	procMesh_AddImpulse_1 = false;
	procMesh_AddImpulse_2 = false;

	UE_LOG(LogTemp, Warning, TEXT("[Resurrect] Resurrect Zombie ID: %d"), ZombieId);
}

void ABaseZombie::PlayGrowlSound()
{
	auto* world = GetWorld();
	if (IsValid(world) && GrowlSound && !IsGrowlSoundPlaying)
	{
		if (GetZombieName() == TEXT("ShoutingZombie") && m_bIsShouting == true) { // 샤우팅 좀비의 경우, 샤우팅이랑 소리 안 겹치게
			UE_LOG(LogTemp, Log, TEXT("[PlaySoundLog] Playing GrowlSound is skipped for Shouting! - ZombieID: %d"), ZombieId);

			IsGrowlSoundPlaying = true;	// 게임 서버는 이렇게 돌아가니 맞춰준거임
			
			GetWorld()->GetTimerManager().SetTimer(GrowlSoundTimerHandle, [this]()	// 그래서 20초 딜레이 타이머도 켜 놓고
				{
					IsGrowlSoundPlaying = false;
				}, 20.0f, false);

			return;
		}

		UE_LOG(LogTemp, Log, TEXT("[PlaySoundLog] Playing GrowlSound at Location! - ZombieID: %d"), ZombieId);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[PlaySoundLog] Playing GrowlSound at Location! - ZombieID: %d"), ZombieId));

		if(GetZombieName() == TEXT("ShoutingZombie"))
			UGameplayStatics::PlaySoundAtLocation(world, GrowlSound.Get(), GetActorLocation(), GetActorRotation(), 1.6f);
		else
			UGameplayStatics::PlaySoundAtLocation(world, GrowlSound.Get(), GetActorLocation(), GetActorRotation(), 0.8f);

		IsGrowlSoundPlaying = true;

		// 일정 시간이 지나면 다시 재생 가능하도록 설정 (20초 후) => 연속 재생 방지
		GetWorld()->GetTimerManager().SetTimer(GrowlSoundTimerHandle, [this]()
			{
				IsGrowlSoundPlaying = false;
			}, 20.0f, false);
	}

	else if (IsGrowlSoundPlaying) {
		UE_LOG(LogTemp, Log, TEXT("[PlaySoundLog] GrowlSound is on cooldown! - ZombieID: %d"), ZombieId);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[PlaySoundLog] GrowlSound is on cooldown! - ZombieID: %d"), ZombieId));
	}
}

void ABaseZombie::PlayDeathSound()
{
	auto* world = GetWorld();
	if (IsValid(world) && DeathSound)
	{
		UE_LOG(LogTemp, Log, TEXT("[PlaySoundLog] Playing DeathSound at Location! - ZombieID: %d"), ZombieId);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[PlaySoundLog] Playing DeathSound at Location! - ZombieID: %d"), ZombieId));

		UGameplayStatics::PlaySoundAtLocation(world, DeathSound.Get(), GetActorLocation(), GetActorRotation(), 1.0f);
	}
}

void ABaseZombie::PlayScaredSound()
{
	auto* world = GetWorld();
	if (IsValid(world) && ScaredSound)
	{
		UE_LOG(LogTemp, Log, TEXT("[PlaySoundLog] Playing ScaredSound at Location! - ZombieID: %d"), ZombieId);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("[PlaySoundLog] Playing ScaredSound at Location! - ZombieID: %d"), ZombieId));

		UGameplayStatics::PlaySoundAtLocation(world, ScaredSound.Get(), GetActorLocation(), GetActorRotation(), 1.0f);
	}
}

void ABaseZombie::IgnorePlayer()
{
	// 도망가기때 플레이어 잠시 무시하기 (1초) -> 고개 돌리는 시간 기다려주기
	IgnorePlayerForFewSecond = true;

	// 혹시 모르니 이전 타이머 초기화 시켜주고
	GetWorld()->GetTimerManager().ClearTimer(IgnorePlayerTimerHandle);

	// 1초 뒤에는 다시 플레이어를 포착하도록
	GetWorld()->GetTimerManager().SetTimer(IgnorePlayerTimerHandle, [this]()
		{
			IgnorePlayerForFewSecond = false;
		}, 1.0f, false);
}

void ABaseZombie::Runaway()
{
	HealingFX = GetWorld()->SpawnActor<AHealingNiagaEffect>(AHealingNiagaEffect::StaticClass(), this->GetActorLocation(), FRotator::ZeroRotator);
	HealingFX->OwnerChar = this;
	HealingFX->materialType = 2;
	if (GetZombieName() == "NormalZombie") {
		HealingFX->spawn_offset.Z = 23.f;
	}
	else if (GetZombieName() == "RunningZombie") {
		HealingFX->spawn_offset.Z = 24.f;
	}
	else if (GetZombieName() == "ShoutingZombie") {
		HealingFX->spawn_offset.Z = 27.f;
	}
	HealingFX->spawn_flag = true;
}