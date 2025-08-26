// Fill out your copyright notice in the Description page of Project Settings.


#include "ProZombie/ShoutingZombieAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GStruct.pb.h"
#include "ProZombie/ShoutingZombie.h"
//#include "ProCharacter/BaseCharacter.h"
#include "ProGamemode/OneGameModeBase.h"
#include "ProGamemode/ProGameInstance.h"

// AI 추가되면 수정될 것 같은 것!!

const FName AShoutingZombieAIController::TargetKey(TEXT("Target"));
const FName AShoutingZombieAIController::StartLocationKey(TEXT("StartLocation"));
const FName AShoutingZombieAIController::PatrolLocationKey(TEXT("PatrolLocation"));

AShoutingZombieAIController::AShoutingZombieAIController()
{
	OwnerZombie = nullptr;
}

void AShoutingZombieAIController::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UProGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}


void AShoutingZombieAIController::ZombieMoveTo(float deltasecond, int& indx)
{
	FVector zomlocation;

	if (OwnerZombie) {
		zomlocation = OwnerZombie->GetActorLocation();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Zombie #%d's OwnerZombie is nullptr!"), Zombie_Id);	// 이미 앞에서 검사해서 의미 없긴하지만
		return;
	}

	// 좀비 공격 중일 때는 MoveTo 정지 
	if (OwnerZombie->CachedAnimInstance->Montage_IsPlaying(OwnerZombie->CachedAnimInstance->AttackMontage) == true) {
		return;
	}

	// 공격/피격/샤우팅 애니메이션이 끝나면 제자리에 잠시 idle 상태로 있게하는 일종의 보간작업
	if (OwnerZombie->afterAnim_idleDuration > 0.f) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("Zombie #%d afterAnim_idleDuration left: %f s"), OwnerZombie->ZombieId, OwnerZombie->afterAnim_idleDuration));

		OwnerZombie->afterAnim_idleDuration -= deltasecond;
		if (OwnerZombie->afterAnim_idleDuration <= 0) {
			OwnerZombie->afterAnim_idleDuration = 0;
		}

		return;
	}

	if (OwnerZombie->targetType == OwnerZombie->LOOKINGAROUND) {	// 좀비가 현재 주위 둘러보기 상태라면 MoveTo 정지!
		OwnerZombie->CachedAnimInstance->SetCurrentPawnSpeed(0);	// 애니메이션은 idle로 전환


		lookingaround_idleDuration += deltasecond;

		if (lookingaround_idleDuration >= 8.f) {	// 만약 좀비가 제자리에서 8초 이상 있을 시에 => 숨고르기 (주위둘러보기 이후 바로 숨고르기에 들어 갈 경우 서버에서 통신을 안보내기 때문에 따로 처리해줘야함)
			OwnerZombie->CachedAnimInstance->SetIsStandingStill(true);	// 애니메이션 standing still로 전환
		}
		else {		// 도망치기->휴식(숨고르기)->주위 둘러보기 에서 standing still 애니메이션으로 주위 둘러보기 하는 문제 때문에
			OwnerZombie->CachedAnimInstance->SetIsStandingStill(false);		// 다시 초기화
		}

		return;
	}
	else {
		lookingaround_idleDuration = 0;	// 주위 둘러보기 끝나면 다시 초기화
	}


	std::tuple<float, float, float> target = OwnerZombie->NextPath[indx];

	// 현재 목표 노드
	float PathX = get<0>(target);
	float PathY = get<1>(target);

	if (PathX == -100000.f && PathY == -100000.f) {	// 초기 더미값
		return;
	}

	// 좀비 이동경로 확인용 debugline
	FVector Pos;
	Pos.X = get<0>(target);
	Pos.Y = get<1>(target);
	Pos.Z = get<2>(target);
	FVector Start = Pos + FVector(0, 0, 100);
	FVector End = Pos - FVector(0, 0, 100);
	FCollisionQueryParams Params;

	//DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 3.f);

	//이미 도착지점에 도착했을때
	if (zomlocation.X == PathX && zomlocation.Y == PathY) {
		idleDuration += deltasecond;

		if (idleDuration >= 0.3f) {	// 만약 좀비가 제자리에 0.3초 이상 있을 시에
			OwnerZombie->CachedAnimInstance->SetCurrentPawnSpeed(0);	// 애니메이션 idle로 전환

			if (idleDuration >= 1.5f) {	// 만약 좀비가 제자리에 1.5초 이상 있을 시에 => 숨고르기
				OwnerZombie->CachedAnimInstance->SetIsStandingStill(true);	// 애니메이션 standing still로 전환
			}
		}
	
		return;
	}
	else {
		idleDuration = 0;	// 다시 초기화
		OwnerZombie->CachedAnimInstance->SetIsStandingStill(false);		// 다시 초기화
	}

	if (OwnerZombie->targetType == OwnerZombie->TARGET::WAIT) {	// 서버 좀비 HaveToWait 상태일때 (애니메이션 재생 중일 떄)
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Zombie #%d HaveToWait!"), OwnerZombie->ZombieId));
		//UE_LOG(LogTemp, Log, TEXT("Zombie #%d HaveToWait!"), OwnerZombie->ZombieId);

		return;	//애니메이션 바꾸는 거 밑에 있어야 제자리 걸음 X
	}


	// 타겟 방향 계산
	float dx = PathX - zomlocation.X;
	float dy = PathY - zomlocation.Y;

	// 거리를 계산
	float distance = sqrt(dx * dx + dy * dy);

	// 이동 방향 벡터를 정규화
	float directionX = dx / distance;
	float directionY = dy / distance;

	// 좀비 속도 지정 (걷기/뛰기)
	float ZombieSpeed = 0.f;
	if (OwnerZombie->targetType == OwnerZombie->TARGET::PATROL) {	// 걷기

		OwnerZombie->SetSpeed(OwnerZombie->ShoutingZombieWalkSpeed);
		ZombieSpeed = OwnerZombie->GetSpeed();

		OwnerZombie->CachedAnimInstance->SetPlayAnimSpeed(0.75f);
	}
	else if (OwnerZombie->targetType == OwnerZombie->TARGET::PLAYER || OwnerZombie->targetType == OwnerZombie->TARGET::SHOUTING 
		|| OwnerZombie->targetType == OwnerZombie->TARGET::FOOTSOUND || OwnerZombie->targetType == OwnerZombie->TARGET::INVESTIGATED
		|| OwnerZombie->targetType == OwnerZombie->TARGET::HORDESOUND || OwnerZombie->targetType == OwnerZombie->TARGET::RUNAWAY) {	// 뛰기

		OwnerZombie->SetSpeed(OwnerZombie->ShoutingZombieSpeed);
		ZombieSpeed = OwnerZombie->GetSpeed();

		OwnerZombie->CachedAnimInstance->SetPlayAnimSpeed(1.075f);
	}
	else if (OwnerZombie->targetType == OwnerZombie->TARGET::BLACKBOARDCLEARED) {
		return;
	}

	if (ZombieSpeed == 0.f) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[ERROR] Zombie #%d's speed is ZERO!!!!!"), OwnerZombie->ZombieId));
		UE_LOG(LogTemp, Error, TEXT("[ERROR] Zombie #%d's speed is ZERO!!!!!"), OwnerZombie->ZombieId);
	}

	// 이동 거리 계산
	float moveDistance = ZombieSpeed * deltasecond;

	// 이동 벡터 계산
	float moveX = directionX * moveDistance;
	float moveY = directionY * moveDistance;

	// 목표에 도착했는지 확인
	float newDistance = sqrt((PathX - zomlocation.X) * (PathX - zomlocation.X) + (PathY - zomlocation.Y) * (PathY - zomlocation.Y));

	if (newDistance < moveDistance) {
		zomlocation.X = PathX;
		zomlocation.Y = PathY;

		// 다음 목표 노드로 이동
		indx++;

		// 경로의 끝에 도착 = 최종 목표지점에 도착
		if (get<0>(OwnerZombie->NextPath[indx]) == -100000.f && get<1>(OwnerZombie->NextPath[indx]) == -100000.f && get<2>(OwnerZombie->NextPath[indx]) == -100000.f) {
			indx--;
		}
		else {	// 꼭지점을 넘어 갈 때
			float mid_deltasecond = newDistance / ZombieSpeed;
			float after_deltasecond = deltasecond - mid_deltasecond;

			ZombieMoveTo(after_deltasecond, indx);
		}
	}
	else {
		// 타겟 방향으로 이동
		zomlocation.X += moveX;
		zomlocation.Y += moveY;
	}


	OwnerZombie->SetActorLocation(zomlocation);
	OwnerZombie->CachedAnimInstance->SetCurrentPawnSpeed(ZombieSpeed);
}

void AShoutingZombieAIController::ZombieTurn(float deltasecond, int& indx)
{
	FVector zombieDest;

	// 좀비가 공격 중일때 => 플레이어 쪽으로 시선 돌리기
	if (OwnerZombie->CachedAnimInstance->Montage_IsPlaying(OwnerZombie->CachedAnimInstance->AttackMontage) == true) {
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);
		ABaseCharacter* Char = nullptr;

		// 플레이어 찾기
		bool result = false;
		for (AActor* Player : Players)
		{
			Char = Cast<ABaseCharacter>(Player);

			// 플레이어 자기 자신일 때는 Char->GetPlayerId() 에 99가 담기므로
			if (Char->GetPlayerId() == 99) {
				if (GameInstance->ClientSocketPtr->MyPlayerId == attackPlayerID) {
					result = true;
					break;
				}
			}
			else if (Char->GetPlayerId() == attackPlayerID) {
				result = true;
				break;
			}
		}

		if (result == false) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[ERROR] ZombieTurn(ToPlayer) - Couldn't find Player ID #%d"), attackPlayerID));
			UE_LOG(LogTemp, Error, TEXT("[ERROR] ZombieTurn(ToPlayer) #%d - Couldn't find Player ID #%d"), Zombie_Id, attackPlayerID);
			return;
		}
		else {
			// 해당 플레이어 쪽으로 회전시키기
			zombieDest.X = Char->GetActorLocation().X;
			zombieDest.Y = Char->GetActorLocation().Y;
			zombieDest.Z = Char->GetActorLocation().Z;

			OwnerZombie->SetTurningSpeed(90.f);	// 기본값
		}
	}
	// 좀비가 샤우팅 중일때 => 플레이어 쪽으로 시선 돌리기
	else if (OwnerZombie->CachedAnimInstance->Montage_IsPlaying(OwnerZombie->CachedAnimInstance->ShoutingMontage) == true) {
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);
		ABaseCharacter* Char = nullptr;

		// 플레이어 찾기
		bool result = false;
		for (AActor* Player : Players)
		{
			Char = Cast<ABaseCharacter>(Player);

			// 플레이어 자기 자신일 때는 Char->GetPlayerId() 에 99가 담기므로
			if (Char->GetPlayerId() == 99) {
				if (GameInstance->ClientSocketPtr->MyPlayerId == shoutingTo_PlayerId) {
					result = true;
					break;
				}
			}
			else if (Char->GetPlayerId() == shoutingTo_PlayerId) {
				result = true;
				break;
			}
		}

		if (result == false) {
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[ERROR] ZombieTurn(ToPlayer) - Couldn't find Player ID #%d"), attackPlayerID));
			UE_LOG(LogTemp, Error, TEXT("[ERROR] ZombieTurn(Shouting) #%d - Couldn't find Player ID #%d"), Zombie_Id, shoutingTo_PlayerId);
			return;
		}
		else {
			// 해당 플레이어 쪽으로 회전시키기
			zombieDest.X = Char->GetActorLocation().X;
			zombieDest.Y = Char->GetActorLocation().Y;
			zombieDest.Z = Char->GetActorLocation().Z;

			OwnerZombie->SetTurningSpeed(90.f);	// 기본값
		}
	}
	// 좀비가 피격 중일때 => 맞은 방향 쪽으로(무기의 위치) 시선 돌리기
	else if (OwnerZombie->CachedAnimInstance->Montage_IsPlaying(OwnerZombie->CachedAnimInstance->BeAttackedMontage) == true // 피격 애니메이션 재생중이고
		&& OwnerZombie->targetType != OwnerZombie->PLAYER	// targetType이 플레이어가 아닌 경우(피격 당시 바로 앞에 플레이어 있는 경우에는 뒤돌 필요X)
		&& GetWorld()->GetTimeSeconds() - OwnerZombie->HitTime >= 1.0f) {	// 피격 후 1초가 지났을때 (자연스럽게 고개 돌리도록)

		// 맞은/피격 당한 쪽으로 회전시키기
		zombieDest.X = OwnerZombie->HitLocation.X;
		zombieDest.Y = OwnerZombie->HitLocation.Y;
		zombieDest.Z = OwnerZombie->HitLocation.Z;

		OwnerZombie->SetTurningSpeed(150.f);	// 고개 빨리 돌리도록
	}
	// 좀비가 idle 상태일때 => 고개 그대로
	else if (OwnerZombie->CachedAnimInstance->m_fCurrentPawnSpeed == 0 && (OwnerZombie->targetType == OwnerZombie->PATROL || OwnerZombie->targetType == OwnerZombie->RUNAWAY)) {
		return;
	}
	// 좀비가 주위 둘러보기 상태 일때 => 8초 동안 2초 간격으로 3번 고개 돌리기 (처음, 마지막 1초는 가만히)
	else if (OwnerZombie->targetType == OwnerZombie->LOOKINGAROUND) {
		// 해당 방향으로 회전시키기
		zombieDest.X = get<0>(OwnerZombie->NextPath[indx]);
		zombieDest.Y = get<1>(OwnerZombie->NextPath[indx]);
		zombieDest.Z = get<2>(OwnerZombie->NextPath[indx]);

		OwnerZombie->SetTurningSpeed(120.f);	// 고개 조금 빨리 돌리도록
	}
	// 아니면 이동 중이므로
	else {
		// 다음 행선지 쪽으로 회전시키기
		if (indx + 1 < 2) {	// 더 자연스러운 고개 돌림을 위함
			if (false == (get<0>(OwnerZombie->NextPath[indx + 1]) == -100000.f && get<1>(OwnerZombie->NextPath[indx + 1]) == -100000.f && get<2>(OwnerZombie->NextPath[indx + 1]) == -100000.f)) {
				zombieDest.X = get<0>(OwnerZombie->NextPath[indx + 1]);
				zombieDest.Y = get<1>(OwnerZombie->NextPath[indx + 1]);
				zombieDest.Z = get<2>(OwnerZombie->NextPath[indx + 1]);

				if (OwnerZombie->targetType == OwnerZombie->RUNAWAY)
					OwnerZombie->SetTurningSpeed(150.f);	// 고개 빨리 돌리도록
				else
					OwnerZombie->SetTurningSpeed(90.f);	// 기본값
			}
			else {
				return;
			}
		}
		else {
			if (false == (get<0>(OwnerZombie->NextPath[indx]) == -100000.f && get<1>(OwnerZombie->NextPath[indx]) == -100000.f && get<2>(OwnerZombie->NextPath[indx]) == -100000.f)) {
				zombieDest.X = get<0>(OwnerZombie->NextPath[indx]);
				zombieDest.Y = get<1>(OwnerZombie->NextPath[indx]);
				zombieDest.Z = get<2>(OwnerZombie->NextPath[indx]);
			
				if (OwnerZombie->targetType == OwnerZombie->RUNAWAY)
					OwnerZombie->SetTurningSpeed(150.f);	// 고개 빨리 돌리도록
				else
					OwnerZombie->SetTurningSpeed(90.f);	// 기본값
			}
			else {
				return;
			}
		}
	}

	FVector zomTarDir = zombieDest - OwnerZombie->GetActorLocation();

	FVector zomTarNDir = zomTarDir.GetSafeNormal();
	FRotator zomTarRot = zomTarNDir.Rotation();
	FRotator zomCurRot = OwnerZombie->GetActorRotation();

	// 회전 계산
	float RotationSpeed = OwnerZombie->GetTurningSpeed() * deltasecond;

	FRotator NewRotation = FMath::RInterpTo(zomCurRot, zomTarRot, deltasecond, RotationSpeed);


	OwnerZombie->SetActorRotation(NewRotation);
}

void AShoutingZombieAIController::Tick(float DeltaTime)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn == nullptr || OwnerZombie == nullptr || OwnerZombie->MyChar == nullptr) {
		return;
	}

	if (OwnerZombie->MyChar->floor != OwnerZombie->floor) {
		return;
	}


	Super::Tick(DeltaTime);


	// 좀비 사망시 => 고대로오오 어어얼으으음!!!
	if (OwnerZombie->GetHP() <= 0) { return; }

	// 좀비가 샤우팅 중일때 => 잠시 멈춤
	else if (OwnerZombie->m_bIsShouting) { 
		int dummy = 0;
		ZombieTurn(DeltaTime, dummy);
		return; 
	}

	int indx = 0;
	ZombieMoveTo(DeltaTime, indx);
	ZombieTurn(DeltaTime, indx);


	// 초기에는 Detect하지 않도록 => 맵 로딩 전에 벽 뚫고 Detect하는 거 방지
	if (OwnerZombie->bCanDetectPlayer == false) { return; }

	// 피격 애니메이션 중이라면 시야 검사 X => 서버에서 좀비 피격 애니메이션 재생 후 블랙보드 초기화하는 거랑 꼬이면서, 숨고르기 상태에서 피격 당해서 뒤돌아보고 플레이어 무시 하는 버그 생길 수 있음;;
	if (OwnerZombie->CachedAnimInstance->Montage_IsPlaying(OwnerZombie->CachedAnimInstance->BeAttackedMontage) == true) { return; }


	FVector ZombieForward = OwnerZombie->GetActorForwardVector(); // 좀비의 전방 벡터
	FVector ZombieLocation = OwnerZombie->GetActorLocation(); // 좀비의 위치

	// 좀비 시야각 (전방 120도)
	float FieldOfView = FMath::Cos(FMath::DegreesToRadians(120.0f / 2.0f));

	APawn* NearestPawn = nullptr;

	uint32 myPlayerId = GameInstance->ClientSocketPtr->GetMyPlayerId();


	// 좀비들의 시야 검사 "나 자신"에 대해서만 실시==========================
	// (즉, 로컬인 플레이어만 포착! - 원격 플레이어는 무시!)
	PlayerPawn = Cast<APawn>(OwnerZombie->MyChar);

	if (PlayerPawn == nullptr) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("[Error] Zombie #%d's PlayerPawn is NULL!!! (from AI Controller)"), OwnerZombie->ZombieId));
		UE_LOG(LogTemp, Error, TEXT("[Error] Zombie #%d's PlayerPawn is NULL!!! (from AI Controller)"), OwnerZombie->ZombieId);
		return;
	}

	if (OwnerZombie->MyChar->GetHP() > 0) {	// 살아 있을 때만 좀비시야 검사 

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("(OwnerZombie->MyChar->GetHP() > 0) => Detect Check Initiate!!!")));

		FVector PlayerLocation = PlayerPawn->GetActorLocation(); // 플레이어의 위치
		FVector DirectionToPlayer = (PlayerLocation - ZombieLocation).GetSafeNormal(); // 플레이어로 향하는 방향 벡터

		float DotProduct = FVector::DotProduct(ZombieForward, DirectionToPlayer);

		float Distance = FVector::Dist(PlayerLocation, ZombieLocation);	// 거리 검사
		bool InZombieSight = FieldOfView <= DotProduct ? true : false;	// 시야 각 내에 들어와 있는지 검사
		bool LineOfSightTo_Player = LineOfSightTo(PlayerPawn);			// 장애물에는 가려지진 않았는지 검사 (360도 전 방향으로 검사)

		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("Distance: %f"), Distance));
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("LineOfSightTo: %s"), LineOfSightTo_Player ? TEXT("true") : TEXT("false")));
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("InZombieSight: %s"), InZombieSight ? TEXT("true") : TEXT("false")));

		if (PlayerPawn && Distance <= OwnerZombie->MaxSightRange && LineOfSightTo_Player && InZombieSight)
		{
			if (OwnerZombie->IgnorePlayerForFewSecond == false) {	// 도망가기때 고개 돌리는 시간 필요
				NearestPawn = PlayerPawn;

				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("Detected Player!!!")));

				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("Detected Player ID #%d"), Char->GetPlayerId()));
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("My Player ID #%d"), myPlayerId));
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, FString::Printf(TEXT("Detected Zombie ID #%d"), OwnerZombie->GetZombieId()));
				//UE_LOG(LogNet, Display, TEXT("Detected Zombie ID #%d"), OwnerZombie->GetZombieId());
			}
		}

		// NearestPawn에 따라 상태 변경
		if (NearestPawn) {	// NearestPawn 존재 O -> 나를 포착함
			if (OwnerZombie->m_bPlayerInSight == false) {
				OwnerZombie->m_bPlayerInSight = true;

				ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(NearestPawn);
				Send_Detected(); // 플레이어 감지 메시지 전송
				LastSeenPlayer = BaseCharacter;

				UE_LOG(LogNet, Display, TEXT("Zombie #%d Detected Player #%d"), OwnerZombie->GetZombieId(), myPlayerId);

				// 샤우팅 실행
				//if (OwnerZombie->IsShouted() == false) {	// 처음 플레이어를 발견하면 샤우팅! (그 후에는 없음)
				//	OwnerZombie->Shouting();
				//	shoutingTo_PlayerId = myPlayerId;
				//}
				// 시야 포착 확률 넣어서 이제는 이렇게 작동하면 X
			}
		}
		else {	// NearestPawn 존재 X -> 나를 못 봄
			if (OwnerZombie->m_bPlayerInSight == true) {
				OwnerZombie->m_bPlayerInSight = false;

				Send_PlayerLost(); // 플레이어를 놓쳤을 때 메시지 전송

				UE_LOG(LogNet, Display, TEXT("Zombie #%d Lost Player #%d"), OwnerZombie->GetZombieId(), myPlayerId);
			}
		}
	}
	// 좀비가 플레이어를 죽였는데 마지막 동기화 작업에서 m_bPlayerInSight가 true여서, 서버에서 PlayerInsight가 계속 true인 걸 방지하는 코드 (즉, 좀비가 플레이어를 죽이고 그자리에 가만히 서있는 버그 방지)
	else if (OwnerZombie->MyChar->GetHP() <= 0 && OwnerZombie->m_bPlayerInSight == true) {
		OwnerZombie->m_bPlayerInSight = false;
		Send_PlayerLost(); // 플레이어를 놓쳤을 때 메시지 전송
	}
}

void AShoutingZombieAIController::Send_Detected()
{
	auto* ZombiePawn = Cast<AShoutingZombie>(GetPawn());
	if (Zombie_Id == 9999)
		Zombie_Id = ZombiePawn->GetZombieId();
	uint32 PlayerId = GameInstance->ClientSocketPtr->MyPlayerId;

	Protocol::Detected packet;
	packet.set_zombieid(Zombie_Id);
	packet.set_playerid(PlayerId);
	packet.set_player_insight(true);
	packet.set_packet_type(9);

	std::string serializedData;
	packet.SerializeToString(&serializedData);

	bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
}

void AShoutingZombieAIController::Send_PlayerLost()
{
	auto* ZombiePawn = Cast<AShoutingZombie>(GetPawn());
	if (Zombie_Id == 9999)
		Zombie_Id = ZombiePawn->GetZombieId();
	uint32 PlayerId = GameInstance->ClientSocketPtr->MyPlayerId;

	Protocol::Detected packet;
	packet.set_zombieid(Zombie_Id);
	packet.set_playerid(PlayerId);
	packet.set_player_insight(false);
	packet.set_packet_type(9);

	std::string serializedData;
	packet.SerializeToString(&serializedData);

	bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
}

//void AShoutingZombieAIController::CheckAndSendMovement()
//{
//	auto* ZombiePawn = Cast<AShoutingZombie>(GetPawn());
//	FVector CurrentLocation = ZombiePawn->GetActorLocation();
//	FRotator CurrentRotation = ZombiePawn->GetActorRotation();
//	ZombieId = ZombiePawn->GetZombieId();
//
//	// 이전 위치와 현재 위치 비교 (움직임 감지)
//	//if (PreviousLocation != CurrentLocation || PreviousRotation != CurrentRotation){}
//
//	// Protobuf를 사용하여 TestPacket 생성
//	Protocol::Zombie packet;
//	packet.set_zombieid(ZombieId);
//	packet.set_packet_type(2); // 원하는 유형 설정
//	packet.set_x(CurrentLocation.X);
//	packet.set_y(CurrentLocation.Y);
//	packet.set_z(CurrentLocation.Z);
//	packet.set_pitch(CurrentRotation.Pitch);
//	packet.set_yaw(CurrentRotation.Yaw);
//	packet.set_roll(CurrentRotation.Roll);
//
//	// 직렬화
//	std::string serializedData;
//	packet.SerializeToString(&serializedData);
//
//	// 직렬화된 데이터를 서버로 전송
//	bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
//
//	// 현재 위치를 이전 위치로 업데이트
//	PreviousLocation = CurrentLocation;
//	PreviousRotation = CurrentRotation;
//
//}

//void AShoutingZombieAIController::SetStartLocationValue(FVector startlocation)
//{
//	GetBlackboardComponent()->SetValueAsVector(StartLocationKey, startlocation);
//}

//void AShoutingZombieAIController::SetPatrolLocationValue(FVector patrollocation)
//{
//	GetBlackboardComponent()->SetValueAsVector(PatrolLocationKey, patrollocation);
//}

void AShoutingZombieAIController::UpdateLastKnownPositionByFootSound(FVector playerlocation)
{
	m_bFootSound = true;
	m_vFootCharacterPos = playerlocation;
}

//void AShoutingZombieAIController::StopAI()
//{
//	auto BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
//	if (nullptr != BehaviorTreeComponent)
//	{
//		BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
//	}
//}

//void AShoutingZombieAIController::StartAI()
//{
//	auto BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
//	if (nullptr != BehaviorTreeComponent)
//	{
//		BehaviorTreeComponent->StartTree(*this->ShoutingZombieAIBehavior, EBTExecutionMode::Looped);
//	}
//}
