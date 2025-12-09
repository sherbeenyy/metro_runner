#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

/**
 * InputManager Class
 * ==================
 * 
 * PURPOSE:
 * Handles all keyboard input for the game with proper debouncing to prevent
 * multiple triggers from a single key press.
 * 
 * RESPONSIBILITIES:
 * - Detect key presses from GLFW window
 * - Implement debouncing logic for toggle actions
 * - Provide clean boolean interface for game state queries
 * 
 * KEY FEATURES:
 * - Debouncing flags prevent key repeat issues
 * - Separate methods for different input types (jump, ability, navigation)
 * - Works with GLFW window pointer
 * 
 * USED BY:
 * - Game class (main game loop calls input methods each frame)
 * 
 * DEPENDENCIES:
 * - GLFW for keyboard input
 */

#include <GLFW/glfw3.h>

class InputManager {
private:
    GLFWwindow* window;
    
    // Debouncing flags
    bool anyKeyPressed;
    bool leftPressed;
    bool rightPressed;
    bool spacePressed;
    bool qPressed;
    bool mPressed;
    
public:
    InputManager(GLFWwindow* win) : window(win), anyKeyPressed(false), 
        leftPressed(false), rightPressed(false), spacePressed(false),
        qPressed(false), mPressed(false) {}
    
    bool isEscapePressed() {
        return glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    }
    
    bool isAnyKeyPressed() {
        bool keyPressed = false;
        for (int key = 32; key <= 348; key++) {
            if (glfwGetKey(window, key) == GLFW_PRESS) {
                keyPressed = true;
                if (!anyKeyPressed) {
                    anyKeyPressed = true;
                    return true;
                }
                break;
            }
        }
        if (!keyPressed) {
            anyKeyPressed = false;
        }
        return false;
    }
    
    bool isLeftPressed() {
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !leftPressed) {
            leftPressed = true;
            return true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
            leftPressed = false;
        }
        return false;
    }
    
    bool isRightPressed() {
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !rightPressed) {
            rightPressed = true;
            return true;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
            rightPressed = false;
        }
        return false;
    }
    
    bool isSpacePressed() {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
            spacePressed = true;
            return true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
            spacePressed = false;
        }
        return false;
    }
    
    bool isJumpPressed() {
        return glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || 
               glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
               glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    }
    
    bool isAbilityPressed() {
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !qPressed) {
            qPressed = true;
            return true;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
            qPressed = false;
        }
        return false;
    }
    
    bool isMutePressed() {
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mPressed) {
            mPressed = true;
            return true;
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
            mPressed = false;
        }
        return false;
    }
};

#endif
