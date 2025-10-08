#ifndef DOWNWELL_GENERATOR_H
#define DOWNWELL_GENERATOR_H

#include <vector>
#include <random>
#include "Platform.h"
#include "Cookie.h"
#include "Enemy.h"
#include "GameRun.h"

struct DownwellSegment {
    std::vector<Platform> platforms;
    std::vector<Cookie*> cookies;
    std::vector<Enemy*> enemies;
    float segmentHeight;
    int difficulty;
    int floorNumber;
};

class DownwellGenerator {
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
    
public:
    DownwellGenerator();
    
    DownwellSegment generateSegment(int floorNumber, int difficulty);
    
    void setSeed(unsigned int seed);
};

#endif