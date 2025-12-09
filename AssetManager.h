#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

/**
 * AssetManager Class
 * ==================
 * 
 * PURPOSE:
 * Centralized management of all game assets including textures and audio.
 * Loads resources once and provides access throughout the game lifecycle.
 * 
 * RESPONSIBILITIES:
 * - Load all texture files (backgrounds, metro platforms, character heads)
 * - Load and control background music
 * - Provide texture access via getter methods
 * - Handle music playback state (play/pause/mute)
 * 
 * MANAGED ASSETS:
 * - Background texture (metro_background.jpg)
 * - Metro platform texture (metro_side_view.PNG)
 * - 4 player head textures (p1.PNG, p2.PNG, p3.PNG, P4.PNG)
 * - Background music (song file)
 * 
 * USED BY:
 * - Game class (initializes assets, provides textures to renderers)
 * - UIRenderer (gets textures for rendering)
 * 
 * DEPENDENCIES:
 * - Texture class for image loading
 * - SFML Audio for music playback
 */

#include <SFML/Audio.hpp>
#include <iostream>
#include "Texture.h"

class AssetManager {
private:
    Texture bgTexture;
    Texture metroTexture;
    Texture playerHeads[4];
    sf::Music music;
    bool musicMuted;
    
public:
    AssetManager() : musicMuted(false) {}
    
    bool loadAssets() {
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
        
        if (!music.openFromFile("song")) {
            std::cerr << "Warning: Failed to load song\n";
        } else {
            music.setLoop(true);
            music.play();
            std::cout << "Background music started! Press M to mute/unmute" << std::endl;
        }
        
        return true;
    }
    
    Texture* getBackgroundTexture() { return &bgTexture; }
    Texture* getMetroTexture() { return &metroTexture; }
    Texture* getPlayerHead(int index) { return &playerHeads[index]; }
    
    void toggleMusic() {
        musicMuted = !musicMuted;
        if (musicMuted) {
            music.pause();
            std::cout << "Music muted" << std::endl;
        } else {
            music.play();
            std::cout << "Music unmuted" << std::endl;
        }
    }
    
    bool isMusicMuted() const { return musicMuted; }
};

#endif
