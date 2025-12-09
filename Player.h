#ifndef PLAYER_H
#define PLAYER_H

/**
 * Player Class
 * ============
 * 
 * PURPOSE:
 * Manages player physics, movement, and character-specific abilities.
 * Handles jumping mechanics including double jump and gravity simulation.
 * 
 * RESPONSIBILITIES:
 * - Track player position and velocity
 * - Handle jump physics with gravity
 * - Manage character-specific abilities (4 unique abilities)
 * - Track ability timers and cooldowns
 * - Provide speed multipliers for dash ability
 * - Handle double jump mechanic for specific character
 * 
 * PHYSICS:
 * - Player dimensions: 40px width x 80px height
 * - Jump velocity: -12 (upward)
 * - Gravity: 0.5f (downward acceleration)
 * - Delta-time based movement for smooth animation
 * - Ground collision detection for landing
 * 
 * CHARACTER ABILITIES (4 CHARACTERS):
 * 
 * Character 0 (BIG JOE) - Shield:
 * - Duration: 5 seconds
 * - Effect: Complete invincibility to obstacles
 * - Cooldown: 8 seconds
 * 
 * Character 1 (ALI 3ALOKA) - Double Jump:
 * - Duration: 8 seconds
 * - Effect: Can jump again while in air
 * - Cooldown: 8 seconds
 * - Implementation: canDoubleJump flag, hasDoubleJumped tracking
 * 
 * Character 2 (HAMDA) - Magnet:
 * - Duration: 6 seconds
 * - Effect: Coins worth 2x (double coin bonus)
 * - Cooldown: 8 seconds
 * 
 * Character 3 (SPEEDY) - Dash:
 * - Duration: 5 seconds
 * - Effect: 1.8x speed multiplier
 * - Cooldown: 8 seconds
 * - Affects player movement and world scroll speed
 * 
 * ABILITY SYSTEM:
 * - Activation: Q key press
 * - All abilities have 8 second cooldown after use
 * - Timers count down in real-time (delta-time based)
 * - Visual feedback through HUD (timer/cooldown display)
 * - Debug output on ability activation
 * 
 * STATE TRACKING:
 * - Position: x, y coordinates
 * - Velocity: velocityY (vertical movement only)
 * - Jumping: isJumping flag
 * - Ability: abilityActive, abilityTimer, abilityCooldown
 * - Character: headIndex (0-3 for different characters)
 * - Double Jump: canDoubleJump, hasDoubleJumped flags
 * 
 * METHODS:
 * - jump(): Initiate jump or double jump
 * - update(): Update timers and ability states
 * - activateAbility(): Start character-specific ability
 * - isInvincible(): Check shield status
 * - hasDoubleCoinBonus(): Check magnet status
 * - getPlayerSpeedMultiplier(): Get dash speed multiplier
 * - getSpeedMultiplier(): Legacy speed modifier (always 1.0)
 * 
 * USED BY:
 * - Game class (updates player state, handles input)
 * - GameWorld class (queries abilities for collision/bonuses)
 * - UIRenderer (displays player and ability status)
 * 
 * DEPENDENCIES:
 * - iostream for debug output
 */

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
