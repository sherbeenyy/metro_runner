#ifndef PLAYER_H
#define PLAYER_H
#include <iostream>

class Player {
public:
    float x, y;
    float velocityY;
    float width, height;
    bool isJumping;
    bool isDucking;
    int headIndex;
    
    // Special abilities
    bool abilityActive;
    float abilityTimer;
    float abilityCooldown;
    bool canDoubleJump;
    bool hasDoubleJumped;
    
    Player() : x(200), y(400), velocityY(0), width(40), height(80), 
               isJumping(false), isDucking(false), headIndex(0),
               abilityActive(false), abilityTimer(0), abilityCooldown(0),
               canDoubleJump(false), hasDoubleJumped(false) {}
    
    void jump() {
        // Normal jump
        if (!isJumping) {
            velocityY = -12;
            isJumping = true;
            hasDoubleJumped = false;
        }
        // Character 1 (p2.PNG): Double Jump ability - can jump in air
        else if (headIndex == 1 && isJumping && !hasDoubleJumped && abilityActive) {
            velocityY = -12;
            hasDoubleJumped = true;
            std::cout << "DOUBLE JUMP!" << std::endl;
        }
    }
    
    void activateAbility() {
        if (abilityCooldown <= 0) {
            abilityActive = true;
            // Character 0: Shield - 5 seconds invincibility
            // Character 1: Double Jump - 8 seconds can jump in air
            // Character 2: Magnet - 6 seconds double coins
            // Character 3: Dash - 5 seconds move faster (player runs ahead)
            if (headIndex == 0) {
                abilityTimer = 5.0f;
                std::cout << "Shield activated!" << std::endl;
            } else if (headIndex == 1) {
                abilityTimer = 8.0f;
                canDoubleJump = true;
                std::cout << "Double Jump activated!" << std::endl;
            } else if (headIndex == 2) {
                abilityTimer = 6.0f;
                std::cout << "Magnet activated!" << std::endl;
            } else if (headIndex == 3) {
                abilityTimer = 5.0f;
                std::cout << "Dash activated!" << std::endl;
            }
            abilityCooldown = 8.0f; // 8 seconds cooldown for all
        }
    }
    
    void duck() {
        if (!isJumping) {
            isDucking = true;
            height = 40;
        }
    }
    
    void stopDuck() {
        if (isDucking) {
            isDucking = false;
            height = 80;
        }
    }
    
    void update(float groundY, float deltaTime) {
        // Update ability timers
        if (abilityActive) {
            abilityTimer -= deltaTime;
            if (abilityTimer <= 0) {
                abilityActive = false;
                if (headIndex == 1) {
                    canDoubleJump = false;
                }
            }
        }
        if (abilityCooldown > 0) {
            abilityCooldown -= deltaTime;
        }
        
        y += velocityY;
        
        if (isJumping) {
            velocityY += 0.5f;  // Same gravity for all characters
            
            if (y >= groundY) {
                y = groundY;
                velocityY = 0;
                isJumping = false;
            }
        } else {
            if (y > groundY) {
                y = groundY;
                velocityY = 0;
            }
        }
    }
    
    // Speed multiplier not used anymore (slow time removed)
    float getSpeedMultiplier() {
        return 1.0f;
    }
    
    // Check if character is invincible
    bool isInvincible() const {
        // Character 0 (p1.PNG): Shield - invincibility during ability
        return headIndex == 0 && abilityActive;
    }
    
    // Check if character can double coin collection
    bool hasDoubleCoinBonus() {
        // Character 2 (p3.PNG): Coin Magnet - collects double coins during ability
        return headIndex == 2 && abilityActive;
    }
    
    // Get player speed multiplier for dash ability
    float getPlayerSpeedMultiplier() {
        // Character 3 (P4.PNG): Dash - player moves faster (world moves slower relative to player)
        if (headIndex == 3 && abilityActive) {
            return 1.8f; // Game speed increases making player appear faster
        }
        return 1.0f;
    }
};

#endif
