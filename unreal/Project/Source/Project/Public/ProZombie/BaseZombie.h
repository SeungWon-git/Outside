// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "tuple"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProceduralMeshComponent.h"
#include "ProZombie/ZombieAnimInstance.h"
#include "ProGamemode/ProGameInstance.h"
#include "ProCharacter/BaseCharacter.h"
#include "ProItem/NormalWeaponActor.h"

// 구조체
#include "ProData/ZBoneStruct.h"
#include "ProData/SkeletalMeshBondData.h"
#include "ProData/ZombieMeshData.h"

#include "BaseZombie.generated.h"





DECLARE_MULTICAST_DELEGATE(FAttackEndDelegate);
DECLARE_MULTICAST_DELEGATE(FShoutingEndDelegate);

class ABloodNiagaEffect;
class AShoutingNiagaEffect;
class AResurrectNiagaEffect;
class AHealingNiagaEffect;

// Zombie 클래스의 부모 클래스
UCLASS()
class PROJECT_API ABaseZombie : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseZombie();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	enum TARGET {
		NULL_TARGET = 1,
		PLAYER,
		SHOUTING,
		FOOTSOUND,
		INVESTIGATED,
		PATROL,
		HORDESOUND,
		RUNAWAY,
		WAIT,
		LOOKINGAROUND,

		BLACKBOARDCLEARED = 69
	};


	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	//void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);


	void SetZombieName(FName zombiename) { m_sZombieName = zombiename; };
	FName GetZombieName() { return m_sZombieName; }

	float GetHP() { return m_fHP; }
	void SetHP(float hp) { m_fHP = hp; }

	float GetStartHP() { return m_fStartHP; }
	void SetStartHP(float hp) { m_fStartHP = hp; }

	float GetSTR() { return m_fSTR; }
	void SetSTR(float str) { m_fSTR = str; }

	float GetSpeed() { return m_fSpeed; }
	void SetSpeed(float speed) { m_fSpeed = speed; }	
	
	float GetTurningSpeed() { return m_fTurningSpeedDegree; }		// 90.f
	void SetTurningSpeed(float degree) { m_fTurningSpeedDegree = degree; }
		
	bool IsDie() { return m_bDie; }
	void SetDie(bool die) { m_bDie = die; }

	bool IsFind() { return m_bFind; }
	void SetFind(bool find) { m_bFind = find; }


	bool IsSpecialAbility() { return m_bSpecialAbility; }
	void SetSpecialAbility(bool specialability) { m_bSpecialAbility = specialability; }
		 
	bool IsShouted() { return m_bIsShouted; }
	void SetShouted(bool shouted) { m_bIsShouted = shouted; }


	void Attack(uint32 PlayerId);

	UFUNCTION()
	void AttackMontageEnded(UAnimMontage* Montage, bool interrup);

	UPROPERTY(EditAnywhere)
	bool m_bIsAttacking = false;

	FAttackEndDelegate m_DAttackEnd;

	void AttackCheck();


	void Shouting();

	UFUNCTION()
	void ShoutingMontageEnded(UAnimMontage* Montage, bool interrup);

	void BeAttacked();

	UFUNCTION()
	void BeAttackedMontageEnded(UAnimMontage* Montage, bool interrup);

	UFUNCTION()
	void OnZombieHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	UPROPERTY(EditAnywhere)
	bool m_bBeAttacked = false;

	void SetNormalDeadWithAnim();
	void SetCuttingDeadWithAnim();


	void SetZombieId(uint32 NewPlayerId);
	uint32 GetZombieId() const;
	void UpdateZombieData(FVector Location);


	// 소리치는 중인지
	UPROPERTY(EditAnywhere)
	bool m_bIsShouting = false;
	// 소리쳤는지
	UPROPERTY(EditAnywhere)
	bool m_bIsShouted = false; 

	UPROPERTY(EditAnywhere)
	bool m_bIsNormalDead = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsCuttingOverlapOn = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsCuttingDead = false;


	UPROPERTY(EditAnywhere)
	bool procMesh_AddImpulse_1 = false;

	UPROPERTY(EditAnywhere)
	bool procMesh_AddImpulse_2 = false;


	FShoutingEndDelegate m_DShoutingEnd;


	UPROPERTY(EditAnywhere)
	FName m_sZombieName;

	// TimerHandle 지정된 몇초 뒤에 부활하기 위해 사용하는 핸들러 (애니메이션 재생)
	FTimerHandle ResurrectionHandle;

	// TimerHandle 지정된 몇초 뒤에 부활하기 위해 사용하는 핸들러 (애니메이션 재생 후 딜레이 기다리고 관련 플래그 값들 초기화)
	FTimerHandle WattingHandle;

	void StartResurrectionTimer();

	void ResurrectionTimerElapsed();

	void StartWatiingTimer();

	void WaittingTimerElapsed();

	void Resurrect();

	UPROPERTY(EditAnywhere)
	bool m_bIsStanding = false;

	UFUNCTION(BlueprintCallable)
	void CutZombie(FVector planeposition, FVector planenormal, bool do_self); 

	void SliceProceduralmeshTest(FVector planeposition, FVector planenormal);


	float CalculateEuclideanDistance(const FVector& Point1, const FVector& Point2);
	float CalculateAverageDistance(const TArray<FVector>& Vertices);
	float CalculateDynamicEps(const TArray<FVector>& Vertices, int K);
	//void DBSCANWithAverageDistance(const TArray<FVector>& Vertices, int MinPts, TArray<int>& Labels, TMap<int, TArray<FVector>>& ClusteredVertices);
	void DBSCANWithAverageDistance(const TArray<FVector>& Vertices, int MinPts, TArray<int>& Labels, TMap<int, TArray<TPair<int, FVector>>>& ClusteredVertices);
	//void DBSCANWithAverageDistance(const TArray<FVector>& Vertices, int MinPts, TArray<int>& Labels, TMap<int, TArray<FVector>>& ClusteredVertices, TMap<int, TPair<int32, int32>>& ClusterIndexRanges);
	void RefineClusterUsingGraph(const TArray<TPair<int, FVector>>& ClusterPoints, TArray<TArray<TPair<int, FVector>>>& SeparatedClusters);
	void KMeansSplitCluster(TArray<TPair<int, FVector>>& ClusterPoints, TArray<TArray<TPair<int, FVector>>>& SeparatedClusters);

	//void GetVerticesByCluster(const TArray<FVector>& Vertices, const TArray<int>& Labels, TMap<int, FVector>& ClusterCenters);
	void GetVerticesByCluster(const TMap<int, TArray<TPair<int, FVector>>>& ClusteredVerticesMap, TMap<int, FVector>& ClusterCenters);
	void MergeClustersBasedOnBoneName(TMap<int, TArray<TPair<int, FVector>>>& ClusteredVertices, TMap<int, FVector>& ClusterCenters, TMap<FName, FVector>& BoneAveragePos);

	void InitVertexBoneMap(TMap<FVector, FVertexBoneData>& VertexBoneMap, TMap<FName, FVector>& BoneAveragePos);
	void InitVertexIndexPosMapForMerge(TMap<FZombieMeshData, FVector>& VertexIndexPosMap);

	void FillSectionVertexBoneMap(const TMap<FVector, FVertexBoneData>& VertexBoneMap, TMap<int32, TMap<int32, FName>>& SectionVertexBoneMap, TMap<FName, FVector>& BoneAveragePos);
	FName FindNearstBoneName(TMap<FName, FVector>& BoneAveragePos, const FVector& TargetPosition);
	void CreateAndApplyBoundingBox(UProceduralMeshComponent* ProceduralMesh);
	void CreateAndApplyBoundingBoxByNewProcMesh(UProceduralMeshComponent* ProceduralMesh);

	void ReportProVertexMap(TMap<FVector, FVertexBoneData>& VertexBoneMap);


	void RotateFromCutProc1MeshToSkel();
	void RotateFromCutProc2MeshToSkel();
	void RotateFromProcMeshToSkel();

	void RotateFromCutProc1MeshToSkelBone();
	void RotateFromCutProc2MeshToSkelBone();
	void RotateFromProcMeshToSkelBone();

	void SetCutProc1MeshPivotToCenter();
	void SetCutProc2MeshPivotToCenter();
	void SetProceduralMeshPivotToCenter();


	FTimerHandle ZombieMergeWattingHandle;

	void StartMergiingTimer();
	void StartMergiingTimerNew();

	FTimerHandle ZombieMergingHandle;

	void MergingTimerElapsed();


	// ZombieMeshData : [스켈레탈 섹션인덱스, 버텍스 인덱스]  , FZombieMeshData : 몇번 프로시저럴메쉬인지 , 버텍스인덱스 -> 섹션인덱스는 어차피 같을것이니까  
	TArray<TMap<FZombieMeshData, FZombieMeshData>> ProMeshIndexArray; // 병합하기 전 프로시저럴 메쉬들이 기존 스켈레탈 메쉬의 어떤 섹션인덱스와 버텍스 인덱스를 가지고 있는지 저장하는 변수

	FVector CutPro_1Distance;
	FVector CutPro_2Distance;
	TArray<FVector> resultDistanceAvg;  // 각 섹션에 대해 계산된 평균값을 저장하는 TArray
	//TArray<TMap<FZombieMeshData, FVector>> DistanceResultArray;
	TArray<FName> ProcMeshBone;

	// Procedural mesh component for the cut part
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProcMesh")
	UProceduralMeshComponent* CutProceduralMesh_1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProcMesh") 
	UProceduralMeshComponent* CutProceduralMesh_2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProcMesh")
	TMap<UProceduralMeshComponent*, bool> ProcMeshImpulseStates;

	void CreativeProceduralMesh(FVector planeposition, FVector planenormal);

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Material_Blood;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Material_ProcMeshBlood;

	UPROPERTY(EditAnywhere)
	TArray<ABloodNiagaEffect*> BloodFX;

	UPROPERTY(EditAnywhere)
	AShoutingNiagaEffect* ShoutingFX;

	UPROPERTY(EditAnywhere)
	AResurrectNiagaEffect* ResurrectFX;

	UPROPERTY(EditAnywhere)
	AHealingNiagaEffect* HealingFX;

	float SetImpulseByWeight(float targetWeight, float baseImpulse);

	UPROPERTY(EditAnywhere)
	ANormalWeaponActor* PlayerWeapon;

	UPROPERTY(EditAnywhere)	// 맞은 위치 기억 => 좀비 해당 방향으로 쳐다보게 하려고
	FVector HitLocation;

	UPROPERTY(EditAnywhere)	// 맞은 시간 기억 (겜시간'GetWorld()->GetTimeSeconds()'->일시정지하면 같이 멈춤)
	double HitTime;	



	TSharedPtr<FZBoneStructure> RootBone;  // **트리 루트 (최상위 본)**
	TSharedPtr<FZBoneStructure> SpecialRootBone;  // **트리 루트 (최상위 본)**

	FVector CutPro_1ClusterCenter;

	FVector CutPro_1StartLocation;
	FVector CutPro_1TargetLocation;

	FRotator CutPro_1StartRotation;
	FRotator CutPro_1TargetRotation;


	FVector CutPro_2StartLocation;
	FVector CutPro_2TargetLocation;
	
	FRotator CutPro_2StartRotation;
	FRotator CutPro_2TargetRotation;



	FName CutPro_1PlaneBoneName;
	FName CutPro_2PlaneBoneName;
	bool m_bIsCutProceduralMesh_2Visibility = true;


	UPROPERTY(EditAnywhere)
	TArray<UProceduralMeshComponent*> ProceduralMeshes;

	TArray<FVector> ProcMeshMergeTargetLocation;
	TArray<FVector> ProcMeshMergeStartLocation;

	TArray<FRotator> ProcMeshMergeStartRotation;
	TArray<FRotator> ProcMeshMergeTargetRotation;

	TArray<FVector> ProcMeshDistance;
	
	UPROPERTY(EditAnywhere)
	double TotalTimeToMove = 5.0; // 5초 동안 이동

	UPROPERTY(EditAnywhere)
	double ElapsedTime = 0.0; // 경과 시간

	UPROPERTY(EditAnywhere)
	bool bIsMergingInProgress = false; // 머지 중 상태

	UPROPERTY(EditAnywhere)
	int m_iMergeFlag = 0; // 머지 중 상태

	UPROPERTY(EditAnywhere)
	TArray<FVector> MergeStartLocation; // 머지 중 상태


	void InitializeBoneHierarchy();  // 본 트리 초기화
	void InitializeSpecialBoneHierarchy();  // 몸통부분 절단 시 새로운 트리 구조
	void PrintBoneHierarchy(TSharedPtr<FZBoneStructure> Bone, int Depth = 0);

	bool InBone(FName BoneAName, FName BoneBName, TSharedPtr<FZBoneStructure> StartBone);
	TSharedPtr<FZBoneStructure> FindBoneByName(TSharedPtr<FZBoneStructure> StartBone, FName BoneName);

	float m_fHP_Prev = 0.f;

	bool doAction_takeDamage_onTick = false;		// 좀비 피격시 클라 동기화용 (다른 클라가 좀비를 때렸을때)
	bool doAction_setIsNormalDead_onTick = false;	// 좀비 사망시 클라 동기화용 (다른 클라가 좀비를 죽였을때)
	bool doAction_setIsCuttingDead_onTick = false;	// 좀비 절단 사망시 클라 동기화용 (다른 클라가 좀비를 절단하여 죽였을때)

	FVector sync_cutPlane;
	FVector sync_cutNormal;
	FVector sync_cutImpulse;

	const float MaxSightRange = 2500.f;	 // 최대 시야 범위

private:

	UPROPERTY(EditAnywhere)
	float m_fHP = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fStartHP = 0.f;
	
	UPROPERTY(EditAnywhere)
	float m_fSTR = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fSpeed = 0.f;

	// 근데 m_bDie 변수 사용 안하고 있긴 함
	UPROPERTY(EditAnywhere)
	bool m_bDie = false;	

	UPROPERTY(EditAnywhere)
	bool m_bFind = false;

	UPROPERTY(EditAnywhere)
	bool m_bSpecialAbility = false;

	UPROPERTY(EditAnywhere)
	float m_fAttackRadius = 50.f;

	UPROPERTY(EditAnywhere)
	float m_fAttackRange = 100.f;

	// 1초당 고개 돌리는 각도
	UPROPERTY(EditAnywhere)
	float m_fTurningSpeedDegree = 90.f;		// 90.f

public:
	UPROPERTY(EditAnywhere)
	UZombieAnimInstance* CachedAnimInstance;

	UProGameInstance* GameInstance;

	UPROPERTY(EditAnywhere)
	uint32 ZombieId = 99;

	FVector NewLocation;

	FVector OldLocation = FVector(0.0f, 0.0f, 0.0f);

	//UPROPERTY(VisibleAnywhere)
	std::tuple<float, float, float> NextPath[2];

	UPROPERTY(EditAnywhere)
	FVector TargetLocation;

	enum FLOOR {
		Zero, B2, B1, F1, F2
	};

	UPROPERTY(EditAnywhere)
	int floor = FLOOR::Zero;

	UPROPERTY(EditAnywhere)
	ABaseCharacter* MyChar;

	bool print_Velocity_1 = true;
	bool print_Velocity_2 = true;

	const float afterAnim_idleInterpol = 0.25f;	// -> 0.25초 idle: 공격/피격/샤우팅 애니메이션 재생후 바로 ZombieMoveTo 불려서 움직이게 되는 거 방지 (보간작업)
	UPROPERTY(EditAnywhere)
	float afterAnim_idleDuration = 0.f;

	// 좀비 설정값들
	const float NormalZombieStartHP = 20.0f;        // 20.0f
	const float NormalZombieSpeed = 200.0f;         // 200.0f (뛰기 스피드)
	const float NormalZombieWalkSpeed = 100.0f;     // 100.0f (걷기 스피드)

	const float RunningZombieStartHP = 20.0f;        // 20.0f
	const float RunningZombieSpeed = 300.0f;         // 300.0f (뛰기 스피드)
	const float RunningZombieWalkSpeed = 200.0f;     // 200.0f (걷기 스피드)

	const float ShoutingZombieStartHP = 30.0f;        // 30.0f
	const float ShoutingZombieSpeed = 230.0f;         // 230.0f (뛰기 스피드)
	const float ShoutingZombieWalkSpeed = 150.0f;     // 150.0f (걷기 스피드)

	const float ZombieInvestigatedSpeed_Offset = 40.f;     // 플레이어 마지막 발견 위치로 움직일 때는 걷기 스피드에서 +40.f 스피드
	const float ZombieHeardFootSoundSpeed_Offset = -20.f;   // 발소리를 들었을 때는 뛰기 스피드에서 -20.f 스피드

	float RelativeHeight_ZOffset = 0; // 좀비마다 키 변경에 따른 공중 떠있기 방지하기 위해 사용

	TARGET	targetType = NULL_TARGET;		// 현재 쫓아가고 있는 타겟의 타입 (1-NULL_TARGET, 2-PLAYER, 3-SHOUTING, 4-FOOTSOUND, 5-INVESTIGATED, 6-PATROL, 7-HORDESOUND, 8-RUNAWAY, 9-WAIT, 10-LOOKINGAROUND, 69-BLACKBOARDCLEARED)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrowlSound", Transient)
	TObjectPtr<USoundBase> GrowlSound;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BeAttackedSound", Transient)
	//TObjectPtr<USoundBase> BeAttackedSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathSound", Transient)
	TObjectPtr<USoundBase> DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScaredSound", Transient)
	TObjectPtr<USoundBase> ScaredSound;

	void PlayGrowlSound();

	void PlayDeathSound();

	void PlayScaredSound();

	void Runaway();

	bool IsGrowlSoundPlaying = false;   // 사운드 중복 재생 방지용 플래그
	FTimerHandle GrowlSoundTimerHandle;     

	UPROPERTY(EditAnywhere)
	bool m_bPlayerInSight;

	FTimerHandle PlayerInSightResetTimerHandle;

	bool bCanDetectPlayer = false;  // 초기에는 탐지 불가능
	FTimerHandle DetectionTimerHandle;

	bool IsRunaway = false;

	bool IgnorePlayerForFewSecond = false;	// (1초) => 좀비가 도망가기 상태일때 플레이어가 정면에 있으면 고개를 돌리기 전에 계속 시야에 들어 왔다 판단해서, 플레이어를 마주보고 이상하게 움직임 치는 거 방지하려고 
	FTimerHandle IgnorePlayerTimerHandle;

	void IgnorePlayer();	// 도망가기때 플레이어 포착 검사 잠시 무시하기 -> 고개 돌리는 시간 기다려주기

};
