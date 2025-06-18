#pragma once

#include <set>
#include <fstream>
#include <sstream>
#include <string>

//#include "iocpServerClass.h"
#include "Node.h"




using namespace std;


enum class FLOOR {
    FLOOR_B2,
    FLOOR_B1,
    FLOOR_F1,
    FLOOR_F2,
    FLOOR_F3
};


class ZombiePathfinder {
public:
    ZombiePathfinder() = default;

    ZombiePathfinder(int roomId, int ownerZombieId, float startX, float startY, float startZ, float goalX, float goalY, float goalZ)
        : roomID(roomId), ownerZombieID(ownerZombieId), startX(startX), startY(startY), startZ(startZ), goalX(goalX), goalY(goalY), goalZ(goalZ) { }

    void Run(vector<tuple<float, float, float>>& t, int patroltype);

    void UpdateStartGoal(float startX, float startY, float startZ, float goalX, float goalY, float goalZ);
    float OBSTACLE_RADIUS = 25.f;

    FLOOR floor;

    int roomID;
    int ownerZombieID;

private:
    float startX, startY, startZ;
    float goalX, goalY, goalZ;
    float beforegoalX = 0.f;
    float beforegoalY = 0.f;
    float beforegoalZ = 0.f;
    int  PatrolType = 0; // 0일때는 일반 동작 , 1일때는 patrol check

    vector<tuple<float, float, float>> validPositions;
    vector<tuple<float, float, float>> obstacles;
    const float OBSTACLE_CHECK_INTERVAL = 25.0f;

    void SetObstacleRadius(float radious) { OBSTACLE_RADIUS = radious; }
    void SetPatrolType(int patroltype) { PatrolType = patroltype; }

    void DetermineFloor();


    void PrintObstacles();

    void FindPath(vector<tuple<float, float, float>>& t);


    vector<Node> FindNeighbors(const Node& current);

    vector<Node> AStar(float startX, float startY, float startZ, float goalX, float goalY, float goalZ,
        const vector<tuple<float, float, float>>& validPositions,
        const vector<tuple<float, float, float>>& obstacles); 
    
    
    vector<Node> NewAStar(float startX, float startY, float startZ, float goalX, float goalY, float goalZ);

    bool IsPathBlockedByObstacle(const Node& startNode, const Node& endNode);

    float FloorZPos();

    double Heuristic(float x1, float y1, float x2, float y2) {
        return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
    }

    double NewMaxeuristic(float x1, float y1, float x2, float y2, float maxWeight) {
        return maxWeight * sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
    }

    bool IsInObstacleRange(float x, float y, float z);

    double EuclideanDistance(float x1, float y1, float x2, float y2);
    tuple<float, float, float> FindClosestValidPosition(float goalX, float goalY, float goalZ);
};


