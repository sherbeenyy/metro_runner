#ifndef GAME_H
#define GAME_H

/**
 * Game Class (Main Coordinator)
 * ==============================
 * 
 * PURPOSE:
 * Central coordinator that orchestrates all game systems. Implements the main
 * game loop and manages transitions between different game states.
 * 
 * ARCHITECTURE:
 * This is the top-level class that integrates all specialized subsystems:
 * 
 * 1. InputManager - Handles keyboard input
 * 2. AssetManager - Manages textures and audio
 * 3. GameWorld - Updates game objects and physics
 * 4. UIRenderer - Renders all UI elements
 * 5. ScoreManager - Persists high scores
 * 6. Player - Player state and abilities
 * 7. Renderer2D - Low-level OpenGL rendering
 * 
 * GAME STATES:
 * - START_SCREEN: Initial menu, waits for any key
 * - CHARACTER_SELECT: Choose from 4 characters with abilities
 * - PLAYING: Active gameplay with physics and collisions
 * - GAME_OVER: Show scores, allow restart
 * 
 * MAIN LOOP FLOW:
 * 1. handleInput() - Process keyboard via InputManager
 * 2. update(deltaTime) - Update game state via GameWorld and Player
 * 3. render(currentTime) - Draw everything via UIRenderer
 * 
 * INITIALIZATION SEQUENCE:
 * 1. Init GLFW and create window (1280x720)
 * 2. Load OpenGL via GLAD
 * 3. Create InputManager with window pointer
 * 4. Create UIRenderer with Renderer2D
 * 5. Load all assets via AssetManager
 * 6. Initialize GameWorld with platforms
 * 7. Position player at ground level
 * 
 * STATE TRANSITIONS:
 * START_SCREEN -> CHARACTER_SELECT: Any key press
 * CHARACTER_SELECT -> PLAYING: Space after selecting character
 * PLAYING -> GAME_OVER: Collision or fall detected
 * GAME_OVER -> CHARACTER_SELECT: Space to restart
 * 
 * COORDINATE SYSTEM:
 * - Origin (0,0) at top-left
 * - Screen size: 1280x720 pixels
 * - Y increases downward
 * 
 * DEPENDENCIES:
 * - GLFW for window and input
 * - GLAD for OpenGL loading
 * - All subsystem classes listed above
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "InputManager.h"
#include "AssetManager.h"
#include "GameWorld.h"
#include "UIRenderer.h"
#include "GameData.h"
#include "Player.h"

enum class GameState {
    START_SCREEN,
    CHARACTER_SELECT,
    PLAYING,
    GAME_OVER
};

class Game {
private:
    GLFWwindow* window;
    Renderer2D renderer;
    
    InputManager* inputManager;
    AssetManager assetManager;
    GameWorld gameWorld;
    UIRenderer* uiRenderer;
    ScoreManager scoreManager;
    
    GameState state;
    Player player;
    int selectedChar;
    float lastTime;
    
public:
    Game() : inputManager(nullptr), uiRenderer(nullptr), 
             state(GameState::START_SCREEN), selectedChar(0) {
        srand((unsigned)time(0));
    }
    
    ~Game() {
        delete inputManager;
        delete uiRenderer;
    }
    
    bool init() {
        if (!glfwInit()) {
            std::cerr << "Failed to init GLFW\n";
            return false;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Metro Runner", NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create window\n";
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to init GLAD\n";
            return false;
        }
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        inputManager = new InputManager(window);
        uiRenderer = new UIRenderer(renderer);
        
        if (!assetManager.loadAssets()) {
            return false;
        }
        
        gameWorld.init();
        player.y = gameWorld.getGroundY(player);
        lastTime = (float)glfwGetTime();
        
        std::cout << "=== METRO RUNNER ===" << std::endl;
        std::cout << "Press ANY KEY to start!" << std::endl;
        
        return true;
    }
    
    void run() {
        while (!glfwWindowShouldClose(window)) {
            float currentTime = (float)glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            
            handleInput();
            update(deltaTime);
            render(currentTime);
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    
    void handleInput() {
        if (inputManager->isEscapePressed()) {
            glfwSetWindowShouldClose(window, true);
        }
        
        if (inputManager->isMutePressed()) {
            assetManager.toggleMusic();
        }
        
        switch (state) {
            case GameState::START_SCREEN:
                if (inputManager->isAnyKeyPressed()) {
                    state = GameState::CHARACTER_SELECT;
                }
                break;
                
            case GameState::CHARACTER_SELECT:
                if (inputManager->isLeftPressed()) {
                    selectedChar = (selectedChar - 1 + 4) % 4;
                }
                if (inputManager->isRightPressed()) {
                    selectedChar = (selectedChar + 1) % 4;
                }
                if (inputManager->isSpacePressed()) {
                    startGame();
                }
                break;
                
            case GameState::PLAYING:
                if (inputManager->isJumpPressed()) {
                    player.jump();
                }
                if (inputManager->isAbilityPressed()) {
                    player.activateAbility();
                }
                break;
                
            case GameState::GAME_OVER:
                if (inputManager->isSpacePressed()) {
                    state = GameState::CHARACTER_SELECT;
                }
                break;
        }
    }
    
    void startGame() {
        player = Player();
        player.headIndex = selectedChar;
        player.y = gameWorld.getGroundY(player);
        state = GameState::PLAYING;
        gameWorld.init();
    }
    
    void update(float deltaTime) {
        if (state != GameState::PLAYING) return;
        
        updatePlayer(deltaTime);
        gameWorld.update(deltaTime, player);
        checkCollisions();
    }
    
    void updatePlayer(float deltaTime) {
        bool standingOnPlatform = gameWorld.isPlayerOnPlatform(player);
        float groundY = gameWorld.getGroundY(player);
        
        if (standingOnPlatform && !player.isJumping) {
            player.update(groundY, deltaTime);
        } else {
            if (!player.isJumping && player.y >= groundY) {
                player.isJumping = true;
                player.velocityY = 1.0f;
            }
            player.y += player.velocityY;
            player.velocityY += 0.5f;
            
            if (player.y >= groundY && standingOnPlatform) {
                player.y = groundY;
                player.velocityY = 0;
                player.isJumping = false;
                player.hasDoubleJumped = false;
            }
        }
    }
    
    void checkCollisions() {
        if (gameWorld.checkObstacleCollision(player) || gameWorld.checkFallThrough(player)) {
            endGame();
        }
    }
    
    void endGame() {
        state = GameState::GAME_OVER;
        int coins = gameWorld.getCoinsCollected();
        scoreManager.updateBestScore(coins);
        scoreManager.addCoins(coins);
        std::cout << "\n=== GAME OVER ===" << std::endl;
        std::cout << "Coins: " << coins << std::endl;
        std::cout << "Best: " << scoreManager.getBestScore() << std::endl;
    }
    
    void render(float currentTime) {
        glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        switch (state) {
            case GameState::START_SCREEN:
                uiRenderer->renderStartScreen(assetManager.getBackgroundTexture());
                break;
                
            case GameState::CHARACTER_SELECT: {
                Texture* heads[4] = {
                    assetManager.getPlayerHead(0),
                    assetManager.getPlayerHead(1),
                    assetManager.getPlayerHead(2),
                    assetManager.getPlayerHead(3)
                };
                uiRenderer->renderCharacterSelect(assetManager.getBackgroundTexture(), heads, selectedChar);
                break;
            }
                
            case GameState::PLAYING:
                renderPlaying();
                break;
                
            case GameState::GAME_OVER:
                uiRenderer->renderGameOver(gameWorld.getCoinsCollected(), 
                                          scoreManager.getBestScore(), currentTime);
                break;
        }
    }
    
    void renderPlaying() {
        renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, 0.7, 0.85, 0.95);
        
        // Render metros
        for (const auto& metro : gameWorld.getMetros()) {
            renderer.drawQuad(metro.x, metro.y, metro.width, 100, assetManager.getMetroTexture());
        }
        
        // Render obstacles
        for (const auto& obs : gameWorld.getObstacles()) {
            float r = obs.isFlying ? 1.0f : 0.8f;
            float g = obs.isFlying ? 0.5f : 0.2f;
            renderer.drawQuad(obs.x, obs.y, obs.width, obs.height, nullptr, r, g, 0.2f);
        }
        
        // Render coins
        for (const auto& coin : gameWorld.getCoins()) {
            renderer.drawQuad(coin.x, coin.y, coin.size, coin.size, nullptr, 1, 0.84, 0);
        }
        
        // Render player
        Texture* heads[4] = {
            assetManager.getPlayerHead(0),
            assetManager.getPlayerHead(1),
            assetManager.getPlayerHead(2),
            assetManager.getPlayerHead(3)
        };
        uiRenderer->renderPlayer(player, heads);
        
        // Render HUD
        uiRenderer->renderHUD(player, gameWorld.getCoinsCollected(), assetManager.isMusicMuted());
    }
    
    void cleanup() {
        glfwTerminate();
    }
};

#endif
