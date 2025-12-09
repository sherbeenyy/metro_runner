#ifndef UI_RENDERER_H
#define UI_RENDERER_H

/**
 * UIRenderer Class
 * ================
 * 
 * PURPOSE:
 * Handles all UI rendering including menus, HUD elements, player visualization,
 * and game state screens. Separates UI logic from core game rendering.
 * 
 * RESPONSIBILITIES:
 * - Render start screen with title and instructions
 * - Render character selection screen with 4 characters
 * - Display character stats and abilities
 * - Render player as stick figure with custom head texture
 * - Render HUD (coins, ability status, music indicator, controls)
 * - Render game over screen with scores
 * 
 * SCREEN LAYOUTS:
 * 
 * START_SCREEN:
 * - Centered title "METRO RUNNER"
 * - "PRESS ANY KEY" prompt
 * 
 * CHARACTER_SELECT:
 * - 4 characters displayed horizontally
 * - Each shows: head texture, stick body, name, ability description
 * - Green highlight on selected character
 * - Names: BIG JOE, ALI S, H, AK 47
 * - Abilities: SHIELD, DOUBLE JUMP, MAGNET, DASH
 * 
 * PLAYING:
 * - Top-left: Music status (ON/OFF with M key)
 * - Top-right: Ability status (timer/cooldown/ready)
 * - Upper-left: Coin counter with icon
 * - Bottom-left: Control instructions
 * - Player rendered as stick figure with character head
 * 
 * GAME_OVER:
 * - Centered "GAME OVER" text
 * - Current coins collected
 * - Best score
 * - Pulsing "PRESS SPACE TO RESTART" prompt
 * 
 * USED BY:
 * - Game class (calls render methods based on game state)
 * 
 * DEPENDENCIES:
 * - Renderer2D for low-level drawing operations
 * - Player.h for player state information
 * - Texture.h for character head images
 */

#include <string>
#include <cmath>
#include "Renderer2D.h"
#include "Player.h"
#include "Texture.h"

class UIRenderer {
private:
    Renderer2D& renderer;
    
public:
    UIRenderer(Renderer2D& rend) : renderer(rend) {}
    
    void renderStartScreen(Texture* bgTexture) {
        renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bgTexture);
        renderer.drawQuad(SCREEN_WIDTH/2 - 250, 100, 500, 100, nullptr, 0, 0, 0, 0.8);
        renderer.drawText("METRO RUNNER", SCREEN_WIDTH/2 - 200, 120, 50, 1, 1, 0);
        renderer.drawText("PRESS ANY KEY", SCREEN_WIDTH/2 - 200, 250, 40, 0, 1, 0);
    }
    
    void renderCharacterSelect(Texture* bgTexture, Texture* playerHeads[], int selectedChar) {
        renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bgTexture, 1, 1, 1, 0.3);
        
        for (int i = 0; i < 4; i++) {
            renderCharacter(i, selectedChar, playerHeads);
        }
        
        renderer.drawQuad(SCREEN_WIDTH/2 - 300, 50, 600, 100, nullptr, 0, 0, 0, 0.8);
        renderer.drawText("SELECT CHARACTER", SCREEN_WIDTH/2 - 220, 70, 40, 1, 1, 0);
        renderer.drawQuad(SCREEN_WIDTH/2 - 300, 650, 600, 80, nullptr, 0.2, 0.8, 0.2, 0.9);
        renderer.drawText("LEFT RIGHT ARROWS - SPACE TO START", SCREEN_WIDTH/2 - 350, 675, 30, 1, 1, 1);
    }
    
    void renderCharacter(int i, int selectedChar, Texture* playerHeads[]) {
        float charX = 200 + i * 250;
        float charY = SCREEN_HEIGHT/2 - 50;
        bool selected = (i == selectedChar);
        float headSize = 40;
        
        if (selected) {
            renderer.drawQuad(charX - 70, charY - 30, 140, 240, nullptr, 0, 1, 0, 0.3);
        }
        
        // Head
        renderer.drawQuad(charX - headSize/2, charY, headSize, headSize, playerHeads[i]);
        
        // Body
        renderer.drawQuad(charX - 2, charY + headSize, 4, 50, nullptr, 0, 0, 0);
        
        // Arms
        renderer.drawQuad(charX - 30, charY + headSize + 15, 60, 4, nullptr, 0, 0, 0);
        
        // Legs
        renderer.drawQuad(charX - 15, charY + headSize + 50, 4, 40, nullptr, 0, 0, 0);
        renderer.drawQuad(charX + 11, charY + headSize + 50, 4, 40, nullptr, 0, 0, 0);
        
        // Names
        const char* names[] = {"BIG JOE", "ALI S", "H", "AK 47"};
        int nameOffsets[] = {-60, -80, -45, -45};
        renderer.drawText(names[i], charX + nameOffsets[i], charY - 60, 25, 1, 1, 0);
        
        // Abilities
        const char* abilities[] = {"SHIELD", "DOUBLE JUMP", "MAGNET", "DASH"};
        const char* descriptions[] = {"5S INVINCIBLE", "8S AIR JUMP", "6S 2X COINS", "5S FAST RUN"};
        int abilityOffsets[] = {-55, -90, -50, -35};
        int descOffsets[] = {-100, -85, -85, -85};
        
        renderer.drawText(abilities[i], charX + abilityOffsets[i], charY + 170, 22, 0, 0.8, 1);
        renderer.drawText(descriptions[i], charX + descOffsets[i], charY + 195, 18, 0.8, 0.8, 0.8);
    }
    
    void renderPlayer(const Player& player, Texture* playerHeads[]) {
        float px = player.x;
        float py = player.y;
        float headSize = 25;
        float bodyLength = 35;
        float armLength = 30;
        float legLength = 35;
        float totalHeight = headSize + bodyLength + legLength;
        float stickY = py - totalHeight + player.height;
        
        // Head
        renderer.drawQuad(px + player.width/2 - headSize/2, stickY, headSize, headSize, playerHeads[player.headIndex]);
        
        // Body
        renderer.drawQuad(px + player.width/2 - 2, stickY + headSize, 4, bodyLength, nullptr, 0, 0, 0);
        
        // Arms
        renderer.drawQuad(px + player.width/2 - armLength/2, stickY + headSize + 15, armLength, 4, nullptr, 0, 0, 0);
        
        // Legs
        renderer.drawQuad(px + player.width/2 - 10, stickY + headSize + bodyLength, 3, legLength, nullptr, 0, 0, 0);
        renderer.drawQuad(px + player.width/2 + 7, stickY + headSize + bodyLength, 3, legLength, nullptr, 0, 0, 0);
    }
    
    void renderHUD(const Player& player, int coinsCollected, bool musicMuted) {
        // Music indicator
        renderer.drawQuad(10, 10, 180, 40, nullptr, 0, 0, 0, 0.7);
        if (musicMuted) {
            renderer.drawText("MUSIC OFF M", 20, 20, 20, 1, 0, 0);
        } else {
            renderer.drawText("MUSIC ON M", 20, 20, 20, 0, 1, 0);
        }
        
        // Ability status
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
        
        // Coin counter
        renderer.drawQuad(20, 20, 250, 70, nullptr, 0, 0, 0, 0.7);
        renderer.drawQuad(30, 30, 40, 40, nullptr, 1, 0.84, 0);
        renderer.drawText("COINS " + std::to_string(coinsCollected), 80, 40, 30, 1, 1, 0);
        
        // Controls
        renderer.drawQuad(20, 700, 350, 80, nullptr, 0, 0, 0, 0.6);
        renderer.drawText("UP W SPACE TO JUMP", 30, 720, 25, 1, 1, 1);
    }
    
    void renderGameOver(int coinsCollected, int bestScore, float currentTime) {
        renderer.drawQuad(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, 0, 0, 0, 0.7);
        renderer.drawQuad(SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 - 250, 600, 500, nullptr, 0.2, 0.2, 0.3, 0.95);
        
        renderer.drawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 220, 50, 1, 0, 0);
        
        renderer.drawQuad(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 100, 500, 80, nullptr, 0.9, 0.9, 0.3, 0.8);
        renderer.drawText("COINS " + std::to_string(coinsCollected), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 75, 40, 0, 0, 0);
        
        renderer.drawQuad(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 500, 70, nullptr, 0.3, 0.9, 0.3, 0.8);
        renderer.drawText("BEST " + std::to_string(bestScore), SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 20, 40, 0, 0, 0);
        
        float pulse = 0.5f + 0.3f * sin(currentTime * 5);
        renderer.drawQuad(SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 100, 400, 70, nullptr, 0.2, pulse, 0.2);
        renderer.drawText("PRESS SPACE TO RESTART", SCREEN_WIDTH/2 - 190, SCREEN_HEIGHT/2 + 120, 30, 1, 1, 1);
    }
};

#endif
