#include "player.h"
#include "constants.h"
#include <cstdio>
#include <cmath>

Player::Player(float startX, float startY)
{
    x = startX;
    y = startY;
    width = HITBOX_WIDTH;   // 27px (9 * 3 scale)
    height = HITBOX_HEIGHT; // 39px (13 * 3 scale)
    velocityX = 0;
    velocityY = 0;
    onGround = false;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;

    // Initialize energy/health (Sugar Rush system)
    energy = MAX_ENERGY;
    maxEnergy = MAX_ENERGY;
    hearts = STARTING_HEARTS;
    maxHearts = MAX_HEARTS;
    isDead = false;
    isInvincible = false;
    invincibilityTimer = 0.0f;

    // Initialize stomp variables
    isStomping = false;
    stompBounce = -10.0f;

    // Animation
    animState = IDLE;
    previousAnimState = IDLE;
    animTimer = 0.0f;
    currentFrame = 0;
    currentRow = 0;
    facingLeft = false;
    deathTimer = 0.0f;
    deathFadeAlpha = 0.0f;
}

void Player::updateAnimation()
{
    // 1. Determine State
    AnimationState newState = animState; // Start with current state
    
    if (isDead) {
        newState = DIE;
    } else if (isGliding) {
        newState = GLIDE;
    } else if (!onGround) {
        // If we're moving slowly and falling slowly, we're probably still on ground
        // (collision resolution can briefly set onGround to false)
        // Only switch to FALL if we're actually falling with significant downward velocity
        if (velocityY < 0) {
            newState = JUMP;
        } else if (velocityY < 5.0f) {
            // Falling slowly - maintain current ground-based animation state
            if (fabs(velocityX) > 0.5f) {
                newState = WALK;
            } else {
                newState = IDLE;
            }
        } else {
            // Actually falling with significant velocity
            newState = FALL;
        }
    } else {
        // On ground - check horizontal movement
        if (fabs(velocityX) > 0.5f) {
            newState = WALK;
        } else {
            newState = IDLE;
        }
    }
    
    
    // Reset animation if state changed (except for FALL <-> GLIDE transitions)
    if (newState != animState) {
        // Check if transitioning between FALL and GLIDE (hair animations are synced)
        bool isFallGlideTransition = (animState == FALL && newState == GLIDE) || 
                                      (animState == GLIDE && newState == FALL);
        
        animState = newState;
        
        // Only reset frame if NOT transitioning between FALL and GLIDE
        if (!isFallGlideTransition) {
            currentFrame = 0;
            animTimer = 0.0f;
        }
        // For FALL <-> GLIDE transitions, keep the current frame for smooth transition
    }

    // 2. Set animation parameters based on state FIRST
    // You control the row, numFrames, and frameDuration here
    int numFrames = 1;
    float frameDuration = 0.2f; // Default
    
    switch (animState)
    {
    case IDLE:
        numFrames = 2; currentRow = 0; frameDuration = 0.5f; break;  // 0.5s per frame = 1 second total for 2 frames
    case WALK:
        numFrames = 4; currentRow = 1; frameDuration = 0.15f; break; // 0.15s per frame = 0.6s total for 4 frames
    case JUMP:
        numFrames = 2; currentRow = 0; frameDuration = 0.2f; break;
    case DIE:
        numFrames = 4; currentRow = 2; frameDuration = 0.2f; break;
    case GLIDE:
        numFrames = 6; currentRow = 3; frameDuration = 0.2; break;
    case FALL:
        numFrames = 6; currentRow = 4; frameDuration = 0.1f; break;
    default: break;
    }

    // 3. Update Timer & Frame (using the frameDuration set above)
    animTimer += 1.0f / 60.0f; // Assuming 60 FPS update

    if (animTimer >= frameDuration)
    {
        animTimer -= frameDuration; // Preserve overflow time for smooth animation
        
        // Don't loop death animation - hold on last frame
        if (animState == DIE && currentFrame >= numFrames - 1) {
            currentFrame = numFrames - 1;
            animTimer = 0; // Reset timer when death animation is locked
        } else {
            currentFrame = (currentFrame + 1) % numFrames;
        }
    }
}

SDL_Rect Player::getSpriteSrcRect()
{
    // currentRow is set in updateAnimation() based on the animation state
    // This just returns the source rectangle for the current frame and row
    return {currentFrame * SPRITE_SIZE, currentRow * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
}

void Player::jump()
{
    if (onGround)
    {
        // Weaker jump when sluggish (no energy)
        if (isSluggish())
        {
            velocityY = SLUGGISH_JUMP_FORCE;
            // printf("    Player::jump() called - SLUGGISH jump, velocityY set to %f\n", velocityY);
        }
        else
        {
            velocityY = JUMP_FORCE;
            // printf("    Player::jump() called - velocityY set to %f\n", velocityY);
        }
        onGround = false;
        isGliding = false;
    }
    else
    {
        // printf("    Player::jump() BLOCKED - not on ground\n");
    }
}

void Player::startGliding()
{
    // Can't glide when sluggish (no energy)
    if (isSluggish())
    {
        // printf("    Can't glide - no energy! Need cookies!\n");
        return;
    }

    // Can only glide if not on ground, has glide time
    if (!onGround && glideTime > 0)
    {
        isGliding = true;
    }
}

void Player::stopGliding()
{
    isGliding = false;
}

void Player::moveLeft()
{
    // Slower movement when sluggish
    if (isSluggish())
    {
        velocityX = -SLUGGISH_MOVE_SPEED;
    }
    else
    {
        velocityX = -MOVE_SPEED;
    }
}

void Player::moveRight()
{
    // Slower movement when sluggish
    if (isSluggish())
    {
        velocityX = SLUGGISH_MOVE_SPEED;
    }
    else
    {
        velocityX = MOVE_SPEED;
    }
}

void Player::stopMoving()
{
    velocityX = 0;
}

void Player::restoreEnergy(float amount)
{
    energy += amount;
    if (energy > maxEnergy)
    {
        energy = maxEnergy;
    }
    // printf("Energy restored! Current energy: %.1f\n", energy);
}

bool Player::isSluggish()
{
    return energy <= 0;
}

void Player::loseHeart()
{
    if (isInvincible)
    {
        return; // Don't lose hearts while invincible
    }

    hearts--;
    isInvincible = true;
    invincibilityTimer = INVINCIBILITY_TIME;

    if (hearts <= 0)
    {
        hearts = 0;
        isDead = true;
        deathTimer = 0.0f;  // Start death animation timer
        deathFadeAlpha = 0.0f;  // Start with no overlay
        // printf("Player died! Game Over\n");
    }
    else
    {
        // printf("Hit by enemy! Hearts remaining: %d (Invincible for %.1fs)\n", hearts, INVINCIBILITY_TIME);
    }
}

// FULL RESET... want to keep some data persistent when moving between levels... like hearts
void Player::reset(float startX, float startY)
{
    x = startX;
    y = startY;
    velocityX = 0;
    velocityY = 0;

    hearts = STARTING_HEARTS;
    maxHearts = STARTING_HEARTS;
    energy = MAX_ENERGY;
    maxEnergy = MAX_ENERGY;

    onGround = false;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;
    isDead = false;
    isInvincible = false;
    invincibilityTimer = 0;
}

void Player::update()
{
    if (isDead)
    {
        // Update death timer and animation
        deathTimer += 1.0f / FPS;
        updateAnimation();  // Continue animating during death
        
        // Apply gravity so player falls if dead in mid-air
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED)
        {
            velocityY = MAX_FALL_SPEED;
        }
        
        // Update position
        y += velocityY;
        
        // Start fading in the death overlay after animation plays for a bit
        // Death animation is 4 frames * 0.2s = 0.8s total
        // Wait 0.8s for animation, then fade in over 1 second
        const float FADE_START_TIME = 0.8f;
        const float FADE_DURATION = 1.0f;
        
        if (deathTimer > FADE_START_TIME)
        {
            float fadeProgress = (deathTimer - FADE_START_TIME) / FADE_DURATION;
            if (fadeProgress > 1.0f) fadeProgress = 1.0f;
            deathFadeAlpha = fadeProgress * 200.0f;  // Max alpha of 200 (semi-transparent)
        }
        
        return;
    }

    // Update invincibility timer
    if (isInvincible)
    {
        invincibilityTimer -= 1.0f / FPS;
        if (invincibilityTimer <= 0)
        {
            isInvincible = false;
            invincibilityTimer = 0;
        }
    }

    // ONLY drain energy when gliding (not just existing!)
    if (isGliding && glideTime > 0 && !onGround)
    {
        energy -= ENERGY_DRAIN_RATE / FPS;
        if (energy < 0)
        {
            energy = 0;
        }
    }

    // Stop gliding if energy runs out
    if (isSluggish() && isGliding)
    {
        isGliding = false;
        printf("Sugar crash! Can't glide anymore - need cookies!\n");
    }

    // Stomp detection
    if (velocityY > 0 && !onGround)
    {
        isStomping = true;
    }
    else
    {
        isStomping = false;
    }

    // Gliding: constant fall speed
    if (isGliding && glideTime > 0 && !onGround)
    {
        velocityY = GLIDE_FALL_SPEED;
        glideTime -= 1.0f / FPS;

        if (glideTime <= 0)
        {
            glideTime = 0;
            isGliding = false;
        }
    }
    else
    {
        // Gravity when not gliding
        velocityY += GRAVITY;
        if (!onGround)
        {
            isGliding = false;
        }

        // Cap fall speed
        if (velocityY > MAX_FALL_SPEED)
        {
            velocityY = MAX_FALL_SPEED;
        }
    }

    // Reset glide when on ground
    if (onGround)
    {
        glideTime = MAX_GLIDE_TIME;
    }

    x += velocityX;
    y += velocityY;

    // Update facing direction
    if (velocityX < -0.1f) facingLeft = true;
    if (velocityX > 0.1f) facingLeft = false;

    updateAnimation();
}

void Player::render(SDL_Renderer *renderer, bool showBars)
{
    if (isDead)
    {
        return; // Don't render if dead
    }

    // NOTE: Invincibility blink is handled in game.cpp via color modulation
    // NOTE: Player sprite rendering is now handled in game.cpp
    // This function only handles the energy/glide bars

    // Only show bars if requested (hide in lobby)
    if (!showBars)
    {
        return;
    }

    // Draw glide bar above player
    int barWidth = 30;
    int barHeight = 4;
    int barX = (int)x + 1;
    int barY = (int)y - 10;

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect barBg = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &barBg);

    float glidePercent = glideTime / MAX_GLIDE_TIME;
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_Rect glideMeter = {barX, barY, (int)(barWidth * glidePercent), barHeight};
    SDL_RenderFillRect(renderer, &glideMeter);

    // Draw energy bar above glide bar (Sugar Rush meter)
    int energyBarY = barY - 8;
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect energyBarBg = {barX, energyBarY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &energyBarBg);

    float energyPercent = energy / maxEnergy;
    // Color based on energy level
    if (energyPercent > 0.5f)
    {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold (sugar rush!)
    }
    else if (energyPercent > 0.25f)
    {
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange (fading)
    }
    else if (energyPercent > 0)
    {
        SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255); // Red (crashing)
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Gray (0)
    }
    SDL_Rect energyMeter = {barX, energyBarY, (int)(barWidth * energyPercent), barHeight};
    SDL_RenderFillRect(renderer, &energyMeter);
}

void Player::setPosition(float newX, float newY)
{
    x = newX;
    y = newY;
    velocityX = 0;
    velocityY = 0;
    isGliding = false;
    glideTime = MAX_GLIDE_TIME;
}

SDL_Rect Player::getRect() const
{
    return {(int)x, (int)y, (int)width, (int)height};
}

SDL_Rect Player::getFeetRect() const
{
    return {(int)x, (int)(y + height * 0.66f), (int)width, (int)(height * 0.34f)};
}