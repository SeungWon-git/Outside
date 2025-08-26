#pragma once
#include "Common.h"
#include "iocpServerClass.h"
#include "ZombiePathfinder.h"


class IOCP_CORE;


enum class EItemClass
{
	NORMALWEAPON,
	THROWINGWEAPON,
	HEALINGITEM,
	BLEEDINGHEALINGITEM,
	KEYITEM,
	BAGITEM,
	NONE
};

struct Item_Data {
	int itemID;
	std::string itemName;
	int itemClass;
	std::string texturePath;
	int count;
	int itemFloor;
	float x, y, z;
	int owner_playerid = 0;

	Item_Data()
		: itemID(0), itemName(""), itemClass(0), texturePath(""),
		count(0), itemFloor(0), x(0.f), y(0.f), z(0.f) {}

	Item_Data(int id, const std::string& name, int cls, const std::string& texture, int cnt, int floor, float posX, float posY, float posZ)
		: itemID(id), itemName(name), itemClass(cls), texturePath(texture),
		count(cnt), itemFloor(floor), x(posX), y(posY), z(posZ) {}
};

struct Car_Data {
	int carID;
	std::string carName;
	float x, y, z;
	float pitch, yaw, roll;
	std::string carkeyName;

	// 기본 생성자
	Car_Data()
		: carID(0), carName(""), x(0.0f), y(0.0f), z(0.0f),
		pitch(0.0f), yaw(0.0f), roll(0.0f), carkeyName("") {}

	// 매개변수가 있는 생성자
	Car_Data(int id, const std::string& name)
		: carID(id), carName(name), x(0.0f), y(0.0f), z(0.0f),
		pitch(0.0f), yaw(0.0f), roll(0.0f), carkeyName("") {}
};


struct FVector {
	float x, y, z;
};

struct FRotator {
	float Pitch, Yaw, Roll;
};

struct ItemRandomLocation
{
	FVector sLocation;                  // 위치가 어딘가
	bool    bIsSeatLocation = false;    // 위치에 누가 이미 차지하고 있는가 true 
};

struct CarActorRandomLocation
{
	FVector sLocation;
	FRotator sRotation;
	bool bIsSeatLocation = false;
};

struct CarKeyRandom
{
	std::string CarKeyName;
	bool bIsSeatCarKey = false;
};


class ItemController
{
public:
	ItemController(IOCP_CORE* mainServer, int roomid);
	~ItemController();

	void ItemRandomLocationSetting();
    FVector RandomItemLocation(FLOOR itemfloor);
	void SpawnItemBoxes(int itemboxindex, std::string itemname, EItemClass itemclass, string texture, int count, FLOOR itemfloor);
	void SendItemData(int id);

	int RandomCarActorLocation();
	int RandomCarKey();
	void CarActorRandomLocationSetting();
	void CarKeyRandomSetting();
	void SpawnInterItem(int carid, const std::string carname);

	void SendCarData(int id);

public:
	IOCP_CORE* iocpServer;

	ItemRandomLocation  ItemRandomLocationStruct[60];
	CarActorRandomLocation CarActorRandomLocationStruct[7];
	CarKeyRandom CarKeyRandomStruct[7];

	std::vector<Item_Data> items;
	std::vector<Car_Data> cars;

	int item_roomid;
};

