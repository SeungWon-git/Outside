// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProGamemode/ProGameInstance.h"
#include "ProItem/ItemBoxActor.h"

#include <atomic>

// 구조체
#include "ProData/ItemDataStructure.h"

#include "BaseCharacter.generated.h"


class UGamePlayerUI;
class UConditionUI;
class UProGameClearUI;
class UPlayerSight;
class ANormalWeaponActor;
class AThrowWeaponActor;
class AHealingItemActor;
class ABleedingHealingItemActor;
class AKeyActor;
class UGameTimerUI;
class UPickUpUI;
class UCircularPB_UI;
class AHealingNiagaEffect;
class UTextMissionUI;
class UOtherPlayerUI;
class UGameChatUI;
class AInterActor;


DECLARE_DELEGATE_SevenParams(FThrowOnGround, FName, EItemClass, UTexture2D*, int, float, float, int);
DECLARE_MULTICAST_DELEGATE(FAttackEndPlayerDelegate);
DECLARE_MULTICAST_DELEGATE(FPickUpEndPlayerDelegate);
DECLARE_MULTICAST_DELEGATE(FBleedHealingEndPlayerDelegate);


// 플레이어 캐릭터들의 부모클래스
UCLASS()
class PROJECT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	bool DraggingSwap(int from, ESlotType fromtype, int to, ESlotType totype);
	bool SwapInven(int from, int to);

	// 바닥에 떨어트리는 델리게이트 
	FThrowOnGround ThrowOnGround;
	void SpawnOnGround(int slotindex);
	void SpawnAllOnGround();

	//void AttackCheck();

	// 스프링 암
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;

	// 카메라
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	// 플레이어가 물체를 바라보는지
	UPROPERTY(EditAnywhere)
	UPlayerSight* PlayerSight;

	// 퀵슬롯에 있는 아이템들 넣을 예정
	UPROPERTY(EditAnywhere)
	ANormalWeaponActor* CurrentWeapon;

	UPROPERTY(EditAnywhere)
	AThrowWeaponActor* CurrentThrowWeapon;

	UPROPERTY(EditAnywhere)
	AHealingItemActor* CurrentHealingItem;

	UPROPERTY(EditAnywhere)
	ABleedingHealingItemActor* CurrentBleedingHealingItem;

	UPROPERTY(EditAnywhere)
	AKeyActor* CurrentKeyItem;

	UPROPERTY(EditAnywhere)
	AHealingNiagaEffect* HealingFX;

	UPROPERTY(EditAnywhere)
	USpotLightComponent* FlashLight;

	// 게임 전체 UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> QuickSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UGamePlayerUI> GameUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UGamePlayerUI* GameUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UConditionUI> ConditionUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UConditionUI* ConditionUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UProGameClearUI> ProGameClearUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UProGameClearUI* ProGameClearUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UGameTimerUI> GameTimerUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UGameTimerUI* GameTimerUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> PickUpSlot;	// 이건 대체 왜 배열로 선언함?;; 어차피 하나만 쓰면서...

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPickUpUI> PickUpUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UPickUpUI* PickUpUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTextMissionUI> TextMissionUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTextMissionUI* TextMissionUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOtherPlayerUI> OtherPlayerUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UOtherPlayerUI* OtherPlayerUIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> OtherPlayerInven;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOtherPlayerUI> OtherPlayer2UIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UOtherPlayerUI* OtherPlayer2UIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> OtherPlayer2Inven;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOtherPlayerUI> OtherPlayer3UIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UOtherPlayerUI* OtherPlayer3UIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UGameChatUI> GameChatUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UGameChatUI* GameChatUIWidget;

	// 게임 전체 UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FItemDataStructure> OtherPlayer3Inven;

	// 텍스트를 화면에 표시하는 함수
	void ShowActionText(FText Text, const FSlateColor& Color, float DisplayTime = 5); // fadeout이 5초

	// 텍스트를 화면에 표시하는 함수(DEATH)
	void ShowDeathActionText(FText Text, const FSlateColor& Color, float DisplayTime = 5);

	void ShowMissionText(FText Text, const FSlateColor& Color, int TextNumber);


	FTimerHandle CarKeyFindHandle;
	void CarKeyFindUpdateUI();

	FTimerHandle RoofKeyFindHandle;
	void RoofKeyFindUpdateUI();

	void OnPickUPUISlot();

	FTimerHandle PickUpUIHandle;
	void ProStartPickUpUI();

	FTimerHandle GameOverUIHandle;
	void ProStartGameOverUI();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCircularPB_UI> CircularPB_Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UCircularPB_UI* CircularPB_Widget;

	UFUNCTION()
	void AttackMontageEnded(UAnimMontage* Montage, bool interrup);

	UFUNCTION()
	void HealingMontageEnded(UAnimMontage* Montage, bool interrup);

	UFUNCTION()
	void BleedHealingMontageEnded(UAnimMontage* Montage, bool interrup);

	UFUNCTION()
	void KeyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void PickUpMontageEnded(UAnimMontage* Montage, bool interrup);

	//Sound
	void PlaySoundAtLocationForPlayer(USoundBase* Sound, FVector Location);
	void PlaySoundForPlayer(USoundBase* Sound);

	// input
	void MoveForward(FVector RotateYaw, float NewAxisValue);
	void MoveLeft(FVector RotateYaw, float NewAxisValue);
	void Run();
	void Other_Run(bool mbrun);
	void LookUp(float NewAxisValue);
	void Turn(float NewAxisValue);
	void GetItem();
	//void OtherGetItem(); => 사용 X
	void LightOnOff();
	void InventoryOnOff();

	FAttackEndPlayerDelegate m_DAttackEnd;
	void Attack(int attack_type);	// attack_type = 1: 세로-대각 베기 / = 2: 가로 베기 

	void Healing();
	void BleedHealing();
	void PlayKey();
	void Throw();

	void UpdateHealingSlot();
	void UpdateBHealingSlot();
	void UpdateKeySlot();
	void UpdateNormalWeaponSlot();

	FPickUpEndPlayerDelegate m_DPickUpEnd;
	void PickUp();

	void OtherPickUp();

	void QuickNWeapon();
	void QuickBHItem();
	void QuickHItem();
	void QuickTWeapon();
	void QuickKeyItem();


	void UpSTR();
	void DownSTR();
	void UpSpeed();
	void DownSpeed();


	// 행동 확인 Get 함수 08/18
	bool IsAttack() { return m_bIsAttacking; }
	bool IsHealing() { return m_bIsHealingTime; }
	bool IsBHealing() { return m_bIsBleedHealing; }
	// bool IsKey() { return m_bIsk; }
	// bool IsTH() { return m_bIsk; }

	FRotator GetSpringArmRot() { return m_rSpringArmRot; }
	void SetSpringArmRot(FRotator springarmrot) { m_rSpringArmRot = springarmrot; }

	float GetSpringArmPitch() { return m_fPitch; }
	void SetSpringArmPitch(float pitch) { m_fPitch = pitch; }

	float GetHP() { return m_fHP; }
	void SetHP(float hp) { m_fHP = hp; }

	float GetMaxHP() { return m_fMaxHP; }
	void SetMaxHP(float maxhp) { m_fMaxHP = maxhp; }

	float GetSTR() { return m_fSTR; }
	void SetSTR(float str) { m_fSTR = str; }

	float GetSpeed() { return m_fSpeed; }
	void SetSpeed(float speed) { m_fSpeed = speed; }

	float GetBasicSpeed() { return m_fBasicSpeed; }
	void SetBasicSpeed(float speed) { m_fBasicSpeed = speed; }

	bool IsUsingKey() { return bIsKeyPlaying; }

	float GetStamina() { return m_fStamina; }
	void SetStamina(float stamina) { m_fStamina = stamina; }

	float GetMaxStamina() { return m_fMaxStamina; }
	void SetMaxStamina(float maxstamina) { m_fMaxStamina = maxstamina; }

	float GetStaminaHealing() { return m_fstaminaHealing; }
	void SetStaminaHealing(float staminahealing) { m_fstaminaHealing = staminahealing; }

	float GetHealing() { return m_fHealing; }
	void SetHealing(float Healing) { m_fHealing = Healing; }

	bool IsBleeding() { return m_bBleeding; }
	void SetBleeding(bool bleeding) { m_bBleeding = bleeding; }

	bool IsSitDown() { return m_bSitDown; }
	void SetSitDown(bool sitdown) { m_bSitDown = sitdown; }

	bool IsRun() { return m_bRun; }
	void SetRun(bool run) { m_bRun = run; }


	bool IsNWHandIn() { return m_bNWHandIn; }
	void SetNWHandIn(bool handin) { m_bNWHandIn = handin; } 
	// 이 함수를 이렇게 한줄로 하지 말고  m_bNWHandIn = handin; 이후에 m_bNWHandin값을 send해주고 
	// 그 값을 받아 다른 클라에 해당 캐릭터에 m_bNWHandIn만 수정해주는 함수를 만들면 될듯?
	bool IsBHHandIn() { return m_bBHHandIn; }
	void SetBHHandIn(bool handin) { m_bBHHandIn = handin; }

	bool IsHealHandIn() { return m_bHealHandIn; }
	void SetHealHandIn(bool handin) { m_bHealHandIn = handin; }

	bool IsKeyHandIn() { return m_bKeyHandIn; }
	void SetKeyHandIn(bool handin) { m_bKeyHandIn = handin; }

	bool IsThrowWHandIn() { return m_bThrowWHandIn; }
	void SetThrowWHandIn(bool handin) { m_bThrowWHandIn = handin; }

	bool IsDeadNoCollision() { return m_bIsDead_NoCol; }
	void SetDeadNoCollision(bool no_col) { m_bIsDead_NoCol = no_col; }

	bool IsDead() { return m_bIsDead; }
	void SetDead(bool dead) { m_bIsDead = dead; }

	bool IsDeadPlay() { return m_bIsDeadPlay; }
	void SetDeadPlay(bool deadplay) { m_bIsDeadPlay = deadplay; }

	bool IsBringCurrentWeapon() { return m_bIsBringCurrentWeapon; }
	void SetBringCurrentWeapon(bool bringcurrentweapon) { m_bIsBringCurrentWeapon = bringcurrentweapon; }

	bool IsBringCurrentBleedingHealingItem() { return m_bIsBringCurrentBleedingHealingItem; }
	void SetBringCurrentBleedingHealingItem(bool bringcurrentbleedinghealingitem) { m_bIsBringCurrentBleedingHealingItem = bringcurrentbleedinghealingitem; }

	bool IsBringCurrentHealingItem() { return m_bIsBringCurrentHealingItem; }
	void SetBringCurrentHealingItem(bool bringcurrenthealingitem) { m_bIsBringCurrentHealingItem = bringcurrenthealingitem; }

	bool IsBringCurrentThrowWeapon() { return m_bIsBringCurrentThrowWeapon; }
	void SetBringCurrentThrowWeapon(bool bringcurrentthrowweapon) { m_bIsBringCurrentThrowWeapon = bringcurrentthrowweapon; }

	bool IsBringCurrentKeyItem() { return m_bIsBringCurrentKeyItem; }
	void SetBringCurrentKeyItem(bool bringcurrentkeyitem) { m_bIsBringCurrentKeyItem = bringcurrentkeyitem; }

	//bool IsSpecialEffect() { return m_bSpecialEffect; }	
	//void SetSpecialEffect(bool specialeffect) { m_bSpecialEffect = specialeffect; }

	bool IsSpotLight() { return m_bOnSpotLight; }
	void SetSpotLight(bool spotlight) { m_bOnSpotLight = spotlight; }

	bool IsInventory() { return m_bInventoryOn; }
	void SetInventory(bool inven) { m_bInventoryOn = inven; }

	void SetCharacterName(FName charactername) { m_nCharacterName = charactername; };
	FName GetCharacterName() { return m_nCharacterName; }

	void SetCarryBagName(FName carrybagname) { m_nCarryBagName = carrybagname; };
	FName GetCarryBagName() { return m_nCarryBagName; }

	void SetInvenSize(int invensize) { m_iInvenSize = invensize; };
	int GetInvenSize() { return m_iInvenSize; }

	void SetStartLocation(FVector startlocation) { m_VStartLocation = startlocation; }
	FVector GetStartLocation() { return m_VStartLocation; }

	bool CanSetWeapon();
	// void SetWeapon(ANormalWeaponActor* NewWeapon); // 아마도 안쓸예정

	void GameUIUpdate();

	// 퀵슬롯 장착 시 생성
	void SpawnNormalWeapon();
	void SpawnThrowWeapon();
	void SpawnHealingItem();
	void SpawnBleedingHealingItem();
	void SpawnKeyItem();

	// 퀵슬롯 해제 시 삭제
	void DestroyNormalWeapon();
	void DestroyThrowWeapon();
	void DestroyHealingItem();
	void DestroyBleedingHealingItem();
	void DestroyKeyItem();


	// 슬롯에 아이템 내용 삭제
	void DestroyNormalWepaonItemSlot();
	void DestroyThrowWeaponItemSlot();
	void DestroyHealingItemSlot();
	void DestroyBleedingHealingItemSlot();
	void DestroyKeyItemSlot();

	void FootSound();

	// 체력 Timer
	void StartHealingTimer(float healingspeed, float healingduration);
	void HealingTimerElapsed();

	FTimerHandle HealingHandle;

	UPROPERTY(EditAnywhere)
	bool m_bIsHealing = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsHealingTime = false;

	UPROPERTY(EditAnywhere)
	float m_fHealingCount = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fItemHealingSpeed = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fItemHealingDuration = 0.f;

	void AddScore(int32 score);

	UPROPERTY(EditAnywhere)
	int32 m_iClearScore = 0;


	FTimerHandle GameEndHandle;
	void ProStartGameEnd();
	void ProGameEnd();

	// 소방관
	virtual void Smoking(AHealingItemActor* smokeActor) {};

	// 출혈 회복
	FBleedHealingEndPlayerDelegate m_DBleedingHealingEnd;
	FTimerHandle BleedingHandle;
	void StartBleedingTimer();
	void BleedingTimerElapsed();

	bool RandomBleeding();
	bool RandomBleedHealing(float bhpercent);

	UPROPERTY(EditAnywhere)
	bool m_bIsBleedHealing = false;

	UPROPERTY(EditAnywhere)
	float m_fBleedPercent = 0.3f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	AInterActor* CurrentInterActor;

	UPROPERTY(EditAnywhere)
	int m_iSetPlayerID = 0;

	void OtherSpawnItemBefore();
	void OtherUnEquipItem(uint32 itemtype);

	UPROPERTY(EditAnywhere)
	int32 GameClearFlag = 0;

	void LimitSmokingIcon();
	void SmokingIcon();
	void RunWalkIcon(bool bisrun);
	void KeyIcon(bool bisbringkey);

	// 스태미나 부분
	FTimerHandle UseStaminaHandle;

	void UseStamina();
	void UseStaminaTimerElapsed();
	void StaminaWaitingByZero();

	FTimerHandle HealingStaminaHandle;
	void HealingStamina();
	void HealingStaminaTimerElapsed();

	void PlayKeyAnim(float PlaySpeed);

	void UpdateOpenKey(uint32 keyindex);

	UPROPERTY(EditAnywhere)
	bool m_bZeroStamina = false;

	void PlayDead();

	void OtherPlayerUIOffset(uint32 playerid);
	void UpdateOtherPlayerUI(uint32 playerid, float hp, uint32 charactertype, std::string Playername);
	void UpdatePickUpKey(uint32 keyid, uint32 playerid);

	void ProGameClear(uint32 root, uint32 alive_players, uint32 dead_players,
		const FString& open_player, uint32 my_kill_count,
		uint32 best_kill_count, const FString& best_kill_player);

	FTimerHandle GameDeadEndHandle;
	void ProStartGameDeadEnd();
	void ProGameDeadEnd();

	FTimerHandle GameTimerEndHandle;
	void ProStartGameTimerEnd();
	void ProGameTimerEnd();

	//무기 깨졌을때에 문구	
	void CommentDestroyWeapon();

	virtual uint32 GetPlayerId() const;
	void SetPlayerId(uint32 NewPlayerId);
	std::string GetPlayerName() const;
	void SetPlayerName(std::string NewPlayerName);
	void UpdatePlayerData(FVector Location);

	void SetAttack(bool b_attack, int attack_type, float aimoffset);	// attack_type = 1: 세로-대각 베기 / = 2: 가로 베기 

	void SetPickUp();

	void SetPlayerJump();

	bool GetAttack();

	void OtherSpawnNormalWeapon(const FString& WeaponName);

	void OtherSpawnBleedingHealingItem(const FString& ItemName);

	void OtherSpawnHealingItem(const FString& ItemName);

	void OtherSpawnThrowWeapon(const FString& ItemName);

	void OtherSpawnKeyItem(const FString& ItemName);

	void Send_Destroy(uint32 itemboxid);

	void PickUpItem(AItemBoxActor* itemboxid);

	void Send_GetKey(uint32 itemid, uint32 itemboxid);

	void Send_OpenRoot(uint32 itemid, uint32 carid);

	void Send_DetachItem(uint32 itemtype);

	float Get_AimOffSet();


private:

	UPROPERTY(EditAnywhere)
	bool m_bIsStanding = false;

	UPROPERTY(EditAnywhere)
	float m_fHP = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fMaxHP = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fSTR = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fSpeed = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fBasicSpeed = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fStamina = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fMaxStamina = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fstaminaHealing = 0.f;

	UPROPERTY(EditAnywhere)
	float m_fHealing = 0.f;		// 얘 딱히 쓰는데 없음...

	UPROPERTY(EditAnywhere)
	bool m_bBleeding = false;

	UPROPERTY(EditAnywhere)
	bool m_bSitDown = false;		// 얘 딱히 쓰는데 없음...

	UPROPERTY(EditAnywhere)
	bool m_bRun = false;

	UPROPERTY(EditAnywhere)
	bool m_bJump = false;

	// 인벤토리
	UPROPERTY(EditAnywhere)
	bool m_bNWHandIn = false;

	UPROPERTY(EditAnywhere)
	bool m_bBHHandIn = false;

	UPROPERTY(EditAnywhere)
	bool m_bHealHandIn = false;

	UPROPERTY(EditAnywhere)
	bool m_bKeyHandIn = false;

	UPROPERTY(EditAnywhere)
	bool m_bThrowWHandIn = false;

	//UPROPERTY(EditAnywhere)
	//bool m_bSpecialEffect = false;	// 이거 안 씀;;

	UPROPERTY(EditAnywhere)
	FName m_nCarryBagName;

	UPROPERTY(EditAnywhere)
	int m_iInvenSize = 5;				

	// 어택
	UPROPERTY(EditAnywhere)
	bool m_bIsAttacking = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsPickUping = false;

	// 손전등이 켜져있는지
	UPROPERTY(EditAnywhere)
	bool m_bOnSpotLight = true;

	UPROPERTY(EditAnywhere)
	bool m_bInventoryOn = false;

	UPROPERTY(EditAnywhere)
	FVector m_VStartLocation;

	UPROPERTY(EditAnywhere)
	FName m_nCharacterName;

	// 현재 
	UPROPERTY(EditAnywhere)
	bool m_bIsBringCurrentWeapon;

	UPROPERTY(EditAnywhere)
	bool m_bIsBringCurrentBleedingHealingItem;

	UPROPERTY(EditAnywhere)
	bool m_bIsBringCurrentHealingItem;

	UPROPERTY(EditAnywhere)
	bool m_bIsBringCurrentThrowWeapon;

	UPROPERTY(EditAnywhere)
	bool m_bIsBringCurrentKeyItem;

	UPROPERTY(EditAnywhere)
	bool m_bIsDead_NoCol = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsDead = false;

	UPROPERTY(EditAnywhere)
	bool m_bIsDeadPlay = false;

	UPROPERTY(EditAnywhere)
	int m_iHealingMontageFlag;

	UPROPERTY(EditAnywhere)
	int m_iBleedHealingMontageFlag;

	UPROPERTY(EditAnywhere)
	int m_iAttackMontageFlag;

	UPROPERTY(EditAnywhere)
	bool bIsKeyPlaying;

	UPROPERTY(EditAnywhere)
	int KeyMontageFlag;

	UPROPERTY(EditAnywhere)
	int m_iPickUpMontageFlag;

	UPROPERTY(EditAnywhere)
	int m_iOpenRoofKey;

	// 스프링암 회전값
	UPROPERTY(EditAnywhere)
	FRotator m_rSpringArmRot;

	UPROPERTY(EditAnywhere)
	float m_fPitch;

	float LastStopAnimTime = 0.f;

private:
	UPROPERTY(VisibleAnywhere)
	uint32 PlayerId = 99;		// 나 자신은 99번(초기값) 고정이고 ,다른 클라는 실제 서버에서 관리하는 플레이어 DB에 인덱스를 따름 (+ 그래서 나 자신의 실제 서버 DB의 인덱스 알고 싶으면 ClientSocket->MyPlayerId 사용)
	std::string PlayerName;

	float PreviousSpeed;

public:
	UProGameInstance* GameInstance;

	std::atomic<bool> b_attack{ false };
	bool b_run = false;

	FVector NewLocation;
	FVector OldLocation = FVector(0.0f, 0.0f, 0.0f);

	float Speed;

	uint32 GetItemBox_Id = 0;


	uint32 BPlayerCount = 0;
	uint32 BPlayerThrowCount = 0;


	enum FLOOR {
		Zero, B2, B1, F1, F2, F3
	};

	UPROPERTY(EditAnywhere)
	int floor = FLOOR::Zero;

	float RelativeHeight_ZOffset = 0; // 캐릭마다 키 변경에 따른 공중 떠있기 방지하기 위해 사용

};
