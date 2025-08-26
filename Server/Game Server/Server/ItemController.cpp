#include "ItemController.h"

random_device rd;
uniform_int_distribution Cuid(0, 6);

ItemController::ItemController(IOCP_CORE* mainServer, int roomid)
{
	iocpServer = mainServer;
    item_roomid = roomid;

    ItemRandomLocationSetting();
    CarActorRandomLocationSetting();
    CarKeyRandomSetting();

    // B1
    SpawnItemBoxes(1, "SquareWood", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSquareWood.InvenSquareWood", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(2, "SquareWood", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSquareWood.InvenSquareWood", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(3, "SashimiKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSasimiKnife.InvenSasimiKnife", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(4, "ButchersKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenButherKnife.InvenButherKnife", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(5, "SashimiKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSasimiKnife.InvenSasimiKnife", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(6, "ButchersKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenButherKnife.InvenButherKnife", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(7, "Bottle", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenBottle.InvenBottle", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(8, "DirtyClothes", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenDirtyCloth.InvenDirtyCloth", 3, FLOOR::FLOOR_B1);
    SpawnItemBoxes(9, "Water", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenWater.InvenWater", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(10, "Smoke", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSmoke.InvenSmoke", 2, FLOOR::FLOOR_B1);

    SpawnItemBoxes(11, "Drink", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenDrink.InvenDrink", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(12, "Snack", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSnack.InvenSnack", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(13, "CannedTuna", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenCannedTuna.InvenCannedTuna", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(14, "Water", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenWater.InvenWater", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(15, "Smoke", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSmoke.InvenSmoke", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(16, "Drink", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenDrink.InvenDrink", 2, FLOOR::FLOOR_B1);
    SpawnItemBoxes(17, "Bag", EItemClass::BAGITEM, "/Game/InvenPng/InvenBag.InvenBag", 1, FLOOR::FLOOR_B1); // 수정 필요
    SpawnItemBoxes(18, "BigBag", EItemClass::BAGITEM, "/Game/InvenPng/InvenBigBag.InvenBigBag", 1, FLOOR::FLOOR_B1);

    SpawnItemBoxes(19, "CarKey1", EItemClass::KEYITEM, "/Game/InvenPng/InvenCarKey1.InvenCarKey1", 1, FLOOR::FLOOR_B1);
    SpawnItemBoxes(20, "RoofKey1", EItemClass::KEYITEM, "/Game/InvenPng/InvenRoofKey1.InvenRoofKey1", 1, FLOOR::FLOOR_B1);

    // F1
    SpawnItemBoxes(21, "SquareWood", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSquareWood.InvenSquareWood", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(22, "SquareWood", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSquareWood.InvenSquareWood", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(23, "ButchersKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenButherKnife.InvenButherKnife", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(24, "FireAxe", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenFireAxe.InvenFireAxe", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(25, "FireAxe", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenFireAxe.InvenFireAxe", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(26, "Book", EItemClass::NORMALWEAPON, "/Game/InvenPng/Invenbook.Invenbook", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(27, "Book", EItemClass::NORMALWEAPON, "/Game/InvenPng/Invenbook.Invenbook", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(28, "SashimiKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSasimiKnife.InvenSasimiKnife", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(29, "MagicStick", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenMagicStick.InvenMagicStick", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(30, "DirtyClothes", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenDirtyCloth.InvenDirtyCloth", 3, FLOOR::FLOOR_F1);

    SpawnItemBoxes(31, "Bandage", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenBandage.InvenBandage", 3, FLOOR::FLOOR_F1);
    SpawnItemBoxes(32, "Clothes", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenCloth.InvenCloth", 3, FLOOR::FLOOR_F1);
    SpawnItemBoxes(33, "Gauze", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenGauze.InvenGauze", 3, FLOOR::FLOOR_F1);
    SpawnItemBoxes(34, "Water", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenWater.InvenWater", 2, FLOOR::FLOOR_F1);
    SpawnItemBoxes(35, "Smoke", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSmoke.InvenSmoke", 2, FLOOR::FLOOR_F1);
    SpawnItemBoxes(36, "Disinfectant", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenDisinfectant.InvenDisinfectant", 2, FLOOR::FLOOR_F1);
    SpawnItemBoxes(37, "Ointment", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenOintment.InvenOintment", 2, FLOOR::FLOOR_F1);
    SpawnItemBoxes(38, "BigBag", EItemClass::BAGITEM, "/Game/InvenPng/InvenBigBag.InvenBigBag", 1, FLOOR::FLOOR_F1);

    SpawnItemBoxes(39, "CarKey2", EItemClass::KEYITEM, "/Game/InvenPng/InvenCarKey1.InvenCarKey1", 1, FLOOR::FLOOR_F1);
    SpawnItemBoxes(40, "RoofKey2", EItemClass::KEYITEM, "/Game/InvenPng/InvenRoofKey2.InvenRoofKey2", 1, FLOOR::FLOOR_F1);

    // F2
    SpawnItemBoxes(41, "SashimiKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenSasimiKnife.InvenSasimiKnife", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(42, "ButchersKnife", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenButherKnife.InvenButherKnife", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(43, "WoodenBat", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenWoodenBat.InvenWoodenBat", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(44, "FireAxe", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenFireAxe.InvenFireAxe", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(45, "Plunger", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenPlunger.InvenPlunger", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(46, "FryingPan", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenFryingPan.InvenFryingPan", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(47, "Shovels", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenShovel.InvenShovel", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(48, "Shovels", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenShovel.InvenShovel", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(49, "GolfClub", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenGolfClub.InvenGolfClub", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(50, "WoodenBat", EItemClass::NORMALWEAPON, "/Game/InvenPng/InvenWoodenBat.InvenWoodenBat", 1, FLOOR::FLOOR_F2);

    SpawnItemBoxes(51, "DirtyClothes", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenDirtyCloth.InvenDirtyCloth", 3, FLOOR::FLOOR_F2);
    SpawnItemBoxes(52, "Clothes", EItemClass::BLEEDINGHEALINGITEM, "/Game/InvenPng/InvenCloth.InvenCloth", 3, FLOOR::FLOOR_F2);
    SpawnItemBoxes(53, "Water", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenWater.InvenWater", 2, FLOOR::FLOOR_F2);
    SpawnItemBoxes(54, "Smoke", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSmoke.InvenSmoke", 2, FLOOR::FLOOR_F2);
    SpawnItemBoxes(55, "Water", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenWater.InvenWater", 2, FLOOR::FLOOR_F2);
    SpawnItemBoxes(56, "Smoke", EItemClass::HEALINGITEM, "/Game/InvenPng/InvenSmoke.InvenSmoke", 2, FLOOR::FLOOR_F2);
    SpawnItemBoxes(57, "Bag", EItemClass::BAGITEM, "/Game/InvenPng/InvenBag.InvenBag", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(58, "BigBag", EItemClass::BAGITEM, "/Game/InvenPng/InvenBigBag.InvenBigBag", 1, FLOOR::FLOOR_F2);

    SpawnItemBoxes(59, "CarKey3", EItemClass::KEYITEM, "/Game/InvenPng/InvenCarKey2.InvenCarKey2", 1, FLOOR::FLOOR_F2);
    SpawnItemBoxes(60, "CarKey4", EItemClass::KEYITEM, "/Game/InvenPng/InvenCarKey2.InvenCarKey2", 1, FLOOR::FLOOR_F2);


    SpawnInterItem(1, "CarActor");
    SpawnInterItem(2, "CarActor");
    SpawnInterItem(3, "CarActor");
    SpawnInterItem(4, "CarActor");
    SpawnInterItem(5, "CarActor");
    SpawnInterItem(6, "CarActor");
    SpawnInterItem(7, "CarActor");
    SpawnInterItem(8, "RoofTopDoorActor");
}

ItemController::~ItemController()
{
}

void ItemController::ItemRandomLocationSetting()
{
    // 0~19은 B1, 20~39는 F1, 40~59는 F2
    // B1
    ItemRandomLocationStruct[0].sLocation = FVector(2330.f, 3200.f, 980.0626f);
    ItemRandomLocationStruct[1].sLocation = FVector(1880.f, 3870.f, 980.0626f);
    ItemRandomLocationStruct[2].sLocation = FVector(400.f, 3650.f, 980.0626f);

    ItemRandomLocationStruct[3].sLocation = FVector(1000.f, 2800.f, 980.0626f);
    ItemRandomLocationStruct[4].sLocation = FVector(2200.f, 3300.f, 980.0626f);
    ItemRandomLocationStruct[5].sLocation = FVector(1770.f, 2170.f, 980.0626f);

    ItemRandomLocationStruct[6].sLocation = FVector(440.f, 2310.f, 980.0626f);
    ItemRandomLocationStruct[7].sLocation = FVector(2300.f, 1800.f, 980.0626f);
    ItemRandomLocationStruct[8].sLocation = FVector(820.f, 1650.f, 980.0626f);

    ItemRandomLocationStruct[9].sLocation = FVector(120.f, 1200.f, 980.0626f);
    ItemRandomLocationStruct[10].sLocation = FVector(270.f, 2000.f, 980.0626f);
    ItemRandomLocationStruct[11].sLocation = FVector(1870.f, 800.f, 980.0626f);

    ItemRandomLocationStruct[12].sLocation = FVector(1200.f, 100.f, 980.0626f);
    ItemRandomLocationStruct[13].sLocation = FVector(30.f, 540.f, 980.0626f);
    ItemRandomLocationStruct[14].sLocation = FVector(500.f, -238.f, 980.0626f);

    ItemRandomLocationStruct[15].sLocation = FVector(1200.f, -976.f, 980.0626f);
    ItemRandomLocationStruct[16].sLocation = FVector(480.f, 800.f, 980.0626f);
    ItemRandomLocationStruct[17].sLocation = FVector(2330.f, 400.f, 980.0626f);

    ItemRandomLocationStruct[18].sLocation = FVector(730.f, 2130.f, 980.0626f);
    ItemRandomLocationStruct[19].sLocation = FVector(30.f, 3540.f, 980.0626f);


    // F1
    ItemRandomLocationStruct[20].sLocation = FVector(1200.f, 70.f, 1929.0626f);
    ItemRandomLocationStruct[21].sLocation = FVector(180.f, 520.f, 1929.0626f);
    ItemRandomLocationStruct[22].sLocation = FVector(1200.f, -817.f, 1929.0626f);

    ItemRandomLocationStruct[23].sLocation = FVector(800.f, -817.f, 1929.0626f);
    ItemRandomLocationStruct[24].sLocation = FVector(165.f, 960.f, 1929.0626f);
    ItemRandomLocationStruct[25].sLocation = FVector(1980.f, 1100.f, 1929.0626f);

    ItemRandomLocationStruct[26].sLocation = FVector(1390.f, 800.f, 1929.0626f);
    ItemRandomLocationStruct[27].sLocation = FVector(400.f, 1680.f, 1929.0626f);
    ItemRandomLocationStruct[28].sLocation = FVector(1100.f, 1260.f, 1929.0626f);

    ItemRandomLocationStruct[29].sLocation = FVector(70.f, 2390.f, 1929.0626f);
    ItemRandomLocationStruct[30].sLocation = FVector(550.f, 2520.f, 1929.0626f);
    ItemRandomLocationStruct[31].sLocation = FVector(200.f, 2800.f, 1929.0626f);

    ItemRandomLocationStruct[32].sLocation = FVector(1600.f, 3050.f, 1929.0626f);
    ItemRandomLocationStruct[33].sLocation = FVector(1530.f, 3900.f, 1929.0626f);
    ItemRandomLocationStruct[34].sLocation = FVector(2340.f, 3850.f, 1929.0626f);

    ItemRandomLocationStruct[35].sLocation = FVector(290.f, 3020.f, 1929.0626f);
    ItemRandomLocationStruct[36].sLocation = FVector(90.f, 3770.f, 1929.0626f);
    ItemRandomLocationStruct[37].sLocation = FVector(1050.f, 3350.f, 1929.0626f);

    ItemRandomLocationStruct[38].sLocation = FVector(50.f, 1740.f, 1929.0626f);
    ItemRandomLocationStruct[39].sLocation = FVector(2280.f, 2620.f, 1929.0626f);


    // F2                                                              
    ItemRandomLocationStruct[40].sLocation = FVector(870.f, -926.f, 2888.0626f);
    ItemRandomLocationStruct[41].sLocation = FVector(1260.f, -467.f, 2888.0626f);
    ItemRandomLocationStruct[42].sLocation = FVector(2170.f, 60.f, 2888.0626f);

    ItemRandomLocationStruct[43].sLocation = FVector(460.f, 800.f, 2888.0626f);
    ItemRandomLocationStruct[44].sLocation = FVector(1100.f, 600.f, 2888.0626f);
    ItemRandomLocationStruct[45].sLocation = FVector(1600.f, 1140.f, 2888.0626f);

    ItemRandomLocationStruct[46].sLocation = FVector(1200.f, 1300.f, 2888.0626f);
    ItemRandomLocationStruct[47].sLocation = FVector(1980.f, 1600.f, 2888.0626f);
    ItemRandomLocationStruct[48].sLocation = FVector(550.f, 1770.f, 2888.0626f);

    ItemRandomLocationStruct[49].sLocation = FVector(170.f, 1120.f, 2888.0626f);
    ItemRandomLocationStruct[50].sLocation = FVector(1240.f, 2270.f, 2888.0626f);
    ItemRandomLocationStruct[51].sLocation = FVector(1410.f, 2650.f, 2888.0626f);

    ItemRandomLocationStruct[52].sLocation = FVector(410.f, 3030.f, 2888.0626f);
    ItemRandomLocationStruct[53].sLocation = FVector(801.f, 3780.f, 2888.0626f);
    ItemRandomLocationStruct[54].sLocation = FVector(2001.f, 3470.f, 2888.0626f);

    ItemRandomLocationStruct[55].sLocation = FVector(1730.f, 1200.f, 2888.0626f);
    ItemRandomLocationStruct[56].sLocation = FVector(260.f, 3780.f, 2888.0626f);
    ItemRandomLocationStruct[57].sLocation = FVector(2310.f, 2040.f, 2888.0626f);

    ItemRandomLocationStruct[58].sLocation = FVector(869.f, 3120.f, 2888.0626f);
    ItemRandomLocationStruct[59].sLocation = FVector(1140.f, 1460.f, 2888.0626f);

    for (int i = 0; i < 60; ++i) {
        ItemRandomLocationStruct[i].bIsSeatLocation = false;
    }
}

FVector ItemController::RandomItemLocation(FLOOR itemfloor)
{
    int RandomNumber = 0;

    int MinIndex = 0;
    int MaxIndex = 0;


    if (itemfloor == FLOOR::FLOOR_B1) {
        MinIndex = 0;
        MaxIndex = 19;
    }
    else if (itemfloor == FLOOR::FLOOR_F1) {
        MinIndex = 20;
        MaxIndex = 39;
    }
    else if (itemfloor == FLOOR::FLOOR_F2) {
        MinIndex = 40;
        MaxIndex = 59;
    }

    uniform_int_distribution Iuid(MinIndex, MaxIndex);

    while (true) {
        RandomNumber = Iuid(rd);

        if (!ItemRandomLocationStruct[RandomNumber].bIsSeatLocation) {
            ItemRandomLocationStruct[RandomNumber].bIsSeatLocation = true;
            return ItemRandomLocationStruct[RandomNumber].sLocation;
        }
    }

    return FVector(0, 0, 0);
}

void ItemController::SpawnItemBoxes(int itemID, std::string itemName, EItemClass itemClass, std::string texturePath, int count, FLOOR itemFloor)
{
    Item_Data newItemData;

    newItemData.itemID = itemID;
    newItemData.itemName = itemName;
    newItemData.itemClass = static_cast<int>(itemClass);
    newItemData.texturePath = texturePath;
    newItemData.count = count;
    newItemData.itemFloor = static_cast<int>(itemFloor);

    // 아이템 초기 위치 설정 (예: 임의의 위치 설정)
    FVector position = RandomItemLocation(itemFloor); // 기본 위치
    newItemData.x = position.x;
    newItemData.y = position.y;
    newItemData.z = position.z;

    items.push_back(newItemData);

    //printf("Spawned Item ID: %d, itemName: %s\n", itemID, itemName.c_str());
}

void ItemController::SendItemData(int id)
{
    Protocol::ItemDataList itemDataList;

    for (const auto& item : items) {
        Protocol::set_item* itemProto = itemDataList.add_items();
        itemProto->set_itemid(item.itemID);
        //cout << "item Name: " << item.itemName << endl;
        itemProto->set_itemname(item.itemName);
        itemProto->set_itemclass(item.itemClass);
        itemProto->set_texture_path(item.texturePath);
        //cout << "item texturePath: " << item.texturePath << endl;
        itemProto->set_count(item.count);
        itemProto->set_floor(item.itemFloor);
        itemProto->set_posx(item.x);
        itemProto->set_posy(item.y);
        itemProto->set_posz(item.z);
    }

    itemDataList.set_packet_type(15);

    std::string serializedData;
    itemDataList.SerializeToString(&serializedData);
    cout << "SendItemData Player #" << id << " - Serialized data size: " << serializedData.size() << endl;

    iocpServer->IOCP_SendPacket(id, serializedData.data(), serializedData.size());
}


int ItemController::RandomCarActorLocation()
{
    int RandomNumber = 0;

    while (true) {
        RandomNumber = Cuid(rd);

        if (!CarActorRandomLocationStruct[RandomNumber].bIsSeatLocation) {
            CarActorRandomLocationStruct[RandomNumber].bIsSeatLocation = true;
            return RandomNumber;
        }
    }


    return -1;
}

int ItemController::RandomCarKey()
{
    int RandomNumber = 0;

    while (true) {
        RandomNumber = Cuid(rd);

        if (!CarKeyRandomStruct[RandomNumber].bIsSeatCarKey) {
            CarKeyRandomStruct[RandomNumber].bIsSeatCarKey = true;
            return RandomNumber;
        }
    }

    return -1;
}

void ItemController::CarActorRandomLocationSetting()
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

void ItemController::CarKeyRandomSetting()
{
    CarKeyRandomStruct[0].CarKeyName = "CarKey1";
    CarKeyRandomStruct[1].CarKeyName = "CarKey2";
    CarKeyRandomStruct[2].CarKeyName = "CarKey3";
    CarKeyRandomStruct[3].CarKeyName = "CarKey4";
    CarKeyRandomStruct[4].CarKeyName = "None";
    CarKeyRandomStruct[5].CarKeyName = "None";
    CarKeyRandomStruct[6].CarKeyName = "None";

    for (int i = 0; i < 7; ++i) {
        CarKeyRandomStruct[i].bIsSeatCarKey = false;
    }
}

void ItemController::SpawnInterItem(int carid, const std::string carname)
{
    Car_Data newCarData;

    newCarData.carID = carid;
    newCarData.carName = carname;

    if (carname == "RoofTopDoorActor"){
        newCarData.x = 2410.f;
        newCarData.y = 200.f;
        newCarData.z = 3940.f;

        newCarData.pitch = 0.f;
        newCarData.yaw = 0.f;
        newCarData.roll = 0.f;

        newCarData.carkeyName = "";
    }

    else {
        // 아이템 초기 위치 설정 (예: 임의의 위치 설정)
        int RandomValue = RandomCarActorLocation();
        int Randomkey = RandomCarKey();
        FVector Location = CarActorRandomLocationStruct[RandomValue].sLocation;
        FRotator Rotation = CarActorRandomLocationStruct[RandomValue].sRotation;
        std::string CarKey = CarKeyRandomStruct[Randomkey].CarKeyName;

        newCarData.x = Location.x;
        newCarData.y = Location.y;
        newCarData.z = Location.z;

        newCarData.pitch = Rotation.Pitch;
        newCarData.yaw = Rotation.Yaw;
        newCarData.roll = Rotation.Roll;

        newCarData.carkeyName = CarKey;
    }

    cars.push_back(newCarData);

    //printf("Spawned InterItem ID: %d, itemName: %s\n", carid, carname.c_str());
}

void ItemController::SendCarData(int id)
{
    Protocol::CarDataList carDataList;

    for (const auto& car : cars) {
        Protocol::set_car* carProto = carDataList.add_cars();
        carProto->set_carid(car.carID);
        carProto->set_carname(car.carName);
        carProto->set_posx(car.x);
        carProto->set_posy(car.y);
        carProto->set_posz(car.z);
        carProto->set_pitch(car.pitch);
        carProto->set_yaw(car.yaw);
        carProto->set_roll(car.roll);
        carProto->set_carkeyname(car.carkeyName);
    }

    carDataList.set_packet_type(16);

    std::string serializedData;
    carDataList.SerializeToString(&serializedData);
    cout << "SendInterItemData Player #" << id << " - Serialized data size: " << serializedData.size() << endl;

    iocpServer->IOCP_SendPacket(id, serializedData.data(), serializedData.size());
}