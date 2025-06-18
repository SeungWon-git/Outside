// Fill out your copyright notice in the Description page of Project Settings.

#include "ProGamemode/OneGameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "ProCharacter/BaseCharacter.h"
#include "ProCharacter/EmployeeCharacter.h"
#include "ProCharacter/FireFighterCharacter.h"
#include "ProCharacter/GirlCharacter.h"
#include "ProCharacter/IdolCharacter.h"
#include "ProCharacter/PlayerCharacterController.h"
#include "ProZombie/BaseZombie.h"
#include "ProZombie/NormalZombie.h"
#include "ProZombie/ShoutingZombie.h"
#include "ProZombie/RunningZombie.h"
#include "ProZombie/RunningZombieAIController.h"
#include "ProZombie/ShoutingZombieAIController.h"
#include "ProZombie/ZombieAIController.h"
#include "Math/UnrealMathUtility.h"
#include "ProItem/ItemBoxActor.h"
#include "ProItem/InterActor.h"
#include "ProItem/CarActor.h"
#include "ProItem/RoofTopDoorActor.h"
#include "ClientSocket.h"
#include "EngineUtils.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerStart.h"
#include "ProGamemode/GameModeManager.h"
#include "ProGamemode/LobbyGameMode.h"
#include "ProGamemode/ProGameInstance.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"
#include "GStruct.pb.h"
#include "ProCharacter/PlayerCharacterAnimInstance.h"

#include "NavMesh/RecastNavMesh.h"


TMap<uint32, ABaseZombie*> ZombieMap;


AOneGameModeBase::AOneGameModeBase()
{
    //DefaultPawnClass = ABaseCharacter::StaticClass();
    //PlayerControllerClass = APlayerCharacterController::StaticClass();


    // 변수들
    m_iZombieNumber = 0;
    m_iShoutingZombieNumber = 0;
    m_iRunningZombieNumber = 0;

    // 플레이어를 선택한 것을 받아서 넘겨주려고 할 예정
    CharacterIconIndex = EPlayerCharacter::EMPLOYEE;

    CarActorRandomLocationSetting();

    CarKeyRandomSetting();

    SpawnInterActorStaticClasses();
}

void AOneGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    if (GameInstance == nullptr) {
        GameInstance = Cast<UProGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    }

    ZombieMap.Empty(); // ZombieMap 초기화

    if (GEngine)
    {
        //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat FPS"));  // fps 카운터 ON => 작동 안돼서 주석 처리;; (콘솔 명령어로 직접 입력하면 작동함)

        GEngine->Exec(GetWorld(), TEXT("DisableAllScreenMessages"));    // 게임 시작시에 인게임 화면 위 모든 로그 출력 일단 disable (콘솔창 따로 열어서 다시 킬 수 있음)
    }

    //ABaseCharacter* DefaultPawn = nullptr;
    //
    //UWorld* World = GetWorld();
    //if (World) {
    //
    //    for (TActorIterator<ABaseCharacter> ActorItr(World); ActorItr; ++ActorItr) {
    //        UE_LOG(LogTemp, Log, TEXT("DefaultPawn Complete"));
    //        DefaultPawn = *ActorItr;
    //        if (DefaultPawn) {
    //            break;
    //        }
    //    }
    //
    //    DefaultPawn->ThrowOnGround.BindUObject(this, &AOneGameModeBase::SpawnOnGroundItem);
    //}

 
    // commit debug용 
    //TArray<FVector> OutVertices;
    //TArray<FVector> Outlines;
    //TArray<FVector> OutPoly;
    //ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(UNavigationSystemV1::GetCurrent(World)->MainNavData);
    //
    //
    //if (NavMesh)
    //{
    //    FRecastDebugGeometry NavMeshGeometry;
    //    NavMesh->GetDebugGeometry(NavMeshGeometry);
    //    //NavMesh->GetPathSegmentBoundaryEdges(NavMeshGeometry);       // 이거 Edge 받을 수 있는 거 처럼 생겼는데 인자 뭐 넘겨 줘야 되는 지 모르겠음;;
    //    //NavMesh->GetPolyEdges();
    //    
    //    Outlines.Append(NavMeshGeometry.NavMeshEdges);  // 사실상 안 담김
    //    OutPoly.Append(NavMeshGeometry.PolyEdges);      // 사실상 안 담김
    //
    //
    //
    //    int i = 0;
    //    // 데이터를 텍스트 파일로 저장하기 위한 문자열로 변환
    //    FString OutlineData;
    //    for (const FVector& Edge : Outlines)
    //    {
    //        i++;
    //        OutlineData += FString::Printf(TEXT("%f,%f,%f\n"), Edge.X, Edge.Y, Edge.Z);
    //    }
    //
    //    UE_LOG(LogTemp, Log, TEXT("Outlines :: %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"), i);
    //
    //    // 데이터를 텍스트 파일로 저장하기 위한 문자열로 변환
    //    FString OutPolyData;
    //    for (const FVector& Poly : OutPoly)
    //    {
    //        OutPolyData += FString::Printf(TEXT("%f,%f,%f\n"), Poly.X, Poly.Y, Poly.Z);
    //    }
    //
    //    // 저장할 파일 경로 설정 (프로젝트 디렉토리)
    //    FString FilePath = FPaths::ProjectDir() + TEXT("NavMeshEdges.txt");
    //    FString FilePathPol = FPaths::ProjectDir() + TEXT("NavMeshPolyes.txt");
    //
    //    // 파일에 데이터 저장
    //    FFileHelper::SaveStringToFile(OutlineData, *FilePath);
    //    FFileHelper::SaveStringToFile(OutPolyData, *FilePathPol);
    //
    //    OutVertices.Append(NavMeshGeometry.MeshVerts);
    //
    //    for (const FVector& Vertex : NavMeshGeometry.MeshVerts)
    //    {
    //        DrawDebugPoint(GetWorld(), Vertex, 10.0f, FColor::Red, false, -1.0f, 0);
    //    }
    //
    //    FString FilePathB2 = FPaths::ProjectDir() + TEXT("B2.txt");
    //    FString FilePathB1 = FPaths::ProjectDir() + TEXT("B1.txt");
    //    FString FilePathF1 = FPaths::ProjectDir() + TEXT("F1.txt");
    //    FString FilePathF2 = FPaths::ProjectDir() + TEXT("F2.txt");
    //    FString FilePathF3 = FPaths::ProjectDir() + TEXT("F3.txt");
    //
    //
    //    FString NodeDataB2;
    //    FString NodeDataB1;
    //    FString NodeDataF1;
    //    FString NodeDataF2;
    //    FString NodeDataF3;
    //
    //    for (const FVector& Node : OutVertices)
    //    {
    //        if (Node.Z < 800.f) {
    //            NodeDataB2 += FString::Printf(TEXT("%f,%f,%f\n"), Node.X, Node.Y, Node.Z);
    //        }
    //        else if (Node.Z < 1800.f) {
    //            NodeDataB1 += FString::Printf(TEXT("%f,%f,%f\n"), Node.X, Node.Y, Node.Z);
    //        }
    //        else if (Node.Z < 2500.f) {
    //            NodeDataF1 += FString::Printf(TEXT("%f,%f,%f\n"), Node.X, Node.Y, Node.Z);
    //        }
    //        else if (Node.Z < 3600.f) {
    //            NodeDataF2 += FString::Printf(TEXT("%f,%f,%f\n"), Node.X, Node.Y, Node.Z);
    //        }
    //        else {
    //            NodeDataF3 += FString::Printf(TEXT("%f,%f,%f\n"), Node.X, Node.Y, Node.Z);
    //        }
    //    }
    //
    //    FFileHelper::SaveStringToFile(NodeDataB2, *FilePathB2);
    //    FFileHelper::SaveStringToFile(NodeDataB1, *FilePathB1);
    //    FFileHelper::SaveStringToFile(NodeDataF1, *FilePathF1);
    //    FFileHelper::SaveStringToFile(NodeDataF2, *FilePathF2);
    //    FFileHelper::SaveStringToFile(NodeDataF3, *FilePathF3);
    //
    //}

    //==========================================
    {
        //// 현재 월드에서 네비게이션 시스템을 가져옵니다.
        //UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
        //if (NavSystem)
        //{
        //    // 네비게이션 데이터 인스턴스를 가져옵니다.
        //    ARecastNavMesh* NavData = Cast<ARecastNavMesh>(NavSystem->GetDefaultNavDataInstance());
        //    if (NavData)
        //    {
        //        // NavData를 통해 NavMesh 세부 정보에 접근할 수 있습니다.
        //        //const TArray<FNavMeshTile>& Tiles = NavData->GetNavMeshTiles();
        //        // Tiles를 통해 폴리곤 및 정점 정보를 가져옵니다.
        //        
        //        
        //        
        //        FBox Tiles = NavData->GetNavMeshTileBounds(0);
        //    }
        //}
    }
    //==========================================
}

void AOneGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    ChoiceCharacterBefore();
    ChoiceCharacter();
}


void AOneGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
}

void AOneGameModeBase::ChoiceCharacterBefore()
{
    if (GameInstance == nullptr) {
        GameInstance = Cast<UProGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    }

    if (GameInstance) {
        int CharacterNumber = GameInstance->GetChoicedCharacterNumber();
        if (CharacterNumber == 1) {
            CharacterIconIndex = EPlayerCharacter::GIRL;
        }
        else if (CharacterNumber == 2) {
            CharacterIconIndex = EPlayerCharacter::EMPLOYEE;
        }
        else if (CharacterNumber == 3) {
            CharacterIconIndex = EPlayerCharacter::IDOL;
        }
        else if (CharacterNumber == 4) {
            CharacterIconIndex = EPlayerCharacter::FIREFIGHTER;
        }
        else {
            // 게임 인스턴스가 유효하지 않은 경우에 대한 처리
        }
    }
    else {
        // 게임 인스턴스가 아직 생성되지 않은 경우에 대한 처리
    }
}


void AOneGameModeBase::ChoiceCharacter()
{
    if (CharacterIconIndex == EPlayerCharacter::EMPLOYEE) {
        PlayerCharacterClasses.Add(AEmployeeCharacter::StaticClass());
    }
    else if (CharacterIconIndex == EPlayerCharacter::FIREFIGHTER) {
        PlayerCharacterClasses.Add(AFireFighterCharacter::StaticClass());
    }
    else if (CharacterIconIndex == EPlayerCharacter::GIRL) {
        PlayerCharacterClasses.Add(AGirlCharacter::StaticClass());
    }
    else if (CharacterIconIndex == EPlayerCharacter::IDOL) {
        PlayerCharacterClasses.Add(AIdolCharacter::StaticClass());
    }


    PlayerCharacterControllerClasses.Add(APlayerCharacterController::StaticClass());

    DefaultPawnClass = PlayerCharacterClasses[0];
    PlayerControllerClass = PlayerCharacterControllerClasses[0];
}

//AActor* AOneGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
//{
//    // 선택된 캐릭터 타입에 따라 태그 설정
//    FName DesiredTag;
//
//    UProGameInstance* GameInstance = Cast<UProGameInstance>(GetGameInstance());
//    uint32 playerid = GameInstance->ClientSocketPtr->GetMyPlayerId();
//
//    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart_Implementation -> playerid: %d"), playerid);
//    if (playerid == 1)
//    {
//        DesiredTag = FName("Start1");
//    }
//    else if (playerid == 2)
//    {
//        DesiredTag = FName("Start4");
//    }
//    else if (playerid == 3)
//    {
//        DesiredTag = FName("Start2");
//    }
//    else if (playerid == 4)
//    {
//        DesiredTag = FName("Start3");
//    }
//
//    // 월드의 모든 PlayerStart 검색
//    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
//    {
//        APlayerStart* PlayerStart = *It;
//        if (PlayerStart->Tags.Contains(DesiredTag))
//        {
//            return PlayerStart; // 적합한 PlayerStart 반환
//        }
//    }
//
//    // 태그에 맞는 PlayerStart를 찾지 못한 경우 기본 로직 사용
//    return Super::ChoosePlayerStart_Implementation(Player);
//}


int32 AOneGameModeBase::RandomCarActorLocation()
{
    int32 RandomNumber = 0;

    while (true) {
        RandomNumber = FMath::RandRange(0, 6);

        if (!CarActorRandomLocationStruct[RandomNumber].bIsSeatLocation) {
            CarActorRandomLocationStruct[RandomNumber].bIsSeatLocation = true;
            return RandomNumber;
        }
    }


    return -1;
}

int32 AOneGameModeBase::RandomCarKey()
{
    int32 RandomNumber = 0;

    while (true) {
        RandomNumber = FMath::RandRange(0, 6);

        if (!CarKeyRandom[RandomNumber].bIsSeatCarKey) {
            CarKeyRandom[RandomNumber].bIsSeatCarKey = true;
            return RandomNumber;
        }
    }

    return -1;
}

// 최초로 맵에 아이템 생성시 (월드에 아이템 박스 생성)
void AOneGameModeBase::SpawnItemBoxes(int32 itemid, FName itemname, uint32 itemclass, UTexture2D* texture, int count, uint32 itemfloor, FVector itempos)
{
    int32 itemboxindex = itemid - 1;
    if (itemboxindex >= 60) {
        UE_LOG(LogTemp, Warning, TEXT("SpawnItemBoxes -> itemid: 60 over!!!! itemid : %d"), itemid);
        return;
    }

    if (ItemBoxClasses.Num() <= itemboxindex) {
        //UE_LOG(LogTemp, Warning, TEXT("SpawnItemBoxes -> itemid: %d"), original_itembox_id);
        ItemBoxClasses.SetNum(itemboxindex + 1);
    }

    if (ItemBoxClasses.IsValidIndex(itemboxindex)) {
        if (!ItemBoxClasses[itemboxindex]) {
            ItemBoxClasses[itemboxindex] = AItemBoxActor::StaticClass();
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Invalid index: %d for ItemBoxClasses array"), itemboxindex);
        return;
    }


    TSubclassOf<AItemBoxActor> SelectedItemBoxClass = ItemBoxClasses[itemboxindex];

    EItemClass iclass{};

    if (itemclass == 0) {
        iclass = EItemClass::NORMALWEAPON;
    }
    else if (itemclass == 1) {
        iclass = EItemClass::THROWINGWEAPON;
    }
    else if (itemclass == 2) {
        iclass = EItemClass::HEALINGITEM;
    }
    else if (itemclass == 3) {
        iclass = EItemClass::BLEEDINGHEALINGITEM;
    }
    else if (itemclass == 4) {
        iclass = EItemClass::KEYITEM;
    }
    else if (itemclass == 5) {
        iclass = EItemClass::BAGITEM;
    }
    else if (itemclass == 6) {
        iclass = EItemClass::NONE;
    }

    // 선택된 아이템 박스 클래스로 실제로 월드에 아이템 박스 생성 
    AItemBoxActor* SpawnedItemBox = GetWorld()->SpawnActor<AItemBoxActor>(SelectedItemBoxClass, itempos, FRotator::ZeroRotator);

    //UE_LOG(LogTemp, Error, TEXT("ITEM___111"));
    if (SpawnedItemBox) {
        SpawnedItemBox->ItemName = itemname;
        SpawnedItemBox->ItemClassType = iclass;
        SpawnedItemBox->Texture = texture;
        SpawnedItemBox->Count = count;
        SpawnedItemBox->ItemBoxId = itemid;

        //UE_LOG(LogTemp, Warning, TEXT("SpawnItemBoxes -> SpawnedItemBox->ItemBoxOriginalId (= Index + 1) : %d"), itemid);

        float durability = 0;
        if (itemclass == 0) {  // 생성하는 박스가 무기일때 내구도 최초 설정
            if (itemname == "Book") { 
                durability = WeaponDurability::Book;
            }
            else if (itemname == "Bottle") {
                durability = WeaponDurability::Bottle;
            }
            else if (itemname == "ButchersKnife") {
                durability = WeaponDurability::ButchersKnife;
            }
            else if (itemname == "FireAxe") {
                durability = WeaponDurability::FireAxe;
            }
            else if (itemname == "FireExtinguisher") { 
                durability = WeaponDurability::FireExtinguisher;
            }
            else if (itemname == "FryingPan") { 
                durability = WeaponDurability::FryingPan;
            }        
            else if (itemname == "GolfClub") {
                durability = WeaponDurability::GolfClub;
            }       
            else if (itemname == "Iron") { 
                durability = WeaponDurability::Iron;
            }        
            else if (itemname == "MagicStick") { 
                durability = WeaponDurability::MagicStick;
            }        
            else if (itemname == "MannequinArm") { 
                durability = WeaponDurability::MannequinArm;
            }       
            else if (itemname == "MannequinLeg") { 
                durability = WeaponDurability::MannequinLeg;
            }        
            else if (itemname == "Pipe") {
                durability = WeaponDurability::Pipe;
            }        
            else if (itemname == "Plunger") { 
                durability = WeaponDurability::Plunger;
            }
            else if (itemname == "SashimiKnife") {
                durability = WeaponDurability::SashimiKnife;
            }
            else if (itemname == "Scissors") {
                durability = WeaponDurability::Scissors;
            }
            else if (itemname == "Shovels") {
                durability = WeaponDurability::Shovels;
            }
            else if (itemname == "SquareWood") {
                durability = WeaponDurability::SquareWood;
            }
            else if (itemname == "WoodenBat") {
                durability = WeaponDurability::WoodenBat;
            }
        }

        SpawnedItemBox->Durability = durability;
        SpawnedItemBox->Durability_Max = SpawnedItemBox->Durability;
    }
    
    //UE_LOG(LogTemp, Warning, TEXT("SpawnItemBoxes -> ItemBoxClasses.Num(): %d"), ItemBoxClasses.Num());
}

void AOneGameModeBase::NullPtrItemBoxesIndex(int32 itemboxindex)
{
    if (ItemBoxClasses[itemboxindex] != nullptr) {
        ItemBoxClasses[itemboxindex] = nullptr;
    }
}

// 플레이어가 아이템을 떨굴때 (월드에 아이템 박스 생성, 패킷 전송)
void AOneGameModeBase::SpawnOnGroundItem(FName itemname, EItemClass itemclass, UTexture2D* texture, int count, float durability, float durability_max, int original_itembox_id)
{
    ABaseCharacter* DefaultPawn = nullptr;

    UWorld* World = GetWorld();
    if (World) {
        for (TActorIterator<ABaseCharacter> ActorItr(World); ActorItr; ++ActorItr) {
            DefaultPawn = *ActorItr;
            if (DefaultPawn) {
                //UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem -> GetPlayerId: %d"), DefaultPawn->GetPlayerId());
                if (DefaultPawn->GetPlayerId() == 99) {
                    UE_LOG(LogTemp, Log, TEXT("SpawnOnGroundItem : 99 - Myself"));
                    break;
                }
            }
        }
    }

    // UE_LOG(LogTemp, Warning, TEXT("DropPosBefore!!!!!!"));
    FVector DropPos = DefaultPawn->GetActorForwardVector() * 100.f;
    // UE_LOG(LogTemp, Warning, TEXT("ItemBoxClassesBefore!!!!!!"));
    //ItemBoxClasses.Add(AItemBoxActor::StaticClass()); // spawn시 .add하지 말고 비어있는 인덱스에다가 아이템 다시 넣어주기

    int32 dropItemBoxIndex = INDEX_NONE;
    bool bAdded = false;
    for (int32 i = 0; i < ItemBoxClasses.Num(); ++i)
    {
        if (i == original_itembox_id - 1 && ItemBoxClasses[i] == nullptr)    // 원래 아이템박스 인덱스에 알맞게 다시 들어감
        {
            ItemBoxClasses[i] = AItemBoxActor::StaticClass();
            dropItemBoxIndex = i;
            bAdded = true;
            break;
        }
    }

    if (!bAdded)
    {
        // 빈 자리가 없으면 새로 추가 => 회사원 서류가방에만 해당해야함!
        dropItemBoxIndex = ItemBoxClasses.Add(AItemBoxActor::StaticClass());
        original_itembox_id = dropItemBoxIndex + 1; // 61이 들어갈 꺼임
    }

    //UE_LOG(LogTemp, Warning, TEXT("SelectedItemBoxClassBefore!"));
    //UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem -> ItemBoxClasses.Num(): %d"), ItemBoxClasses.Num());
    // UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem ->  GetItemBoxNumber(): %d"), GetItemBoxNumber());

    if (dropItemBoxIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnOnGroundItem IndexERROR"));
        return;
    }

    // 실제로 아이템 월드에 생성 
    TSubclassOf<AItemBoxActor> SelectedItemBoxClass;
    if (ItemBoxClasses.IsValidIndex(dropItemBoxIndex)) {    // 접근 가능한 인덱스인지 먼저 확인
        SelectedItemBoxClass = ItemBoxClasses[dropItemBoxIndex];
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("ItemBoxClasses.IsValidIndex(dropItemBoxIndex) == false!!!!!! -> dropItemBoxIndex: %d"), dropItemBoxIndex);
    }

    FVector itemboxpos = DefaultPawn->GetActorLocation() + FVector(DropPos.X, DropPos.Y, -60.149886f);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnedItemBoxBefore!"));
    AItemBoxActor* SpawnedItemBox = GetWorld()->SpawnActor<AItemBoxActor>(SelectedItemBoxClass, itemboxpos, FRotator::ZeroRotator);

    if (SpawnedItemBox) {
        SpawnedItemBox->ItemName = itemname;
        SpawnedItemBox->ItemClassType = itemclass;
        SpawnedItemBox->Texture = texture;
        SpawnedItemBox->Count = count;
        SpawnedItemBox->Durability = durability;
        SpawnedItemBox->Durability_Max = durability_max;
        SpawnedItemBox->ItemBoxId = original_itembox_id;
    }

    UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem DropItemBoxIndex: %d, original_itembox_id: %d"), dropItemBoxIndex, original_itembox_id);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItemEND!"));

    uint32 iclass{};

    if (itemclass == EItemClass::NORMALWEAPON) {
        iclass = 1;
    }
    else if (itemclass == EItemClass::THROWINGWEAPON) {
        iclass = 2;
    }
    else if (itemclass == EItemClass::HEALINGITEM) {
        iclass = 3;
    }
    else if (itemclass == EItemClass::BLEEDINGHEALINGITEM) {
        iclass = 4;
    }
    else if (itemclass == EItemClass::KEYITEM) {
        iclass = 5;
    }
    else if (itemclass == EItemClass::BAGITEM) {
        iclass = 6;
    }
    else if (itemclass == EItemClass::NONE) {
        iclass = 7;
    }

    // 전송 작업
    Protocol::drop_item droppacket;

    droppacket.set_packet_type(22);
    std::string ItemNameStr(TCHAR_TO_UTF8(*itemname.ToString()));
    droppacket.set_itemname(ItemNameStr);
    droppacket.set_itemclass(iclass);
    droppacket.set_count(count);
    if (texture) {
        FString TexturePath = texture->GetPathName();
        std::string TexturePathStr(TCHAR_TO_UTF8(*TexturePath));
        droppacket.set_texture_path(TexturePathStr);
    }
    droppacket.set_itemid(original_itembox_id);
    droppacket.set_posx(itemboxpos.X);
    droppacket.set_posy(itemboxpos.Y);
    droppacket.set_posz(itemboxpos.Z);
    droppacket.set_durability(durability);
    droppacket.set_durability_max(durability_max);

    std::string serializedData;
    droppacket.SerializeToString(&serializedData);

    // 직렬화된 데이터를 서버로 전송
    bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());

}

// 플레이어가 죽을때 바닥에 아이템을 모두 떨구는 것 (월드에 아이템 박스 생성, 패킷 전송)
void AOneGameModeBase::SpawnOnDeathGroundItem(FName itemname, EItemClass itemclass, UTexture2D* texture, int count, float durability, float durability_max, FVector playerlocation, int original_itembox_id)
{
    FVector DropPos = playerlocation;

    float RandomX = FMath::RandRange(-200.0f, 200.0f);
    float RandomY = FMath::RandRange(-200.0f, 200.0f);

    DropPos.X += RandomX;
    DropPos.Y += RandomY;

    int32 dropItemBoxIndex = INDEX_NONE;
    bool bAdded = false;
    for (int32 i = 0; i < ItemBoxClasses.Num(); ++i)    // 원래 아이템박스 인덱스에 알맞게 다시 들어감
    {
        if (i == original_itembox_id - 1 && ItemBoxClasses[i] == nullptr)
        {
            ItemBoxClasses[i] = AItemBoxActor::StaticClass(); 
            dropItemBoxIndex = i;
            bAdded = true;
            break;
        }
    }
    
    if (!bAdded)
    {
        // 빈 자리가 없으면 새로 추가 => 회사원 서류가방에만 해당해야함!
        dropItemBoxIndex = ItemBoxClasses.Add(AItemBoxActor::StaticClass());
        original_itembox_id = dropItemBoxIndex + 1; // 61이 들어갈 꺼임
    }

    //UE_LOG(LogTemp, Warning, TEXT("SelectedItemBoxClassBefore!"));
    //UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem -> ItemBoxClasses.Num(): %d"), ItemBoxClasses.Num());
    // UE_LOG(LogTemp, Warning, TEXT("SpawnOnGroundItem ->  GetItemBoxNumber(): %d"), GetItemBoxNumber());

    if (dropItemBoxIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnOnDeathGroundItem IndexERROR"));
        return;
    }

    // 실제로 아이템 월드에 생성
    TSubclassOf<AItemBoxActor> SelectedItemBoxClass;
    if (ItemBoxClasses.IsValidIndex(dropItemBoxIndex)) {    // 접근 가능한 인덱스인지 먼저 확인
        SelectedItemBoxClass = ItemBoxClasses[dropItemBoxIndex];
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("ItemBoxClasses.IsValidIndex(dropItemBoxIndex) == false!!!!!! -> dropItemBoxIndex: %d"), dropItemBoxIndex);
    }

    FVector itemboxpos = DropPos + FVector(0.f, 0.f, -60.149886f);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnedItemBoxBefore!"));
    AItemBoxActor* SpawnedItemBox = GetWorld()->SpawnActor<AItemBoxActor>(SelectedItemBoxClass, itemboxpos, FRotator::ZeroRotator);

    if (SpawnedItemBox) {
        SpawnedItemBox->ItemName = itemname;
        SpawnedItemBox->ItemClassType = itemclass;
        SpawnedItemBox->Texture = texture;
        SpawnedItemBox->Count = count;
        SpawnedItemBox->Durability = durability;
        SpawnedItemBox->Durability_Max = durability_max;
        SpawnedItemBox->ItemBoxId = original_itembox_id;
    }

    UE_LOG(LogTemp, Warning, TEXT("SpawnOnDeathGroundItem ItemId: %d"), original_itembox_id);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnOnDeathGroundItemEND!"));

    uint32 iclass{};

    if (itemclass == EItemClass::NORMALWEAPON) {
        iclass = 1;
    }
    else if (itemclass == EItemClass::THROWINGWEAPON) {
        iclass = 2;
    }
    else if (itemclass == EItemClass::HEALINGITEM) {
        iclass = 3;
    }
    else if (itemclass == EItemClass::BLEEDINGHEALINGITEM) {
        iclass = 4;
    }
    else if (itemclass == EItemClass::KEYITEM) {
        iclass = 5;
    }
    else if (itemclass == EItemClass::BAGITEM) {
        iclass = 6;
    }
    else if (itemclass == EItemClass::NONE) {
        iclass = 7;
    }

    // 전송 작업
    Protocol::drop_item droppacket;

    droppacket.set_packet_type(22);
    std::string ItemNameStr(TCHAR_TO_UTF8(*itemname.ToString()));
    droppacket.set_itemname(ItemNameStr);
    droppacket.set_itemclass(iclass);
    droppacket.set_count(count);
    if (texture) {
        FString TexturePath = texture->GetPathName();
        std::string TexturePathStr(TCHAR_TO_UTF8(*TexturePath));
        droppacket.set_texture_path(TexturePathStr);
    }
    droppacket.set_itemid(original_itembox_id);
    droppacket.set_posx(itemboxpos.X);
    droppacket.set_posy(itemboxpos.Y);
    droppacket.set_posz(itemboxpos.Z);
    droppacket.set_durability(durability);
    droppacket.set_durability_max(durability_max);

    std::string serializedData;
    droppacket.SerializeToString(&serializedData);

    // 직렬화된 데이터를 서버로 전송
    bool bIsSent = GameInstance->ClientSocketPtr->Send(serializedData.size(), (void*)serializedData.data());
}

// 다른 플레이어가 아이템 떨군거 동기화
void AOneGameModeBase::SpawnOtherCharGroundItemBoxes(int32 original_itembox_id, FName itemname, uint32 itemclass, UTexture2D* texture, int count, FVector itempos, float durability, float durability_max)
{
    int32 dropItemBoxIndex = original_itembox_id - 1;

    if (dropItemBoxIndex >= ItemBoxClasses.Num()) {
        // 빈 자리가 없으면 새로 추가 => 회사원 서류가방에만 해당해야함!
        ItemBoxClasses.SetNum(dropItemBoxIndex + 1);    // 61이 들어갈 꺼임 

        if (ItemBoxClasses.IsValidIndex(dropItemBoxIndex)) {
            if (!ItemBoxClasses[dropItemBoxIndex]) {
                ItemBoxClasses[dropItemBoxIndex] = AItemBoxActor::StaticClass();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("SpawnOtherCharGroundItemBoxes -> itemid >= ItemBoxClasses.Num() !!!!! => Employee SuitCase Drop!!"));
    }
    else if (ItemBoxClasses[dropItemBoxIndex] != nullptr) {
        ItemBoxClasses[dropItemBoxIndex] = nullptr;
        UE_LOG(LogTemp, Error, TEXT("SpawnOtherCharGroundItemBoxes -> ItemBoxClasses[itemid] != nullptr !!!!! => 아이템 동기화 오류 발생했음!"));
    }

    TSubclassOf<AItemBoxActor> SelectedItemBoxClass = ItemBoxClasses[dropItemBoxIndex];
    if (SelectedItemBoxClass == nullptr) {
        SelectedItemBoxClass = AItemBoxActor::StaticClass();
    }

    AItemBoxActor* SpawnedItemBox = GetWorld()->SpawnActor<AItemBoxActor>(SelectedItemBoxClass, itempos, FRotator::ZeroRotator);

    EItemClass itemcl = EItemClass::NORMALWEAPON;
    if (itemclass == 1) {
        itemcl = EItemClass::NORMALWEAPON;
    }
    else if (itemclass == 2) {
        itemcl = EItemClass::THROWINGWEAPON;
    }
    else if (itemclass == 3) {
        itemcl = EItemClass::HEALINGITEM;
    }
    else if (itemclass == 4) {
        itemcl = EItemClass::BLEEDINGHEALINGITEM;
    }
    else if (itemclass == 5) {
        itemcl = EItemClass::KEYITEM;
    }
    else if (itemclass == 6) {
        itemcl = EItemClass::BAGITEM;
    }
    else if (itemclass == 7) {
        itemcl = EItemClass::NONE;
    }

    if (SpawnedItemBox) {
        SpawnedItemBox->ItemName = itemname;
        SpawnedItemBox->ItemClassType = itemcl;
        SpawnedItemBox->Texture = texture;
        SpawnedItemBox->Count = count;
        SpawnedItemBox->Durability = durability;
        SpawnedItemBox->Durability_Max = durability_max;
        SpawnedItemBox->ItemBoxId = original_itembox_id;
    }

    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes    Item Details:"));
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  ItemBoxId: %d"), original_itembox_id);
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  ItemName: %s"), *itemname.ToString());
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  ItemClass: %d"), itemclass);
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  Count: %d"), count);
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  Durability: %f"), durability);
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  Durability_Max: %f"), durability_max);
    UE_LOG(LogTemp, Log, TEXT("SpawnOtherCharGroundItemBoxes  ItemPos: X=%.2f, Y=%.2f, Z=%.2f"), itempos.X, itempos.Y, itempos.Z);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnOtherCharGroundItemBoxes END !!!!!"));
}


void AOneGameModeBase::CarActorRandomLocationSetting()
{
    CarActorRandomLocationStruct[0].sLocation = FVector(1772.f, 978.f, 60.f);
    CarActorRandomLocationStruct[1].sLocation = FVector(1662.f, 1678.f, 60.f);
    CarActorRandomLocationStruct[2].sLocation = FVector(722.f, 978.f, 60.f);
    CarActorRandomLocationStruct[3].sLocation = FVector(1142.f, 2068.f, 60.f);
    CarActorRandomLocationStruct[4].sLocation = FVector(632.f, 2688.f, 60.f);
    CarActorRandomLocationStruct[5].sLocation = FVector(1492.f, 2680.f, 60.f);
    CarActorRandomLocationStruct[6].sLocation = FVector(1122.f, 3468.f, 60.f);

    CarActorRandomLocationStruct[0].sRotation = FRotator(0.f, -30.f, 0.f);
    CarActorRandomLocationStruct[1].sRotation = FRotator(0.f, 30.f, 0.f);
    CarActorRandomLocationStruct[2].sRotation = FRotator(0.f, -50.f, 0.f);
    CarActorRandomLocationStruct[3].sRotation = FRotator(0.f, 30.f, 0.f);
    CarActorRandomLocationStruct[4].sRotation = FRotator(0.f, -30.f, 0.f);
    CarActorRandomLocationStruct[5].sRotation = FRotator(0.f, -130.f, 0.f);
    CarActorRandomLocationStruct[6].sRotation = FRotator(0.f, -70.f, 0.f);

    for (int i = 0; i < 7; ++i) {
        CarActorRandomLocationStruct[i].bIsSeatLocation = false;
    }

}

void AOneGameModeBase::CarKeyRandomSetting()
{
    CarKeyRandom[0].CarKeyName = "CarKey1"; // 지상 1층 파란색 키
    CarKeyRandom[1].CarKeyName = "CarKey2"; // 지상 1층 파란색 키
    CarKeyRandom[2].CarKeyName = "CarKey3"; // 지상 2층 빨간색 키 1
    CarKeyRandom[3].CarKeyName = "CarKey4"; // 지상 2층 빨간색 키 2
    CarKeyRandom[4].CarKeyName = "None";    // 꽝 - 맞는 열쇠가 없는 차량
    CarKeyRandom[5].CarKeyName = "None";    // 꽝 - 맞는 열쇠가 없는 차량
    CarKeyRandom[6].CarKeyName = "None";    // 꽝 - 맞는 열쇠가 없는 차량

    for (int i = 0; i < 7; ++i) {
        CarKeyRandom[i].bIsSeatCarKey = false;
    }
}

void AOneGameModeBase::SpawnInterItem(int32 InterActorindex, FName InterName, FVector carpos, FRotator carrotator, FName carkeyname)
{
    int32 interAindex = InterActorindex - 1;
    //UE_LOG(LogTemp, Log, TEXT("SpawnInterItem -> InterActorindex: %d, SpawnInterName: %s"), interAindex, *InterName.ToString());
    if (interAindex >= 8) {
        return;
    }
    TSubclassOf<AInterActor> SelectedInterActorClass = InterActorClasses[interAindex];


    FVector Location;
    FRotator Rotation;
    if (InterName == "CarActor") {
        Location = carpos;
        Rotation = carrotator;

        FName CarKey = carkeyname;

        ACarActor* SpawnedCarActor = GetWorld()->SpawnActor<ACarActor>(SelectedInterActorClass, Location, Rotation);
        SpawnedCarActor->CarKeyName = CarKey;
        SpawnedCarActor->CarID = InterActorindex;
        
        if (carkeyname == "CarKey3" || carkeyname == "CarKey4") { // 지상 2층 빨간키에 맞는 차량들
            //GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("CarID is #%d! Have to change color to RED!!!"), InterActorindex));
            UE_LOG(LogTemp, Log, TEXT("CarKey is \'%s\'! Have to change color to RED!!!"), *carkeyname.ToString());

            SpawnedCarActor->ChangeColorToRed();
        }
    }
    else if (InterName == "RoofTopDoorActor") {

        ARoofTopDoorActor* SpawnedRoofTopDoor = GetWorld()->SpawnActor<ARoofTopDoorActor>(SelectedInterActorClass, FVector(2410.f, 200.f, 3940.f), FRotator::ZeroRotator);
    }


}

void AOneGameModeBase::TurnOnCarHeadlights(int32 carID)
{
    for (TActorIterator<ACarActor> It(GetWorld()); It; ++It)
    {
        ACarActor* CarActor = *It;
        if (CarActor && CarActor->CarID == carID)
        {
            CarActor->UnLock();
        }
    }
}

void AOneGameModeBase::SpawnInterActorStaticClasses()
{
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ACarActor::StaticClass());
    InterActorClasses.Add(ARoofTopDoorActor::StaticClass());

}

void AOneGameModeBase::UpdateOtherPlayer(uint32 PlayerID, FVector NewLocation, FRotator NewRotation, uint32 charactertype, std::string username, float hp, float DeltaSeconds)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateOtherPlayer: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            FVector OldLocation = BasePlayer->GetActorLocation();

            FVector MoveDir = (NewLocation - OldLocation);
            const float DistToDest = MoveDir.Length();
            MoveDir.Normalize();

            float MoveDist = 0.f;

            if (BasePlayer->IsRun() == true) {
                MoveDist = (MoveDir * (BasePlayer->GetBasicSpeed() * 100.f) * DeltaSeconds).Length();
            }
            else {
                MoveDist = (MoveDir * (BasePlayer->GetBasicSpeed() / 2 * 100.f) * DeltaSeconds).Length();
            }
            MoveDist = FMath::Min(MoveDist, DistToDest);
            FVector NextLocation = OldLocation + MoveDir * MoveDist;

            FRotator SmoothedRotation = FMath::RInterpTo(BasePlayer->GetActorRotation(), NewRotation, DeltaSeconds, 10.0f);

            //if (DistToDest > 200.f){
            //    // 너무 멀면 그냥 순간이동
            //    BasePlayer->TeleportTo(NewLocation, NewRotation);
            //}
            BasePlayer->SetActorLocation(NextLocation);
            
            BasePlayer->SetActorRotation(SmoothedRotation);
            BasePlayer->UpdatePlayerData(NextLocation);


            BasePlayer->SetHP(hp);

            return;
        }
    }

    // 캐릭터 타입에 따라 스폰할 클래스를 결정
    TSubclassOf<ABaseCharacter> CharacterClass = nullptr;

    if (charactertype == 2) {
        CharacterClass = AEmployeeCharacter::StaticClass();
        //OtherCharacterClasses.Add(AEmployeeCharacter::StaticClass());
    }
    else if (charactertype == 4) {
        CharacterClass = AFireFighterCharacter::StaticClass();
        //OtherCharacterClasses.Add(AFireFighterCharacter::StaticClass());
    }
    else if (charactertype == 1) {
        CharacterClass = AGirlCharacter::StaticClass();
        //OtherCharacterClasses.Add(AGirlCharacter::StaticClass());
    }
    else if (charactertype == 3) {
        CharacterClass = AIdolCharacter::StaticClass();
        //OtherCharacterClasses.Add(AIdolCharacter::StaticClass());
    }

    if (CharacterClass)
    {
        //기존 캐릭터를 찾지 못한 경우에만 새 캐릭터 스폰
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ABaseCharacter* NewCharacter = World->SpawnActor<ABaseCharacter>(CharacterClass, NewLocation, NewRotation, SpawnParams);

        if (NewCharacter)
        {
            // 새 캐릭터에 PlayerId 설정
            NewCharacter->SetPlayerId(PlayerID);
            NewCharacter->SetPlayerName(username);
            NewCharacter->SetHP(hp);  // 초기 HP 설정
            auto AnimInstance = Cast<UPlayerCharacterAnimInstance>(NewCharacter->GetMesh()->GetAnimInstance());
            AnimInstance->OwnerCharacter = NewCharacter;    // 새캐릭의 AnimInstance 연결 설정

            UE_LOG(LogTemp, Log, TEXT("[PlayerCharacterAnimInstance 다른 클라 재설정 완료] OwnerCharacter 설정: %d"), AnimInstance->OwnerCharacter->GetPlayerId());
        }
    }
}

void AOneGameModeBase::UpdatePlayerAttack(uint32 PlayerID, bool battack, uint32 attacktype, float aimoffset)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePlayerAttack: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            BasePlayer->SetAttack(battack, attacktype, aimoffset);      
            
            //UE_LOG(LogTemp, Warning, TEXT("real update attack: %d, %s"), PlayerID, battack ? TEXT("true") : TEXT("false"));

            return;
        }
    }
}

void AOneGameModeBase::UpdateEquipItem(uint32 PlayerID, const FString& Itemname, uint32 itemtype)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateEquipItem: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            if (Itemname != "") {
                if (itemtype == 1) {
                    BasePlayer->OtherSpawnBleedingHealingItem(Itemname);
                }
                else if (itemtype == 2) {
                    BasePlayer->OtherSpawnHealingItem(Itemname);
                }
                else if (itemtype == 3) {
                    BasePlayer->OtherSpawnThrowWeapon(Itemname);
                }
                else if (itemtype == 4) {
                    BasePlayer->OtherSpawnKeyItem(Itemname);
                }
                else if (itemtype == 5) {
                    BasePlayer->OtherSpawnNormalWeapon(Itemname);
                    BasePlayer->SetNWHandIn(true);
                }
                UE_LOG(LogTemp, Warning, TEXT("real update equip: %d"), PlayerID);
            }
            return;
        }
    }
}

void AOneGameModeBase::UpdateUnEquipItem(uint32 PlayerID, uint32 itemtype)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateUnEquipItem: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            BasePlayer->OtherUnEquipItem(itemtype);
            UE_LOG(LogTemp, Warning, TEXT("real update equip: %d"), PlayerID);
            return;
        }
    }
}

void AOneGameModeBase::UpdatePlayerRun(uint32 PlayerID, bool b_run)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePlayerRun: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            BasePlayer->Other_Run(b_run);
            //UE_LOG(LogTemp, Warning, TEXT("real update run: %d, %d"), PlayerID, b_run);

            return;
        }
    }
}

void AOneGameModeBase::UpdatePlayerJump(uint32 PlayerID)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePlayerJump: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색
    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        ABaseCharacter* BasePlayer = *It;
        if (BasePlayer && BasePlayer->GetPlayerId() == PlayerID)
        {
            BasePlayer->SetPlayerJump();
            UE_LOG(LogTemp, Display, TEXT("real update jump: %d"), PlayerID);

            return;
        }
    }
}


void AOneGameModeBase::UpdateZombie(uint32 ZombieID, uint32 ZombieType, FVector NewLocation, FRotator NewRotation)
{
    UWorld* World = GetWorld();

    //UE_LOG(LogTemp, Warning, TEXT("Call zombie ID: %d"), ZombieID);

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateZombie: GetWorld() returned nullptr"));
        return;
    }

    TSubclassOf<ABaseZombie> ZombieClass = nullptr;
    TSubclassOf<AAIController> ZombieAIClass = nullptr;

    if (ZombieType == 0) {
        ZombieClass = ANormalZombie::StaticClass();
        ZombieAIClass = AZombieAIController::StaticClass();
    }
    else if (ZombieType == 1) {
        ZombieClass = AShoutingZombie::StaticClass();
        ZombieAIClass = AShoutingZombieAIController::StaticClass();
    }
    else if (ZombieType == 2) {
        ZombieClass = ARunningZombie::StaticClass();
        ZombieAIClass = ARunningZombieAIController::StaticClass();
    }

    // 0번 좀비가 제대로 함수 호출을 하긴 하는지 확인
    //UE_LOG(LogTemp, Warning, TEXT("Updated Zombie ID: %d at new location"), ZombieID);

    ABaseZombie** ZombiePtr = ZombieMap.Find(ZombieID);
    if (ZombiePtr == nullptr)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Could not find Zombie ID: %d"), ZombieID);
    }

    // 기존 좀비 존재함 => 업데이트 ===> (근데 사실상 이제는 사용 안함 -> PlayerCharacterController Tick 맨 아래에서 SetActorLocation을 직접 설정함)
    if (ZombiePtr && *ZombiePtr && IsValid(*ZombiePtr))
    {
        ABaseZombie* BaseZombie = *ZombiePtr;
        if (BaseZombie)
        {

            // 기존 좀비 위치 업데이트
            BaseZombie->SetActorLocation(NewLocation);
            BaseZombie->SetActorRotation(NewRotation);

            //좀비 애니메이션 업데이트용
            BaseZombie->UpdateZombieData(NewLocation);


            //UE_LOG(LogTemp, Warning, TEXT("Updated Zombie ID: %d at new location"), ZombieID);
        }
    }
    // 기존 좀비가 없으면 새로운 좀비 생성
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ABaseZombie* NewZombie = World->SpawnActor<ABaseZombie>(ZombieClass, NewLocation, NewRotation, SpawnParams);

        if (NewZombie)
        {
            // 새 좀비에 ZombieID 설정 및 ZombieMap에 추가
            NewZombie->SetZombieId(ZombieID);
            ZombieMap.Add(ZombieID, NewZombie);

            // 새 좀비 floor 설정
            if (NewLocation.Z < 800.f) {
                NewZombie->floor = ABaseZombie::FLOOR::B2;
            }
            else if (NewLocation.Z < 1800.f) {
                NewZombie->floor = ABaseZombie::FLOOR::B1;
            }
            else if (NewLocation.Z < 2500.f) {
                NewZombie->floor = ABaseZombie::FLOOR::F1;
            }
            else if (NewLocation.Z < 3600.f) {
                NewZombie->floor = ABaseZombie::FLOOR::F2;
            }

           //UE_LOG(LogTemp, Warning, TEXT("Spawned new Zombie ID: %d"), ZombieID);

            if (ZombieType == 0) {
                AZombieAIController* AIZombieController = World->SpawnActor<AZombieAIController>(ZombieAIClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
                if (AIZombieController)
                {
                    AIZombieController->Possess(NewZombie);
                    AIZombieController->OwnerZombie = Cast<ANormalZombie>(NewZombie);

                    // 나 자신 ABaseCharacter 데이터 미리 찾아서 저장해놓기
                    TArray<AActor*> Players;
                    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);
                    ABaseCharacter* Char = nullptr;
                    for (AActor* Player : Players)
                    {
                        Char = Cast<ABaseCharacter>(Player);

                        if (Char->GetPlayerId() == 99) {
                            //AIZombieController->OwnerZombie->MyChar = Char; //결국 아래랑 같음
                            NewZombie->MyChar = Char;
                        }
                    }
                    

                    //UE_LOG(LogTemp, Warning, TEXT("Spawned and possessed new Normal Zombie ID: %d"), ZombieID);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to spawn AI Controller for Normal Zombie ID: %d"), ZombieID);
                }
            }
            else if (ZombieType == 1) {
                AShoutingZombieAIController* AIShoutingZombieController = World->SpawnActor<AShoutingZombieAIController>(ZombieAIClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
                if (AIShoutingZombieController)
                {
                    AIShoutingZombieController->Possess(NewZombie);
                    AIShoutingZombieController->OwnerZombie = Cast<AShoutingZombie>(NewZombie);

                    // 나 자신 ABaseCharacter 데이터 미리 찾아서 저장해놓기
                    TArray<AActor*> Players;
                    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);
                    ABaseCharacter* Char = nullptr;
                    for (AActor* Player : Players)
                    {
                        Char = Cast<ABaseCharacter>(Player);

                        if (Char->GetPlayerId() == 99) {
                            NewZombie->MyChar = Char;
                        }
                    }


                    //UE_LOG(LogTemp, Warning, TEXT("Spawned and possessed new Shouting Zombie ID: %d"), ZombieID);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to spawn AI Controller for Shouting Zombie ID: %d"), ZombieID);
                }
            }
            else if (ZombieType == 2) {
                ARunningZombieAIController* AIRunningZombieController = World->SpawnActor<ARunningZombieAIController>(ZombieAIClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
                if (AIRunningZombieController)
                {
                    AIRunningZombieController->Possess(NewZombie);
                    AIRunningZombieController->OwnerZombie = Cast<ARunningZombie>(NewZombie);

                    // 나 자신 ABaseCharacter 데이터 미리 찾아서 저장해놓기
                    TArray<AActor*> Players;
                    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);
                    ABaseCharacter* Char = nullptr;
                    for (AActor* Player : Players)
                    {
                        Char = Cast<ABaseCharacter>(Player);

                        if (Char->GetPlayerId() == 99) {
                            NewZombie->MyChar = Char;
                        }
                    }


                    //UE_LOG(LogTemp, Warning, TEXT("Spawned and possessed new Running Zombie ID: %d"), ZombieID);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to spawn AI Controller for Running Zombie ID: %d"), ZombieID);
                }
            }


        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn new Zombie ID: %d"), ZombieID);
        }
        //UE_LOG(LogTemp, Warning, TEXT("No zombie found with ID: %d"), ZombieID);
    }
}


void AOneGameModeBase::UpdateZombieAttack(uint32 ZombieId, uint32 PlayerId)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateZombieAttack: GetWorld() returned nullptr"));
        return;
    }

    // ZombieMap에서 ZombieID로 좀비를 찾음
    ABaseZombie** ZombiePtr = ZombieMap.Find(ZombieId);
    if (ZombiePtr && *ZombiePtr)
    {
        ABaseZombie* BaseZombie = *ZombiePtr;
        if (BaseZombie)
        {
            // 좀비의 공격 상태 업데이트
            BaseZombie->Attack(PlayerId);
            UE_LOG(LogTemp, Warning, TEXT("Updated Zombie ID: %d attack"), ZombieId);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No zombie found with ID: %d"), ZombieId);
    }
}

void AOneGameModeBase::UpdateZombieHP(uint32 ZombieId, float Damage)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateZombieHP: GetWorld() returned nullptr"));
        return;
    }

    // ZombieMap에서 ZombieID로 좀비를 찾음
    ABaseZombie** ZombiePtr = ZombieMap.Find(ZombieId);
    if (ZombiePtr && *ZombiePtr)
    {
        ABaseZombie* zombie = *ZombiePtr;
        if (zombie)
        {
            float PrevHP = zombie->GetHP();
            float NewHP = zombie->GetHP() - Damage;
            if (NewHP <= 0.f)   // 체력이 음수가 되지는 않도록
                NewHP = 0.f;

            if (PrevHP == 0) {  // 데이터 레이스 방지 => 좀비 도망치기 중 사망했을때 체력 0이 되고 체력이 회복되어 부활이 정상적으로 안 이루어 지는 버그 방지
                if (NewHP == zombie->GetStartHP()) {
                    // 좀비 부활시키기
                    zombie->Resurrect();
                }
            }
            else {
                //좀비의 체력상태 업데이트
                zombie->SetHP(NewHP);

                if (Damage >= 0) {  // 체력을 깎을 때만
                    zombie->doAction_takeDamage_onTick = true;
                    if (NewHP <= 0) {
                        zombie->doAction_setIsNormalDead_onTick = true;
                    }
                }
            }

            UE_LOG(LogTemp, Log, TEXT("[UpdateZombieHP] Updated Zombie ID: %d, HP: %f -> %f"), ZombieId, PrevHP, NewHP);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UpdateZombieHP] No zombie found with ID: %d"), ZombieId);
    }
}

void AOneGameModeBase::UpdateCuttingZombie(uint32 ZombieId, FVector zombieLocation, FVector planePosition, FVector planeNormal, FVector impulseDirection)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateCuttingZombie: GetWorld() returned nullptr"));
        return;
    }

    // ZombieMap에서 ZombieID로 좀비를 찾음
    ABaseZombie** ZombiePtr = ZombieMap.Find(ZombieId);
    if (ZombiePtr && *ZombiePtr)
    {
        ABaseZombie* BaseZombie = *ZombiePtr;
        if (BaseZombie)
        {
            BaseZombie->doAction_setIsCuttingDead_onTick = true;
            BaseZombie->SetHP(0);
            BaseZombie->SetActorLocation(zombieLocation);
            BaseZombie->sync_cutPlane = planePosition;
            BaseZombie->sync_cutNormal = planeNormal;
            BaseZombie->sync_cutImpulse = impulseDirection;

            UE_LOG(LogTemp, Log, TEXT("zombieLocation.x : %f, zombieLocation.y : %f, zombieLocation.z : %f"), zombieLocation.X, zombieLocation.Y, zombieLocation.Z);
            UE_LOG(LogTemp, Log, TEXT("planePosition.x : %f, planePosition.y : %f, planePosition.z : %f"), planePosition.X, planePosition.Y, planePosition.Z);
            UE_LOG(LogTemp, Log, TEXT("planeNormal.x : %f, planeNormal.y : %f, planeNormal.z : %f"), planeNormal.X, planeNormal.Y, planeNormal.Z);
            UE_LOG(LogTemp, Log, TEXT("impulseDirection.x : %f, impulseDirection.y : %f, impulseDirection.z : %f"), impulseDirection.X, impulseDirection.Y, impulseDirection.Z);

        }
    }
}

void AOneGameModeBase::UpdateShoutingZombie(uint32 ZombieId, uint32 PlayerId)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateShoutingZombie: GetWorld() returned nullptr"));
        return;
    }

    // ZombieMap에서 ZombieID로 해당 샤우팅 좀비를 찾음
    ABaseZombie** ZombiePtr = ZombieMap.Find(ZombieId);
    if (ZombiePtr && *ZombiePtr)
    {
        ABaseZombie* BaseZombie = *ZombiePtr;
        if (BaseZombie)
        {
            BaseZombie->Shouting();
            
            // 해당 좀비에 연결된 AIController 찾기
            AShoutingZombieAIController* AIShoutingZombieController = Cast<AShoutingZombieAIController>(BaseZombie->GetController());
            if (AIShoutingZombieController)
            {
                UE_LOG(LogTemp, Log, TEXT("Found AI Controller: %s"), *AIShoutingZombieController->GetName());

                AIShoutingZombieController->shoutingTo_PlayerId = PlayerId;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Error] No AI Controller is possessing this zombie!"));
                return;
            }

            UE_LOG(LogTemp, Log, TEXT("UpdateShoutingZombie - ZombieId: %d, PlayerId: %d"), ZombieId, PlayerId);

        }
    }
}

// 아이템 획득시 처리 함수
void AOneGameModeBase::DestroyItem(uint32 Itemid, uint32 Playerid)
{
    UWorld* World = GetWorld();
    UE_LOG(LogTemp, Log, TEXT("DestroyItem"));
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("DestroyItem: GetWorld() returned nullptr"));
        return;
    }

    // 캐릭터 검색 및 줍기 애니메이션 동기화
    ABaseCharacter* BasePlayer = nullptr;

    for (TActorIterator<ABaseCharacter> It(World); It; ++It)
    {
        BasePlayer = *It;
        if (BasePlayer != nullptr)
        {
            if (BasePlayer->GetPlayerId() == Playerid || (BasePlayer->GetPlayerId() == 99 && GameInstance->ClientSocketPtr->GetMyPlayerId() == Playerid))
            {
                //UE_LOG(LogTemp, Display, TEXT("BasePlayer->GetPlayerId(): %d"), BasePlayer->GetPlayerId());
                //UE_LOG(LogTemp, Display, TEXT("GameInstance->ClientSocketPtr->GetMyPlayerId(): %d"), GameInstance->ClientSocketPtr->GetMyPlayerId());

                BasePlayer->SetPickUp();
                UE_LOG(LogTemp, Display, TEXT("Pickup updated for PlayerId: %d"), Playerid);
                break;
            }
        }
    }

    for (TActorIterator<AItemBoxActor> It(World); It; ++It)
    {
        if (AItemBoxActor* ItemBox = *It)
        {
            if (ItemBox->ItemBoxId == Itemid)
            {
                if (BasePlayer->GetPlayerId() == 99 && GameInstance->ClientSocketPtr->GetMyPlayerId() == Playerid) {    // 로컬 플레이어가 획득한 경우에는
                    // 플레이어 아이템 실제 장착 => UI 업데이트 및 인벤토리에 추가
                    if (BasePlayer != nullptr) {
                        BasePlayer->PickUpItem(ItemBox);
                    }
                }

                // 아이템 상자 제거
                ItemBox->Destroy();
                NullPtrItemBoxesIndex(ItemBox->ItemBoxId - 1);  // 인덱스 번호를 넘겨줘야 해서 ItemBoxId - 1
                UE_LOG(LogTemp, Warning, TEXT("Item destroyed: ItemId=%d"), Itemid);
                break;
            }
        }
    }
}


void AOneGameModeBase::BeginDestroy()
{
    Super::BeginDestroy();

    // 게임 종료 시 모든 좀비 객체와 맵 초기화
    for (auto& Elem : ZombieMap)
    {
        if (Elem.Value)
        {
            Elem.Value->Destroy();
        }
    }
    ZombieMap.Empty();
}