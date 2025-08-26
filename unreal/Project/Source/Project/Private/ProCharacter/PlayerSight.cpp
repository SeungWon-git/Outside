// Fill out your copyright notice in the Description page of Project Settings.


#include "ProCharacter/PlayerSight.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UPlayerSight::UPlayerSight()
{

	PrimaryComponentTick.bCanEverTick = true;

}


void UPlayerSight::BeginPlay()
{
	Super::BeginPlay();

	
}


void UPlayerSight::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * m_fMaxSightDistance;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(m_fRadius);
	FHitResult HitResult;
	m_bHasHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		Sphere
	);

	HitActor = HitResult.GetActor();

	if (PreviousHighlightedActor != nullptr) {
		if (PreviousHighlightedActor != HitActor) {	// 이전 하이라이트된 아이템이 시선밖에 나가면 다시 원상복구시키기
			UMeshComponent* PrevMeshComp = PreviousHighlightedActor->FindComponentByClass<UMeshComponent>();
			if (PrevMeshComp) {
				PrevMeshComp->SetVectorParameterValueOnMaterials("SunColor", FVector(1.0f, 1.0f, 1.f));	// 흰색
			}
		}
	}

	if (m_bHasHit == true) { // 충돌 시
		if (HitActor != nullptr) {
			UMeshComponent* MeshComp = HitActor->FindComponentByClass<UMeshComponent>();	// 아이템 주울 수 있는 범위에 들어 왔을 시에 하이라이트
			if (MeshComp) {
				MeshComp->SetVectorParameterValueOnMaterials("SunColor", FVector(1.0f, 1.0f, 0.f));	// 노랑색

				PreviousHighlightedActor = HitActor;
			}
		}
	}

}

