#ifndef DOWNWELL_GENERATOR_H
#define DOWNWELL_GENERATOR_H

#include <vector>
#include <random>
#include "Platform.h"
#include "Cookie.h"
#include "Enemy.h"
#include "GameRun.h"

enum EnemyType;

struct DownwellSegment
{
    std::vector<Platform> platforms;
    std::vector<Cookie *> cookies;
    std::vector<Enemy *> enemies;
    float segmentHeight;
    int difficulty;
    int floorNumber;
};

enum SectionType
{
    SECTION_EASY_START,
    SECTION_PLATFORMING,
    SECTION_COMBAT,
    SECTION_GAUNTLET,
    SECTION_FINALE
};

class DownwellGenerator
{
private:
    std::mt19937 rng;

    // platform dimensions
    int platformMinWidth;
    int platformMaxWidth;
    float platformVerticalSpacing;
    float platformHorizontalVariation;

    int randomInt(int min, int max);
    float randomFloat(float min, float max);
    bool randomChance(float probability);

    void generateSection(DownwellSegment &segment, SectionType type, float startY, float sectionHeight, int difficulty);
    void generatePlatformCluster(DownwellSegment &segment, float centerY, int count, int difficulty);
    void createSafeZone(DownwellSegment &segment, float zoneY);
    bool hasEnemyNearby(const std::vector<Enemy *> &enemies, float x, float y, float range);
    SDL_Color getPlatformColor(float segmentProgress, int nearbyEnemies);
    void addBridgePlatforms(DownwellSegment &segment);
    void createFloorWithHole(DownwellSegment &segment);

public:
    DownwellGenerator();

    DownwellSegment generateSegment(int floorNumber, int difficulty);

    void setSeed(unsigned int seed);
};

#endif