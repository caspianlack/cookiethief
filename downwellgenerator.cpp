#include "downwellgenerator.h"
#include "constants.h"
#include <ctime>
#include <cstdio>
#include <cmath>
#include <algorithm>

DownwellGenerator::DownwellGenerator()
{
    rng.seed(time(nullptr));

    platformMinWidth = 80;
    platformMaxWidth = 180;
    platformVerticalSpacing = 150.0f;
    platformHorizontalVariation = 200.0f;
}

void DownwellGenerator::setSeed(unsigned int seed)
{
    rng.seed(seed);
}

int DownwellGenerator::randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

float DownwellGenerator::randomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

bool DownwellGenerator::randomChance(float probability)
{
    return randomFloat(0.0f, 1.0f) < probability;
}

bool DownwellGenerator::hasEnemyNearby(const std::vector<Enemy *> &enemies, float x, float y, float range)
{
    for (const auto *enemy : enemies)
    {
        float dist = sqrt(pow(enemy->x - x, 2) + pow(enemy->y - y, 2));
        if (dist < range)
            return true;
    }
    return false;
}

SDL_Color DownwellGenerator::getPlatformColor(float segmentProgress, int nearbyEnemies)
{
    if (nearbyEnemies > 2)
    {
        return {120, 60, 40, 255};
    }
    else if (segmentProgress < 0.33f)
    {
        return {139, 69, 19, 255};
    }
    else if (segmentProgress < 0.66f)
    {
        return {100, 100, 120, 255};
    }
    else
    {
        return {80, 80, 100, 255};
    }
}

void DownwellGenerator::generatePlatformCluster(DownwellSegment &segment, float centerY, int count, int difficulty)
{
    printf("  Generating platform cluster at Y=%.1f with %d platforms\n", centerY, count);

    float clusterSpread = 150.0f; // Overall horizontal spread of the cluster
    float clusterCenterX = PIT_LEFT + PIT_WIDTH / 2;

    std::vector<SDL_Rect> placedPlatforms; // Used to prevent overlapping placements

    for (int i = 0; i < count; i++)
    {
        bool validPosition = false;
        int attempts = 0;
        float x, y;
        int width;

        // Attempt to find a valid, non-overlapping position
        while (!validPosition && attempts < 20)
        {
            float offsetX = randomFloat(-clusterSpread / 2, clusterSpread / 2);
            float offsetY = randomFloat(-60, 60);

            width = randomInt(90, 130);
            x = clusterCenterX + offsetX - width / 2;
            y = centerY + offsetY + (i * 80);

            // Clamp platforms within pit boundaries
            if (x < PIT_LEFT + 20)
                x = PIT_LEFT + 20;
            if (x + width > PIT_RIGHT - 20)
                x = PIT_RIGHT - 20 - width;

            SDL_Rect newPlat = {(int)x, (int)y, width, 20};

            // Check overlap with other platforms
            validPosition = true;
            for (const auto &existing : placedPlatforms)
            {
                if (SDL_HasIntersection(&newPlat, &existing))
                {
                    validPosition = false;
                    break;
                }

                // Also reject if too close to another platform
                float dx = abs(existing.x - newPlat.x);
                float dy = abs(existing.y - newPlat.y);
                if (dx < 100 && dy < 70)
                {
                    validPosition = false;
                    break;
                }
            }

            attempts++;
        }

        if (!validPosition)
            continue; // Skip if no suitable position found

        SDL_Color color = getPlatformColor(centerY / segment.segmentHeight, 0);
        segment.platforms.push_back({x, y, (float)width, 20.0f, color});
        placedPlatforms.push_back({(int)x, (int)y, width, 20});

        // Occasionally place a cookie on the platform
        if (randomChance(0.4f))
        {
            float cookieX = x + width / 2 - 10;
            float cookieY = y - 25;
            segment.cookies.push_back(new Cookie(cookieX, cookieY));
        }
    }
}

void DownwellGenerator::createSafeZone(DownwellSegment &segment, float zoneY)
{
    printf("  Creating safe zone at Y=%.1f\n", zoneY);

    // Create a wide, safe platform
    float safeX = PIT_LEFT + 50;
    float safeWidth = 300.0f;
    segment.platforms.push_back({safeX, zoneY, safeWidth, 25.0f, {100, 200, 100, 255}}); // Green = safety area

    // Add healing cookies
    segment.cookies.push_back(new Cookie(safeX + 100, zoneY - 25));
    segment.cookies.push_back(new Cookie(safeX + 200, zoneY - 25));

    // Remove enemies near the safe zone
    auto it = segment.enemies.begin();
    while (it != segment.enemies.end())
    {
        if (fabs((*it)->y - zoneY) < 250)
        {
            delete *it;
            it = segment.enemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DownwellGenerator::generateSection(DownwellSegment &segment, SectionType type, float startY, float sectionHeight, int difficulty)
{
    float currentY = startY;
    int platformCount = 0;

    // Parameters depend on section type
    float enemyChance, spacing, minWidth, maxWidth;

    switch (type)
    {
    case SECTION_EASY_START:
        enemyChance = 0.05f;
        spacing = 180.0f;
        minWidth = 120;
        maxWidth = 180;
        break;

    case SECTION_PLATFORMING:
        enemyChance = 0.1f + (difficulty * 0.03f);
        spacing = 130.0f - (difficulty * 5);
        minWidth = 80;
        maxWidth = 140;
        break;

    case SECTION_COMBAT:
        enemyChance = 0.25f + (difficulty * 0.05f);
        spacing = 150.0f;
        minWidth = 110;
        maxWidth = 160;
        break;

    case SECTION_GAUNTLET:
        enemyChance = 0.3f + (difficulty * 0.08f);
        spacing = 110.0f - (difficulty * 5);
        minWidth = 70;
        maxWidth = 120;
        break;

    case SECTION_FINALE:
        enemyChance = 0.15f + (difficulty * 0.05f);
        spacing = 140.0f;
        minWidth = 100;
        maxWidth = 150;
        break;
    }

    // Keep spacing within reasonable limits
    if (spacing < 90.0f)
        spacing = 90.0f;
    if (spacing > 200.0f)
        spacing = 200.0f;

    float lastPlatformY = startY;
    float lastPlatformX = -999;
    float lastPlatformWidth = 0;

    while (currentY < startY + sectionHeight)
    {
        int width = randomInt(minWidth, maxWidth);

        int margin = 20;
        int minX = PIT_LEFT + margin;
        int maxX = PIT_RIGHT - width - margin;
        int x = randomInt(minX, maxX);

        // Ensure vertical distance from last platform
        if (currentY - lastPlatformY < 80.0f)
        {
            currentY = lastPlatformY + 80.0f;
        }

        // Prevent overlapping platforms at similar heights
        if (currentY - lastPlatformY < 150.0f)
        {
            float horizontalOverlap = 0;

            if (x < lastPlatformX + lastPlatformWidth && x + width > lastPlatformX)
            {
                horizontalOverlap = (lastPlatformX + lastPlatformWidth) - x;
                x += horizontalOverlap + 30;

                if (x + width > maxX)
                {
                    x = lastPlatformX - width - 30;
                }

                if (x < minX || x + width > maxX)
                {
                    currentY += spacing + randomFloat(-10.0f, 10.0f);
                    continue;
                }
            }
        }

        float segmentProgress = currentY / segment.segmentHeight;

        // Count enemies nearby to influence platform color
        int nearbyEnemies = 0;
        for (const auto *enemy : segment.enemies)
        {
            if (fabs(enemy->y - currentY) < 200)
                nearbyEnemies++;
        }

        SDL_Color color = getPlatformColor(segmentProgress, nearbyEnemies);
        Platform platform = {(float)x, currentY, (float)width, 20.0f, color};
        segment.platforms.push_back(platform);

        lastPlatformY = currentY;
        lastPlatformX = x;
        lastPlatformWidth = width;

        // Occasionally add a cookie above the platform
        float cookieChance = (type == SECTION_GAUNTLET) ? 0.5f : 0.4f;
        if (randomChance(cookieChance))
        {
            float cookieX = x + width / 2 - 10;
            float cookieY = currentY - 25;
            segment.cookies.push_back(new Cookie(cookieX, cookieY));
        }

        // Add enemies, avoiding close clustering
        if (randomChance(enemyChance) && platformCount > 1)
        {
            float enemyX = x + width / 2 - 16;
            float enemyY = currentY - 48;

            if (!hasEnemyNearby(segment.enemies, enemyX, enemyY, 200.0f))
            {
                EnemyType enemyType = ENEMY_PATROL;

                // Choose enemy type based on section and difficulty
                if (type == SECTION_GAUNTLET)
                {
                    int typeRoll = randomInt(0, 5);
                    if (typeRoll <= 2)
                        enemyType = ENEMY_JUMPER;
                    else if (typeRoll <= 4)
                        enemyType = ENEMY_SHOOTER;
                }
                else if (difficulty == 0)
                {
                    enemyType = ENEMY_PATROL;
                }
                else if (difficulty == 1)
                {
                    int typeRoll = randomInt(0, 3);
                    enemyType = (typeRoll == 0) ? ENEMY_SHOOTER : ENEMY_PATROL;
                }
                else if (difficulty <= 3)
                {
                    int typeRoll = randomInt(0, 2);
                    enemyType = (EnemyType)typeRoll;
                }
                else
                {
                    int typeRoll = randomInt(0, 4);
                    if (typeRoll == 0)
                        enemyType = ENEMY_PATROL;
                    else if (typeRoll <= 2)
                        enemyType = ENEMY_JUMPER;
                    else
                        enemyType = ENEMY_SHOOTER;
                }

                segment.enemies.push_back(new Enemy(enemyX, enemyY, enemyType, difficulty));
            }
        }

        // Apply random variation to spacing
        float spacingVariation = spacing * 0.15f;
        currentY += spacing + randomFloat(-spacingVariation, spacingVariation);
        platformCount++;
    }
}

void DownwellGenerator::addBridgePlatforms(DownwellSegment &segment)
{
    printf("  Adding bridge platforms for large gaps...\n");

    // Sort platforms by Y-coordinate
    std::vector<Platform *> sortedPlatforms;
    for (auto &plat : segment.platforms)
    {
        sortedPlatforms.push_back(&plat);
    }

    std::sort(sortedPlatforms.begin(), sortedPlatforms.end(),
              [](Platform *a, Platform *b)
              { return a->y < b->y; });

    std::vector<Platform> bridgesToAdd;

    // Look for vertical gaps and fill them with bridges
    for (size_t i = 0; i < sortedPlatforms.size() - 1; i++)
    {
        Platform *upper = sortedPlatforms[i];
        Platform *lower = sortedPlatforms[i + 1];

        float verticalGap = lower->y - (upper->y + upper->height);

        // If platforms are too far apart, consider adding a bridge
        if (verticalGap > 200.0f)
        {
            float midY = upper->y + (verticalGap / 2) + upper->height;

            float upperLeft = upper->x;
            float upperRight = upper->x + upper->width;
            float lowerLeft = lower->x;
            float lowerRight = lower->x + lower->width;

            bool needsBridge = false;
            float bridgeX = 0;

            // No horizontal overlap
            if (upperRight < lowerLeft || lowerRight < upperLeft)
            {
                needsBridge = true;
                float gap = (upperRight < lowerLeft) ? (lowerLeft - upperRight) : (upperRight - lowerLeft);

                bridgeX = (upperRight < lowerLeft)
                              ? upperRight + gap / 2 - 40
                              : lowerRight + gap / 2 - 40;
            }
            // Minimal overlap — still add a bridge
            else
            {
                float overlapLeft = std::max(upperLeft, lowerLeft);
                float overlapRight = std::min(upperRight, lowerRight);
                float overlapWidth = overlapRight - overlapLeft;
                float minPlatformWidth = std::min(upper->width, lower->width);

                if (overlapWidth < minPlatformWidth * 0.5f)
                {
                    needsBridge = true;
                    bridgeX = overlapLeft + overlapWidth / 2 - 40;
                }
            }

            if (needsBridge)
            {
                float bridgeWidth = 80.0f;
                if (bridgeX < PIT_LEFT + 20)
                    bridgeX = PIT_LEFT + 20;
                if (bridgeX + bridgeWidth > PIT_RIGHT - 20)
                {
                    bridgeX = PIT_RIGHT - 20 - bridgeWidth;
                }

                SDL_Color bridgeColor = {120, 100, 150, 255};
                Platform bridge = {bridgeX, midY, bridgeWidth, 15.0f, bridgeColor};

                // Skip if overlapping with an existing platform
                bool overlapsExisting = false;
                SDL_Rect bridgeRect = bridge.getRect();
                for (const auto &existingPlat : segment.platforms)
                {
                    SDL_Rect existingRect = existingPlat.getRect();
                    if (SDL_HasIntersection(&bridgeRect, &existingRect))
                    {
                        overlapsExisting = true;
                        break;
                    }
                }

                if (!overlapsExisting)
                {
                    bridgesToAdd.push_back(bridge);
                    printf("    Added bridge at Y=%.1f between platforms at Y=%.1f and Y=%.1f\n",
                           midY, upper->y, lower->y);

                    // Sometimes add a cookie above the bridge
                    if (randomChance(0.5f))
                    {
                        float cookieX = bridgeX + bridgeWidth / 2 - 10;
                        float cookieY = midY - 25;
                        segment.cookies.push_back(new Cookie(cookieX, cookieY));
                    }
                }
            }
        }
    }

    // Append bridges to the main list
    for (const auto &bridge : bridgesToAdd)
    {
        segment.platforms.push_back(bridge);
    }

    printf("  Added %d bridge platforms\n", (int)bridgesToAdd.size());
}

void DownwellGenerator::createFloorWithHole(DownwellSegment &segment)
{
    printf("  Creating floor with exit hole at bottom\n");

    float floorY = segment.segmentHeight - 50;
    float holeWidth = 120.0f;
    float holeCenterX = PIT_LEFT + PIT_WIDTH / 2;

    // Left floor section
    float leftFloorX = PIT_LEFT;
    float leftFloorWidth = (holeCenterX - holeWidth / 2) - PIT_LEFT;

    if (leftFloorWidth > 20)
    {
        segment.platforms.push_back({leftFloorX,
                                     floorY,
                                     leftFloorWidth,
                                     200.0f,
                                     {60, 60, 60, 255}});
    }

    // Right floor section
    float rightFloorX = holeCenterX + holeWidth / 2;
    float rightFloorWidth = PIT_RIGHT - rightFloorX;

    if (rightFloorWidth > 20)
    {
        segment.platforms.push_back({rightFloorX,
                                     floorY,
                                     rightFloorWidth,
                                     200.0f,
                                     {60, 60, 60, 255}});
    }

    printf("  Floor created with hole at X=%.1f, Width=%.1f\n",
           holeCenterX - holeWidth / 2, holeWidth);
}

DownwellSegment DownwellGenerator::generateSegment(int floorNumber, int difficulty)
{
    DownwellSegment segment;
    segment.floorNumber = floorNumber;
    segment.difficulty = difficulty;

    segment.segmentHeight = 3500.0f + (difficulty * 800.0f);

    printf("Generating Downwell segment for floor %d (difficulty %d, height %.0f)\n",
           floorNumber, difficulty, segment.segmentHeight);

    float currentY = 100.0f;

    // 1. Warm-up section (top 15%)
    float easyHeight = segment.segmentHeight * 0.15f;
    generateSection(segment, SECTION_EASY_START, currentY, easyHeight, difficulty);
    currentY += easyHeight + 50;

    // 2. Build-up section (next ~20%)
    float buildHeight = segment.segmentHeight * 0.18f;
    SectionType buildType = (difficulty < 3) ? SECTION_PLATFORMING : SECTION_COMBAT;
    generateSection(segment, buildType, currentY, buildHeight, difficulty);
    currentY += buildHeight + 80;

    // 3. Main challenge (core of the level)
    float challengeHeight = segment.segmentHeight * 0.35f;
    SectionType challengeType = (difficulty < 2) ? SECTION_PLATFORMING : (difficulty < 5) ? SECTION_COMBAT
                                                                                          : SECTION_GAUNTLET;
    generateSection(segment, challengeType, currentY, challengeHeight, difficulty);
    currentY += challengeHeight + 80;

    // 4. Finale section before exit
    float finaleHeight = segment.segmentHeight - currentY - 300;
    if (finaleHeight > 200)
    {
        generateSection(segment, SECTION_FINALE, currentY, finaleHeight, difficulty);
    }

    // Add floor and bridges
    createFloorWithHole(segment);
    addBridgePlatforms(segment);

    // Scatter a few floating cookies for bonuses
    int floatingCookies = 2 + (difficulty / 2);
    for (int i = 0; i < floatingCookies; i++)
    {
        float cookieX = randomFloat(PIT_LEFT + 40, PIT_RIGHT - 60);
        float cookieY = randomFloat(300, segment.segmentHeight - 400);
        segment.cookies.push_back(new Cookie(cookieX, cookieY));
    }

    printf("Generated %d platforms, %d cookies, %d enemies in structured sections\n",
           (int)segment.platforms.size(), (int)segment.cookies.size(),
           (int)segment.enemies.size());

    return segment;
}
