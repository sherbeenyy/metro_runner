# Metro Runner - Architecture Documentation

## System Overview

Metro Runner is a 2D endless runner game built with C++ and OpenGL. The architecture follows a modular design pattern where each class has a single, well-defined responsibility.

## Class Architecture Diagram

```
                                   main.cpp
                                      |
                                      v
                                  Game.h
                        (Main Game Coordinator)
                                      |
                    +----------------+----------------+
                    |                |                |
                    v                v                v
            InputManager.h    AssetManager.h    GameWorld.h
            (Input Handling)  (Asset Loading)   (Game Logic)
                                                      |
                                                      v
                                                 GameObject.h
                                                 (Entities)
                    |
                    +----------------+
                    |                |
                    v                v
              UIRenderer.h       Player.h
              (UI Rendering)     (Player State)
                    |
                    v
              Renderer2D.h
              (Low-level GL)
                    |
                    v
               Texture.h
               (Image Loading)

            GameData.h
            (Score Persistence)
```

## Core Classes

### 1. **Game.h** - Main Coordinator
- **Role**: Top-level orchestrator
- **Manages**: Game loop, state transitions, subsystem coordination
- **Owns**: InputManager, AssetManager, GameWorld, UIRenderer, ScoreManager
- **Key Methods**:
  - `init()`: Initialize all subsystems
  - `run()`: Main game loop
  - `handleInput()`: Delegate input to InputManager
  - `update(deltaTime)`: Update game state
  - `render(currentTime)`: Render current frame

### 2. **InputManager.h** - Input System
- **Role**: Keyboard input handling
- **Purpose**: Centralize all GLFW input queries with debouncing
- **Features**:
  - Prevents key repeat with debouncing flags
  - Clean boolean API (isJumpPressed(), isAbilityPressed(), etc.)
  - Handles navigation (arrows), actions (space, Q), and special keys (M for mute)
- **Used By**: Game class in handleInput() method

### 3. **AssetManager.h** - Resource Management
- **Role**: Load and manage game assets
- **Manages**:
  - Textures: background, metro platform, 4 player heads
  - Audio: background music with loop and mute control
- **Key Methods**:
  - `loadAssets()`: Load all resources at startup
  - `getPlayerHead(index)`: Access character textures
  - `toggleMusic()`: Mute/unmute audio
- **Used By**: Game (initialization), UIRenderer (texture access)

### 4. **GameWorld.h** - Game Logic & Physics
- **Role**: Manage all game objects and gameplay mechanics
- **Manages**:
  - Metro platforms (8 platforms, 350px wide, 80px gaps)
  - Obstacles (spawns every 2s, ground and flying types)
  - Coins (spawns every 1.5s, tracks collection)
  - Game speed (starts at 3.0, increases every 10s)
- **Collision Detection**:
  - `isPlayerOnPlatform()`: Platform collision using player center
  - `checkObstacleCollision()`: Obstacle hit detection with invincibility check
  - `checkFallThrough()`: Detects player falling through gaps
- **Used By**: Game class for world updates and collision queries

### 5. **UIRenderer.h** - UI & Rendering
- **Role**: Handle all visual UI elements
- **Renders**:
  - Start screen (title, prompt)
  - Character select (4 characters with abilities)
  - Playing HUD (coins, ability timer, music status, controls)
  - Player as stick figure with custom head
  - Game over screen (scores, restart prompt)
- **Dependencies**: Uses Renderer2D for actual drawing
- **Used By**: Game class for all UI rendering

### 6. **Player.h** - Player State
- **Role**: Track player physics and abilities
- **Properties**:
  - Position (x, y), velocity, jumping state
  - Character index (0-3 for different abilities)
  - Ability timers and cooldowns
- **Abilities**:
  - Character 0: Shield (5s invincibility)
  - Character 1: Double Jump (8s air jumping)
  - Character 2: Magnet (6s double coins)
  - Character 3: Dash (5s speed boost 1.8x)
- **Used By**: Game (update physics), GameWorld (collision checks)

### 7. **Renderer2D.h** - Low-Level Rendering
- **Role**: OpenGL abstraction layer
- **Features**:
  - Quad rendering (textured and colored)
  - Bitmap font text rendering (A-Z, 0-9)
  - Shader management
  - Transform matrices
- **Used By**: UIRenderer for all drawing operations

### 8. **Texture.h** - Image Loading
- **Role**: Load and manage OpenGL textures
- **Uses**: stb_image.h for PNG/JPG loading
- **Features**: Automatic OpenGL texture creation with proper settings

### 9. **GameObject.h** - Entity Definitions
- **Role**: Define game entity structures
- **Structs**:
  - `Metro`: Platform with x, y, width
  - `Obstacle`: Hazard with dimensions, flying flag
  - `Coin`: Collectible with position and collected state
- **Used By**: GameWorld to store and manage entities

### 10. **GameData.h** - Persistence
- **Role**: Save/load high scores
- **Features**: Custom JSON parser, score tracking
- **Used By**: Game class for score persistence

## Data Flow

### Initialization
```
main() 
  → Game.init()
    → glfwInit() + window creation
    → InputManager(window)
    → AssetManager.loadAssets()
    → GameWorld.init()
    → Player positioning
```

### Game Loop (each frame)
```
Game.run()
  → handleInput()
    → InputManager.isXPressed() queries
    → State-specific input handling
  
  → update(deltaTime)
    → Player.update() (physics)
    → GameWorld.update()
      → Update metros, obstacles, coins
      → Check collisions
    
  → render(currentTime)
    → UIRenderer methods based on state
    → Renderer2D.drawQuad(), drawText()
```

### State Transitions
```
START_SCREEN → CHARACTER_SELECT
  Trigger: Any key press (InputManager.isAnyKeyPressed())
  Action: Switch state

CHARACTER_SELECT → PLAYING
  Trigger: Space key (InputManager.isSpacePressed())
  Action: startGame() - reset player, world, and scores

PLAYING → GAME_OVER
  Trigger: Collision detected (GameWorld.checkObstacleCollision() or checkFallThrough())
  Action: endGame() - save scores, switch state

GAME_OVER → CHARACTER_SELECT
  Trigger: Space key
  Action: Switch state for replay
```

## Communication Patterns

### 1. **Composition**: Game owns all subsystems
- Game creates and destroys InputManager, UIRenderer
- Game owns AssetManager, GameWorld, ScoreManager instances

### 2. **Delegation**: Game delegates responsibilities
- Input handling → InputManager
- Asset access → AssetManager
- World updates → GameWorld
- UI rendering → UIRenderer

### 3. **Queries**: Read-only state access
- GameWorld provides const getters for entities
- Player has const methods for ability checks
- AssetManager provides texture pointers

### 4. **Updates**: Mutable state modifications
- Game updates Player physics directly
- GameWorld updates entities, accepts Player& for abilities
- AssetManager controls music state

## Separation of Concerns

| Class | Input | Logic | Rendering | Data |
|-------|-------|-------|-----------|------|
| Game | ✓ | ✓ | - | - |
| InputManager | ✓ | - | - | - |
| AssetManager | - | - | - | ✓ |
| GameWorld | - | ✓ | - | - |
| UIRenderer | - | - | ✓ | - |
| Player | - | ✓ | - | - |
| Renderer2D | - | - | ✓ | - |

## Advantages of This Architecture

1. **Maintainability**: Each class has clear, single responsibility
2. **Testability**: Systems can be tested independently
3. **Extensibility**: Easy to add new features
   - New input: Add to InputManager
   - New asset: Add to AssetManager
   - New game object: Add to GameWorld
   - New UI element: Add to UIRenderer
4. **Reusability**: Renderer2D, Texture, InputManager can be reused in other projects
5. **Readability**: Clear ownership and communication patterns

## File Dependencies

```
main.cpp
  └─ Game.h
      ├─ InputManager.h (GLFW)
      ├─ AssetManager.h
      │   └─ Texture.h (stb_image.h)
      ├─ GameWorld.h
      │   ├─ GameObject.h
      │   ├─ Player.h
      │   └─ Renderer2D.h (constants)
      ├─ UIRenderer.h
      │   ├─ Renderer2D.h
      │   ├─ Player.h
      │   └─ Texture.h
      ├─ GameData.h
      └─ Player.h
```

## Build System

```bash
g++ -o metro_runner main.cpp libs/glad/src/glad.c \
    -Ilibs/glad/include -lglfw -lGL -ldl -lsfml-audio
```

**External Dependencies**:
- GLFW 3.x: Window and input
- GLAD: OpenGL loader
- SFML Audio: Music playback
- stb_image.h: Image loading (header-only)

## Performance Considerations

- **Asset Loading**: All textures loaded once at startup
- **Memory**: Static entity pools (vectors with reserve)
- **Rendering**: Batch rendering via Renderer2D
- **Updates**: Delta-time based physics for consistent gameplay
- **Collision**: Simple AABB (Axis-Aligned Bounding Box) detection

## Future Extensibility

To add new features:
- **New Character**: Add to AssetManager, update UIRenderer arrays
- **New Obstacle Type**: Extend GameObject.h, modify GameWorld spawn logic
- **New Game State**: Add to GameState enum, implement handlers in Game class
- **Sound Effects**: Extend AssetManager with sf::SoundBuffer
- **Particle Effects**: Create new ParticleSystem class, integrate in Game

---

**Last Updated**: December 9, 2025  
**Architecture Version**: 2.0 (Refactored from monolithic main.cpp)
