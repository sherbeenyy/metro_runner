#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>
#include <cmath>
#include "Texture.h"
#include "GameData.h"
#include "Renderer2D.h"
#include "Player.h"
#include "GameObject.h"

enum class GameState {
    START_SCREEN,
    CHARACTER_SELECT,
    PLAYING,
    GAME_OVER
};

int main() {
    srand((unsigned)time(0));
    
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Metro Runner", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Load textures
    Texture bgTexture, metroTexture;
    Texture playerHeads[4];
    
    if (!bgTexture.load("imgs/metro_background.jpg")) {
        std::cerr << "Warning: failed to load imgs/metro_background.jpg\n";
    }
    if (!metroTexture.load("imgs/metro_side_view.PNG")) {
        std::cerr << "Warning: failed to load imgs/metro_side_view.PNG\n";
    }
    if (!playerHeads[0].load("imgs/players/p1.PNG")) {
        std::cerr << "Warning: failed to load imgs/players/p1.PNG\n";
    }
    if (!playerHeads[1].load("imgs/players/p2.PNG")) {
        std::cerr << "Warning: failed to load imgs/players/p2.PNG\n";
    }
    if (!playerHeads[2].load("imgs/players/p3.PNG")) {
        std::cerr << "Warning: failed to load imgs/players/p3.PNG\n";
    }
    if (!playerHeads[3].load("imgs/players/P4.PNG")) {
        std::cerr << "Warning: failed to load imgs/players/P4.PNG\n";
    }
    
    // Load and play background music
    sf::Music music;
    if (!music.openFromFile("song")) {
        std::cerr << "Warning: Failed to load song\n";
    } else {
        music.setLoop(true);  // Loop the music continuously
        music.play();
        std::cout << "Background music started!" << std::endl;
    }
    
    Renderer2D renderer;
    ScoreManager scoreManager;
    
    GameState state = GameState::START_SCREEN;
    Player player;
    int selectedChar = 0;
    
    std::vector<Metro> metros;
    std::vector<Obstacle> obstacles;
    std::vector<Coin> coins;
    
    float gameSpeed = 3.0f;
    float speedIncreaseTimer = 0;
    int coinsCollected = 0;
    float gameTime = 0;
    
    // Initial metro platforms - first one is long for safe start
    float metroY = 500;
    float metroGap = 80;  // Reduced from 150 for easier jumping
    metros.push_back(Metro(0, metroY, 600));  // Long starting metro
    for (int i = 1; i < 8; i++) {
        metros.push_back(Metro(600 + (i-1) * (350 + metroGap), metroY, 350));  // Wider metros
    }
    
    float lastTime = (float)glfwGetTime();
    
    std::cout << "=== METRO RUNNER ===" << std::endl;
    std::cout << "Press ANY KEY to start!" << std::endl;
    
    player.y = metroY - player.height;
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // State machine
        if (state == GameState::START_SCREEN) {
            static bool anyKeyPressed = false;
            bool keyPressed = false;
            for (int key = 32; key <= 348; key++) {
                if (glfwGetKey(window, key) == GLFW_PRESS) {
                    keyPressed = true;
                    if (!anyKeyPressed) {
                        state = GameState::CHARACTER_SELECT;
                        anyKeyPressed = true;
                    }
                    break;
                }
            }
            if (!keyPressed) {
                anyKeyPressed = false;
            }
        }
        else if (state == GameState::CHARACTER_SELECT) {
            static bool leftPressed = false, rightPressed = false, spacePressed = false;
            
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !leftPressed) {
                selectedChar = (selectedChar - 1 + 4) % 4;
                leftPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) leftPressed = false;
            
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !rightPressed) {
                selectedChar = (selectedChar + 1) % 4;
                rightPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) rightPressed = false;
            
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
                player.headIndex = selectedChar;
                state = GameState::PLAYING;
                spacePressed = true;
                
                // Reset game
                player = Player();
                player.headIndex = selectedChar;
                player.y = metroY - player.height;
                gameSpeed = 3.0f;
                coinsCollected = 0;
                gameTime = 0;
                metros.clear();
                obstacles.clear();
                coins.clear();
                metros.push_back(Metro(0, metroY, 600));
                for (int i = 1; i < 8; i++) {
                    metros.push_back(Metro(600 + (i-1) * (350 + metroGap), metroY, 350));
                }
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) spacePressed = false;
        }
        else if (state == GameState::PLAYING) {
            gameTime += deltaTime;
            speedIncreaseTimer += deltaTime;
            
            if (speedIncreaseTimer >= 10.0f) {
                gameSpeed += 0.5f;
                speedIncreaseTimer = 0;
                std::cout << "Speed increased! Speed: " << gameSpeed << std::endl;
            }
            
            // Input - jump and ability
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || 
                glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                player.jump();
            }
            
            static bool qPressed = false;
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !qPressed) {
                player.activateAbility();
                qPressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
                qPressed = false;
            }
            
            // Check if player is on a platform (check feet position)
            bool standingOnPlatform = false;
            float groundY = metroY - player.height;
            
            // Only check if player is at ground level (not jumping high above)
            if (player.y >= groundY - 5) {
                for (const auto& metro : metros) {
                    // Check if player's center is over the platform
                    float playerCenterX = player.x + player.width / 2;
                    if (playerCenterX > metro.x && playerCenterX < metro.x + metro.width) {
                        standingOnPlatform = true;
                        break;
                    }
                }
            }
            
            // Update player physics
            if (standingOnPlatform && !player.isJumping) {
                player.update(groundY, deltaTime);
            } else {
                // Player is in air (jumping or falling)
                if (!player.isJumping && player.y >= groundY) {
                    // Not on platform and at ground level = start falling
                    player.isJumping = true;
                    player.velocityY = 1.0f;
                }
                player.y += player.velocityY;
                player.velocityY += 0.5f;  // Gravity
                
                // Check if landed back on platform
                if (player.y >= groundY && standingOnPlatform) {
                    player.y = groundY;
                    player.velocityY = 0;
                    player.isJumping = false;
                    player.hasDoubleJumped = false;
                }
            }
            
            // Move metros (with speed multiplier from ability)
            float effectiveSpeed = gameSpeed * player.getSpeedMultiplier() * player.getPlayerSpeedMultiplier();
            for (auto& metro : metros) {
                metro.x -= effectiveSpeed;
                if (metro.x + metro.width < -50) {
                    float maxX = -1000;
                    for (const auto& m : metros) {
                        if (m.x > maxX) maxX = m.x;
                    }
                    metro.x = maxX + 350 + metroGap;  // Match new metro width
                }
            }
            
            // Spawn obstacles
            static float obstacleTimer = 0;
            obstacleTimer += deltaTime;
            if (obstacleTimer > 2.0f) {
                bool flying = rand() % 2 == 0;
                float obsX = SCREEN_WIDTH + 50;
                float obsY = flying ? metroY - 180 : metroY - 60;
                float obsH = flying ? 30 : 60;
                obstacles.push_back(Obstacle(obsX, obsY, 40, obsH, flying));
                obstacleTimer = 0;
            }
            
            // Update obstacles
            for (auto& obs : obstacles) {
                obs.x -= effectiveSpeed;
                if (obs.x + obs.width < 0) obs.active = false;
            }
            obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), 
                [](const Obstacle& o) { return !o.active; }), obstacles.end());
            
            // Spawn coins
            static float coinTimer = 0;
            coinTimer += deltaTime;
            if (coinTimer > 1.5f) {
                float coinY = metroY - 150 - rand() % 100;
                coins.push_back(Coin(SCREEN_WIDTH + 30, coinY));
                coinTimer = 0;
            }
            
            // Update coins
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
            
            // Check collisions with obstacles (invincibility check)
            if (!player.isInvincible()) {
                for (const auto& obs : obstacles) {
                    if (player.x < obs.x + obs.width && player.x + player.width > obs.x &&
                        player.y < obs.y + obs.height && player.y + player.height > obs.y) {
                        state = GameState::GAME_OVER;
                        scoreManager.updateBestScore(coinsCollected);
                        scoreManager.addCoins(coinsCollected);
                        std::cout << "\n=== GAME OVER ===" << std::endl;
                        std::cout << "Coins: " << coinsCollected << std::endl;
                        std::cout << "Best: " << scoreManager.getBestScore() << std::endl;
                    }
                }
            }
            
            // Check if fell through gap (player center must be over a platform)
            bool onPlatform = false;
            float playerCenterX = player.x + player.width / 2;
            for (const auto& metro : metros) {
                if (playerCenterX > metro.x && playerCenterX < metro.x + metro.width) {
                    onPlatform = true;
                    break;
                }
            }
            
            // Game over if fell below platform level
            if (!onPlatform && player.y > metroY + 50) {
                state = GameState::GAME_OVER;
                scoreManager.updateBestScore(coinsCollected);
                scoreManager.addCoins(coinsCollected);
                std::cout << "\n=== GAME OVER - Fell! ===" << std::endl;
                std::cout << "Coins: " << coinsCollected << std::endl;
                std::cout << "Best: " << scoreManager.getBestScore() << std::endl;
            }
        }
        else if (state == GameState::GAME_OVER) {
            static bool spacePressed = false;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
                state = GameState::CHARACTER_SELECT;
                spacePressed = true;
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) spacePressed = false;
        }
        
        // Rendering
        glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if (state == GameState::START_SCREEN) {
            renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, &bgTexture);
            renderer.drawQuad(SCREEN_WIDTH/2 - 250, 100, 500, 100, nullptr, 0, 0, 0, 0.8);
            renderer.drawText("METRO RUNNER", SCREEN_WIDTH/2 - 200, 120, 50, 1, 1, 0);
            renderer.drawText("PRESS ANY KEY", SCREEN_WIDTH/2 - 200, 250, 40, 0, 1, 0);
        }
        else if (state == GameState::CHARACTER_SELECT) {
            renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, &bgTexture, 1, 1, 1, 0.3);
            
            for (int i = 0; i < 4; i++) {
                float charX = 200 + i * 250;
                float charY = SCREEN_HEIGHT/2 - 50;
                bool selected = (i == selectedChar);
                float headSize = 40;
                
                if (selected) {
                    renderer.drawQuad(charX - 70, charY - 30, 140, 240, nullptr, 0, 1, 0, 0.3);
                }
                
                // Head
                renderer.drawQuad(charX - headSize/2, charY, headSize, headSize, &playerHeads[i]);
                
                // Body (vertical line)
                renderer.drawQuad(charX - 2, charY + headSize, 4, 50, nullptr, 0, 0, 0);
                
                // Arms (horizontal line)
                renderer.drawQuad(charX - 30, charY + headSize + 15, 60, 4, nullptr, 0, 0, 0);
                
                // Left leg
                renderer.drawQuad(charX - 15, charY + headSize + 50, 4, 40, nullptr, 0, 0, 0);
                
                // Right leg
                renderer.drawQuad(charX + 11, charY + headSize + 50, 4, 40, nullptr, 0, 0, 0);
                
                // Player names above characters
                if (i == 0) {
                    renderer.drawText("BIG JOE", charX - 60, charY - 60, 25, 1, 1, 0);
                } else if (i == 1) {
                    renderer.drawText("ALI S", charX - 80, charY - 60, 25, 1, 1, 0);
                } else if (i == 2) {
                    renderer.drawText("H", charX - 45, charY - 60, 25, 1, 1, 0);
                } else if (i == 3) {
                    renderer.drawText("AK 47", charX - 45, charY - 60, 25, 1, 1, 0);
                }
                
                // Ability descriptions
                if (i == 0) {
                    renderer.drawText("SHIELD", charX - 55, charY + 170, 22, 0, 0.8, 1);
                    renderer.drawText("5S INVINCIBLE", charX - 100, charY + 195, 18, 0.8, 0.8, 0.8);
                } else if (i == 1) {
                    renderer.drawText("DOUBLE JUMP", charX - 90, charY + 170, 22, 0, 0.8, 1);
                    renderer.drawText("8S AIR JUMP", charX - 85, charY + 195, 18, 0.8, 0.8, 0.8);
                } else if (i == 2) {
                    renderer.drawText("MAGNET", charX - 50, charY + 170, 22, 0, 0.8, 1);
                    renderer.drawText("6S 2X COINS", charX - 85, charY + 195, 18, 0.8, 0.8, 0.8);
                } else if (i == 3) {
                    renderer.drawText("DASH", charX - 35, charY + 170, 22, 0, 0.8, 1);
                    renderer.drawText("5S FAST RUN", charX - 85, charY + 195, 18, 0.8, 0.8, 0.8);
                }
            }
            
            renderer.drawQuad(SCREEN_WIDTH/2 - 300, 50, 600, 100, nullptr, 0, 0, 0, 0.8);
            renderer.drawText("SELECT CHARACTER", SCREEN_WIDTH/2 - 220, 70, 40, 1, 1, 0);
            renderer.drawQuad(SCREEN_WIDTH/2 - 300, 650, 600, 80, nullptr, 0.2, 0.8, 0.2, 0.9);
            renderer.drawText("LEFT RIGHT ARROWS - SPACE TO START", SCREEN_WIDTH/2 - 350, 675, 30, 1, 1, 1);
        }
        else if (state == GameState::PLAYING) {
            renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, 0.7, 0.85, 0.95);
            
            for (const auto& metro : metros) {
                renderer.drawQuad(metro.x, metro.y, metro.width, 100, &metroTexture);
            }
            
            for (const auto& obs : obstacles) {
                float r = obs.isFlying ? 1.0f : 0.8f;
                float g = obs.isFlying ? 0.5f : 0.2f;
                float b = 0.2f;
                renderer.drawQuad(obs.x, obs.y, obs.width, obs.height, nullptr, r, g, b);
            }
            
            for (const auto& coin : coins) {
                renderer.drawQuad(coin.x, coin.y, coin.size, coin.size, nullptr, 1, 0.84, 0);
            }
            
            // Ability status UI in top-right corner
            renderer.drawQuad(SCREEN_WIDTH - 210, 10, 200, 80, nullptr, 0, 0, 0, 0.7);
            renderer.drawText("Q ABILITY", SCREEN_WIDTH - 190, 20, 25, 1, 1, 0);
            
            if (player.abilityActive) {
                int timeLeft = (int)player.abilityTimer + 1;
                std::string timerText = "ACTIVE " + std::to_string(timeLeft) + "S";
                renderer.drawText(timerText.c_str(), SCREEN_WIDTH - 190, 50, 25, 0, 1, 0);
            } else if (player.abilityCooldown > 0) {
                int cooldown = (int)player.abilityCooldown + 1;
                std::string coolText = "COOLDOWN " + std::to_string(cooldown) + "S";
                renderer.drawText(coolText.c_str(), SCREEN_WIDTH - 200, 50, 20, 1, 0.5, 0);
            } else {
                renderer.drawText("READY", SCREEN_WIDTH - 170, 50, 25, 0, 1, 0);
            }
            
            // Player - black stickman with head image
            float px = player.x;
            float py = player.y;
            float headSize = 25;
            
            // Calculate stickman dimensions
            float bodyLength = 35;
            float armLength = 30;
            float legLength = 35;
            float totalHeight = headSize + bodyLength + legLength;
            
            // Adjust Y so feet are at the correct position
            float stickY = py - totalHeight + player.height;
            
            // Head with image
            renderer.drawQuad(px + player.width/2 - headSize/2, stickY, headSize, headSize, &playerHeads[player.headIndex]);
            
            // Body (vertical line)
            renderer.drawQuad(px + player.width/2 - 2, stickY + headSize, 4, bodyLength, nullptr, 0, 0, 0);
            
            // Arms (horizontal line)
            renderer.drawQuad(px + player.width/2 - armLength/2, stickY + headSize + 15, armLength, 4, nullptr, 0, 0, 0);
            
            // Left leg (angled outward)
            renderer.drawQuad(px + player.width/2 - 10, stickY + headSize + bodyLength, 3, legLength, nullptr, 0, 0, 0);
            
            // Right leg (angled outward)
            renderer.drawQuad(px + player.width/2 + 7, stickY + headSize + bodyLength, 3, legLength, nullptr, 0, 0, 0);
            
            // HUD
            renderer.drawQuad(20, 20, 250, 70, nullptr, 0, 0, 0, 0.7);
            renderer.drawQuad(30, 30, 40, 40, nullptr, 1, 0.84, 0);
            renderer.drawText("COINS " + std::to_string(coinsCollected), 80, 40, 30, 1, 1, 0);
            
            renderer.drawQuad(20, 700, 350, 80, nullptr, 0, 0, 0, 0.6);
            renderer.drawText("UP W SPACE TO JUMP", 30, 720, 25, 1, 1, 1);
        }
        else if (state == GameState::GAME_OVER) {
            renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, 0, 0, 0, 0.7);
            renderer.drawQuad(SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 - 250, 600, 500, nullptr, 0.2, 0.2, 0.3, 0.95);
            
            renderer.drawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 220, 50, 1, 0, 0);
            
            renderer.drawQuad(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 100, 500, 80, nullptr, 0.9, 0.9, 0.3, 0.8);
            renderer.drawText("COINS " + std::to_string(coinsCollected), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 75, 40, 0, 0, 0);
            
            renderer.drawQuad(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 500, 70, nullptr, 0.3, 0.9, 0.3, 0.8);
            renderer.drawText("BEST " + std::to_string(scoreManager.getBestScore()), SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 20, 40, 0, 0, 0);
            
            float pulse = 0.5f + 0.3f * sin(currentTime * 5);
            renderer.drawQuad(SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 100, 400, 70, nullptr, 0.2, pulse, 0.2);
            renderer.drawText("PRESS SPACE TO RESTART", SCREEN_WIDTH/2 - 190, SCREEN_HEIGHT/2 + 120, 30, 1, 1, 1);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
