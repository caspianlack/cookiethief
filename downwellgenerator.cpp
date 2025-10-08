#include "downwellgenerator.h"
#include "constants.h"
#include <ctime>
#include <cstdio>

DownwellGenerator::DownwellGenerator() {
    rng.seed(time(nullptr));
    
    platformMinWidth = 80;
    platformMaxWidth = 180;
    platformVerticalSpacing = 150.0f;
    platformHorizontalVariation = 200.0f;
}

void DownwellGenerator::setSeed(unsigned int seed) {
    rng.seed(seed);
}

int DownwellGenerator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

float DownwellGenerator::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

bool DownwellGenerator::randomChance(float probability) {
    return randomFloat(0.0f, 1.0f) < probability;
}

DownwellSegment DownwellGenerator::generateSegment(int floorNumber, int difficulty) {
    DownwellSegment segment;
    segment.floorNumber = floorNumber;
    segment.difficulty = difficulty;
    
    segment.segmentHeight = 3500.0f + (difficulty * 800.0f);
    
    printf("Generating Downwell segment for floor %d (difficulty %d, height %.0f)\n", 
           floorNumber, difficulty, segment.segmentHeight);
    
    // Generate platforms going downward (within middle of screen)
    float currentY = 100.0f;
    int platformCount = 0;
    
    while (currentY < segment.segmentHeight) {
        int width = randomInt(platformMinWidth, platformMaxWidth);
        
        // keep inside middle (pit)
        int margin = 20;
        int minX = PIT_LEFT + margin;
        int maxX = PIT_RIGHT - width - margin;
        int x = randomInt(minX, maxX);
        
        int height = 20;
        
        SDL_Color color;
        if (currentY < segment.segmentHeight * 0.33f) {
            color = {139, 69, 19, 255};
        } else if (currentY < segment.segmentHeight * 0.66f) {
            color = {100, 100, 120, 255};
        } else {
            color = {80, 80, 100, 255};
        }
        
        Platform platform = {(float)x, currentY, (float)width, (float)height, color};
        segment.platforms.push_back(platform);
        
        // add cookies on half platforms
        // TODO: add purchasable luck to also increase by a factor
        if (randomChance(0.5f)) {
            float cookieX = x + width / 2 - 10;
            float cookieY = currentY - 25;
            segment.cookies.push_back(new Cookie(cookieX, cookieY));
        }
        
        // add enemies on some platforms depending on difficulty
        float enemyChance = 0.15f + (difficulty * 0.05f);
        if (randomChance(enemyChance) && platformCount > 2) {
            float enemyX = x + width / 2 - 16;
            float enemyY = currentY - 48;
            segment.enemies.push_back(new Enemy(enemyX, enemyY));
        }
        
        float spacing = platformVerticalSpacing - (difficulty * 10.0f);
        if (spacing < 80.0f) spacing = 80.0f;
        
        currentY += spacing + randomFloat(-20.0f, 20.0f);
        platformCount++;
    }
    
    // airborn cookies
    int floatingCookies = 3 + difficulty;
    for (int i = 0; i < floatingCookies; i++) {
        float cookieX = randomFloat(PIT_LEFT + 30, PIT_RIGHT - 50);
        float cookieY = randomFloat(200, segment.segmentHeight - 200);
        segment.cookies.push_back(new Cookie(cookieX, cookieY));
    }
    
    printf("Generated %d platforms, %d cookies, %d enemies\n", 
           platformCount, (int)segment.cookies.size(), (int)segment.enemies.size());
    
    return segment;
}
