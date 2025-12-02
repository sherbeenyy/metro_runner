#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// Simple JSON-like data structure for game persistence
struct GameData {
    int bestScore;
    int totalCoins;
    int selectedCharacter;
    
    GameData() : bestScore(0), totalCoins(0), selectedCharacter(0) {}
};

class ScoreManager {
private:
    std::string filename;
    GameData data;
    
    void parseJSON(const std::string& content) {
        // Simple JSON parser for our specific format
        size_t pos = 0;
        
        // Find bestScore
        pos = content.find("\"bestScore\":");
        if (pos != std::string::npos) {
            pos += 12;
            data.bestScore = std::stoi(content.substr(pos));
        }
        
        // Find totalCoins
        pos = content.find("\"totalCoins\":");
        if (pos != std::string::npos) {
            pos += 13;
            data.totalCoins = std::stoi(content.substr(pos));
        }
        
        // Find selectedCharacter
        pos = content.find("\"selectedCharacter\":");
        if (pos != std::string::npos) {
            pos += 20;
            data.selectedCharacter = std::stoi(content.substr(pos));
        }
    }
    
    std::string toJSON() {
        return "{\n"
               "  \"bestScore\": " + std::to_string(data.bestScore) + ",\n"
               "  \"totalCoins\": " + std::to_string(data.totalCoins) + ",\n"
               "  \"selectedCharacter\": " + std::to_string(data.selectedCharacter) + "\n"
               "}";
    }
    
public:
    ScoreManager(const std::string& file = "gamedata.json") : filename(file) {
        load();
    }
    
    void load() {
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            std::string content((std::istreambuf_iterator<char>(inFile)),
                               std::istreambuf_iterator<char>());
            parseJSON(content);
            inFile.close();
            std::cout << "Loaded game data: Best Score = " << data.bestScore 
                     << ", Total Coins = " << data.totalCoins << std::endl;
        } else {
            std::cout << "No save file found, starting fresh" << std::endl;
        }
    }
    
    void save() {
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << toJSON();
            outFile.close();
            std::cout << "Game data saved!" << std::endl;
        }
    }
    
    void updateBestScore(int score) {
        if (score > data.bestScore) {
            data.bestScore = score;
            save();
        }
    }
    
    void addCoins(int coins) {
        data.totalCoins += coins;
        save();
    }
    
    void setSelectedCharacter(int character) {
        data.selectedCharacter = character;
        save();
    }
    
    int getBestScore() const { return data.bestScore; }
    int getTotalCoins() const { return data.totalCoins; }
    int getSelectedCharacter() const { return data.selectedCharacter; }
};
