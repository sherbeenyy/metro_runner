#ifndef GAME_WORLD_H
#define GAME_WORLD_H

/**
 * GameWorld Class
 * ===============
 * 
 * PURPOSE:
 * Manages all game objects, physics, and gameplay mechanics including
 * platforms, obstacles, coins, and game difficulty progression.
 * 
 * RESPONSIBILITIES:
 * - Manage metro platforms (creation, movement, recycling)
 * - Spawn and update obstacles (ground and flying types)
 * - Spawn and update coins with collision detection
 * - Handle game speed progression (increases every 10 seconds)
 * - Detect player collisions with obstacles and platforms
 * - Track coin collection with ability bonuses
 * 
 * GAME MECHANICS:
 * - 8 metro platforms with 80px gaps, 350px width each
 * - Platform Y position: 500px from top
 * - Game speed starts at 3.0, increases by 0.5 every 10s
 * - Obstacles spawn every 2 seconds (50% flying, 50% ground)
 * - Coins spawn every 1.5 seconds at random heights
 * - Effective speed affected by player abilities
 * 
 * COLLISION DETECTION:
 * - Platform detection uses player center X position
 * - Obstacle collision with invincibility check
 * - Fall detection when player drops below platform level
 * 
 * USED BY:
 * - Game class (updates world each frame, queries collision state)
 * 
 * DEPENDENCIES:
 * - GameObject.h for entity structs (Metro, Obstacle, Coin)
 * - Player.h for player state and ability queries
 */

#include <vector>
#include <algorithm>
#include <cstdlib>
#include "GameObject.h"
#include "Player.h"
#include "Renderer2D.h"

class GameWorld {
private:
    std::vector<Metro> metros;
    std::vector<Obstacle> obstacles;
    std::vector<Coin> coins;
    
    float metroY;
    float metroGap;
    float gameSpeed;
    float speedIncreaseTimer;
    int coinsCollected;
    
    float obstacleTimer;
    float coinTimer;
    
public:
    GameWorld() : metroY(500), metroGap(80), gameSpeed(3.0f), 
        speedIncreaseTimer(0), coinsCollected(0), obstacleTimer(0), coinTimer(0) {}
    
    void init() {
        metros.clear();
        obstacles.clear();
        coins.clear();
        
        metros.push_back(Metro(0, metroY, 600));
        for (int i = 1; i < 8; i++) {
            metros.push_back(Metro(600 + (i-1) * (350 + metroGap), metroY, 350));
        }
        
        gameSpeed = 3.0f;
        speedIncreaseTimer = 0;
        coinsCollected = 0;
        obstacleTimer = 0;
        coinTimer = 0;
    }
    
    void update(float deltaTime, Player& player) {
        speedIncreaseTimer += deltaTime;
        
        if (speedIncreaseTimer >= 10.0f) {
            gameSpeed += 0.5f;
            speedIncreaseTimer = 0;
            std::cout << "Speed increased! Speed: " << gameSpeed << std::endl;
        }
        
        updateMetros(player);
        updateObstacles(deltaTime, player);
        updateCoins(deltaTime, player);
    }
    
    void updateMetros(Player& player) {
        float effectiveSpeed = gameSpeed * player.getSpeedMultiplier() * player.getPlayerSpeedMultiplier();
        
        for (auto& metro : metros) {
            metro.x -= effectiveSpeed;
            if (metro.x + metro.width < -50) {
                float maxX = -1000;
                for (const auto& m : metros) {
                    if (m.x > maxX) maxX = m.x;
                }
                metro.x = maxX + 350 + metroGap;
            }
        }
    }
    
    void updateObstacles(float deltaTime, Player& player) {
        obstacleTimer += deltaTime;
        
        if (obstacleTimer > 2.0f) {
            bool flying = rand() % 2 == 0;
            float obsX = SCREEN_WIDTH + 50;
            float obsY = flying ? metroY - 180 : metroY - 60;
            float obsH = flying ? 30 : 60;
            obstacles.push_back(Obstacle(obsX, obsY, 40, obsH, flying));
            obstacleTimer = 0;
        }
        
        float effectiveSpeed = gameSpeed * player.getSpeedMultiplier() * player.getPlayerSpeedMultiplier();
        for (auto& obs : obstacles) {
            obs.x -= effectiveSpeed;
            if (obs.x + obs.width < 0) obs.active = false;
        }
        
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), 
            [](const Obstacle& o) { return !o.active; }), obstacles.end());
    }
    
    void updateCoins(float deltaTime, Player& player) {
        coinTimer += deltaTime;
        
        if (coinTimer > 1.5f) {
            float coinY = metroY - 150 - rand() % 100;
            coins.push_back(Coin(SCREEN_WIDTH + 30, coinY));
            coinTimer = 0;
        }
        
        float effectiveSpeed = gameSpeed * player.getSpeedMultiplier() * player.getPlayerSpeedMultiplier();
        for (auto& coin : coins) {
            coin.x -= effectiveSpeed;
            
            if (!coin.collected && 
                player.x < coin.x + coin.size && player.x + player.width > coin.x &&
                player.y < coin.y + coin.size && player.y + player.height > coin.y) {
                coin.collected = true;
                int coinValue = player.hasDoubleCoinBonus() ? 2 : 1;
                coinsCollected += coinValue;
            }
        }
        
        coins.erase(std::remove_if(coins.begin(), coins.end(), 
            [](const Coin& c) { return c.x + c.size < 0 || c.collected; }), coins.end());
    }
    
    bool isPlayerOnPlatform(const Player& player) const {
        float groundY = metroY - player.height;
        
        if (player.y >= groundY - 5) {
            for (const auto& metro : metros) {
                float playerCenterX = player.x + player.width / 2;
                if (playerCenterX > metro.x && playerCenterX < metro.x + metro.width) {
                    return true;
                }
            }
        }
        return false;
    }
    
    bool checkObstacleCollision(const Player& player) const {
        if (player.isInvincible()) return false;
        
        for (const auto& obs : obstacles) {
            if (player.x < obs.x + obs.width && player.x + player.width > obs.x &&
                player.y < obs.y + obs.height && player.y + player.height > obs.y) {
                return true;
            }
        }
        return false;
    }
    
    bool checkFallThrough(const Player& player) const {
        float playerCenterX = player.x + player.width / 2;
        
        for (const auto& metro : metros) {
            if (playerCenterX > metro.x && playerCenterX < metro.x + metro.width) {
                return false;
            }
        }
        
        return player.y > metroY + 50;
    }
    
    float getGroundY(const Player& player) const {
        return metroY - player.height;
    }
    
    int getCoinsCollected() const { return coinsCollected; }
    const std::vector<Metro>& getMetros() const { return metros; }
    const std::vector<Obstacle>& getObstacles() const { return obstacles; }
    const std::vector<Coin>& getCoins() const { return coins; }
};

#endif
