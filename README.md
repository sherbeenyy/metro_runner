# Metro Runner - Game Documentation

## Overview
Metro Runner is a 2D endless runner game inspired by the Egypt metro system. Players control characters running on metro platforms, jumping between cars while avoiding obstacles and collecting coins.

## Project Structure

```
subway/
├── main.cpp              # Main game loop and state management
├── Player.h              # Player character class with abilities
├── Renderer2D.h          # 2D rendering system with bitmap fonts
├── GameObject.h          # Game entity definitions (Metro, Obstacle, Coin)
├── Texture.h             # Image loading wrapper using stb_image
├── GameData.h            # Score/coin persistence with JSON
├── stb_image.h           # STB single-header image library
├── song                  # Background music (MP3)
├── libs/
│   └── glad/             # OpenGL loader
│       ├── include/
│       └── src/
└── imgs/
    ├── metro_background.jpg
    ├── metro_side_view.PNG
    └── players/
        ├── p1.PNG        # BIG JOE head
        ├── p2.PNG        # ALI 3ALOKA head
        ├── p3.PNG        # HAMDA head
        └── P4.PNG        # SPEEDY head
```

## File Documentation

### main.cpp
**Purpose**: Core game engine and main loop

**Key Components**:
- **Game States**: START_SCREEN → CHARACTER_SELECT → PLAYING → GAME_OVER
- **Window Management**: GLFW initialization (1280x720 resolution)
- **Game Loop**: Delta time calculation, input handling, rendering
- **Physics System**: Platform detection, falling mechanics, collision detection
- **Spawning System**: Obstacles, coins, and platform management
- **Audio System**: Background music using SFML

**Main Variables**:
```cpp
GameState state                 // Current game state
Player player                   // Player instance
vector<Metro> metros            // Active metro platforms
vector<Obstacle> obstacles      // Active obstacles
vector<Coin> coins             // Active coins
float gameSpeed                 // Current game speed (increases over time)
float metroGap = 80            // Gap between metro cars (pixels)
float metroY = 500             // Metro platform Y position
```

**Game Flow**:
1. Initialize OpenGL, GLAD, GLFW
2. Load textures and start background music
3. Enter main loop:
   - Process input based on current state
   - Update game physics and entities
   - Check collisions
   - Render all game elements
   - Handle state transitions

**How It Uses Other Files**:
- `#include "Player.h"` - Player character management
- `#include "Renderer2D.h"` - All rendering operations
- `#include "GameObject.h"` - Metro, Obstacle, Coin definitions
- `#include "Texture.h"` - Loading player heads and backgrounds
- `#include "GameData.h"` - Persistent score/coin storage
- `#include <SFML/Audio.hpp>` - Background music playback

---

### Player.h
**Purpose**: Player character class with physics and special abilities

**Class Structure**:
```cpp
class Player {
    // Position and physics
    float x, y;                    // Position on screen
    float velocityY;               // Vertical velocity for jumping
    float width, height;           // Hitbox dimensions (40x80)
    bool isJumping;                // Currently in air
    int headIndex;                 // Selected character (0-3)
    
    // Ability system
    bool abilityActive;            // Ability currently active
    float abilityTimer;            // Time remaining on ability
    float abilityCooldown;         // Cooldown until next use
    bool canDoubleJump;            // For ALI 3ALOKA ability
    bool hasDoubleJumped;          // Tracks if double jump used
}
```

**Key Methods**:
- `jump()` - Execute jump with velocity -12, handles double jump for character 1
- `activateAbility()` - Activate character-specific ability with 8s cooldown
- `update(groundY, deltaTime)` - Update physics, apply gravity (0.5f), update timers
- `isInvincible()` - Returns true if BIG JOE has shield active
- `hasDoubleCoinBonus()` - Returns true if HAMDA has magnet active
- `getSpeedMultiplier()` - Returns 1.0 (no longer used for slow time)
- `getPlayerSpeedMultiplier()` - Returns 1.8 if SPEEDY has dash active

**Character Abilities**:
- **Character 0 (BIG JOE)**: Shield - 5s invincibility
- **Character 1 (ALI 3ALOKA)**: Double Jump - 8s can jump in air
- **Character 2 (HAMDA)**: Magnet - 6s double coin collection
- **Character 3 (SPEEDY)**: Dash - 5s increased game speed (1.8x)

**Physics Details**:
- Jump velocity: -12 pixels/frame
- Gravity: 0.5 pixels/frame²
- Ground level: metroY - height
- Collision box: 40x80 pixels

**How It's Used**:
- `main.cpp` creates Player instance and calls update() every frame
- Checks ability status for invincibility, double coins, speed boost
- Input handling calls jump() and activateAbility()

---

### Renderer2D.h
**Purpose**: Complete 2D rendering system with custom bitmap font

**Class Structure**:
```cpp
class Renderer2D {
    GLuint shaderProgram;          // Compiled shader program
    GLuint VAO, VBO, EBO;          // OpenGL vertex buffers
    GLuint transformLoc;           // Shader uniform location
}
```

**Key Methods**:
- `drawQuad(x, y, w, h, texture, r, g, b, a)` - Draw colored/textured rectangle
- `drawText(text, x, y, size, r, g, b)` - Render text using bitmap font
- `drawChar(char, x, y, size, r, g, b)` - Draw single character (5x7 pixel grid)
- `drawPixel(x, y, size, r, g, b)` - Draw single pixel for bitmap font

**Coordinate System**:
- Origin (0, 0) at top-left corner
- X increases right, Y increases downward
- Screen size: 1280x720 pixels
- Transforms to OpenGL NDC (-1 to 1) internally

**Shader System**:
- **Vertex Shader**: Transforms screen coordinates to NDC
- **Fragment Shader**: Applies color and texture with alpha blending
- **Transform Matrix**: 4x4 matrix for position, scale, texture coords

**Bitmap Font**:
- 5x7 pixel patterns for each letter A-Z and digits 0-9
- Drawn using multiple drawPixel() calls per character
- Scalable size parameter
- Supports custom RGB colors

**Rendering Pipeline**:
1. Set up transform matrix with position/scale
2. Upload matrix to shader
3. Bind texture (if any)
4. Draw quad geometry
5. Repeat for each object

**How It's Used**:
- `main.cpp` creates Renderer2D instance at startup
- All drawing calls go through this class
- Renders: backgrounds, metros, obstacles, coins, player, text UI

---

### GameObject.h
**Purpose**: Simple data structures for game entities

**Structures**:

```cpp
struct Metro {
    float x, y;              // Position
    float width;             // Platform width (350px normal, 600px start)
    bool active;             // Still in use
}

struct Obstacle {
    float x, y;              // Position
    float width, height;     // Collision box
    bool isFlying;           // Ground (false) or aerial (true)
    bool active;             // Still in use
}

struct Coin {
    float x, y;              // Position
    float size;              // Coin dimensions (square)
    bool collected;          // Already picked up
}
```

**How They're Used Together**:
- **Metro**: Spawned in a continuous loop, wraps around when off-screen
  - Player must land with center over platform or falls
  - 80 pixel gaps between platforms
  
- **Obstacle**: Spawns every 2 seconds from right side
  - Ground obstacles: Y = metroY - 60, height = 60
  - Flying obstacles: Y = metroY - 180, height = 30
  - Checked for AABB collision with player
  - Removed when x < 0
  
- **Coin**: Spawns every 1.5 seconds
  - Y position randomized: metroY - 150 to -250
  - Collision detection with player
  - Removed when collected or off-screen

**Entity Lifecycle**:
1. Spawn off-screen right (x = SCREEN_WIDTH + offset)
2. Move left by gameSpeed each frame
3. Check collision/interaction with player
4. Mark as inactive when off-screen left
5. Remove from vector using erase-remove idiom

---

### Texture.h
**Purpose**: Image loading wrapper using stb_image

**Class Structure**:
```cpp
class Texture {
    GLuint id;               // OpenGL texture handle
    int width, height;       // Image dimensions
    int channels;            // Color channels (3=RGB, 4=RGBA)
}
```

**Key Methods**:
- `load(filename)` - Load PNG/JPG file into OpenGL texture
  - Uses stb_image to decode file
  - Generates OpenGL texture with proper format
  - Sets filtering to GL_LINEAR for smooth scaling
  - Returns true if successful
  
- `bind()` - Activate texture for rendering
- `unbind()` - Deactivate texture
- Destructor - Cleans up OpenGL texture

**Supported Formats**:
- PNG (with transparency)
- JPG/JPEG
- BMP
- TGA
- And more via stb_image

**How It's Used**:
- `main.cpp` creates Texture instances for:
  - Background image (metro_background.jpg)
  - Metro platform sprite (metro_side_view.PNG)
  - Player heads (p1.PNG, p2.PNG, p3.PNG, P4.PNG)
- Passed to Renderer2D::drawQuad() for textured rendering
- Automatically handles RGBA vs RGB formats

---

### GameData.h
**Purpose**: JSON-based save system for scores and coins

**Class Structure**:
```cpp
class ScoreManager {
    int bestScore;           // Highest score achieved
    int totalCoins;          // Total coins collected across all games
    string filename;         // Save file path ("game_data.json")
}
```

**Key Methods**:
- `load()` - Read JSON from file, parse into variables
- `save()` - Write current data to JSON file
- `updateBestScore(score)` - Update if new high score
- `addCoins(amount)` - Increment total coin count
- `getBestScore()` - Get current best score
- `getTotalCoins()` - Get total coins

**JSON Format**:
```json
{
    "bestScore": 140,
    "totalCoins": 192
}
```

**Custom JSON Parser**:
- Parses simple key-value pairs
- Handles integers
- Skips whitespace and special characters
- Writes formatted JSON with newlines

**How It's Used**:
- Loaded at game start in `main.cpp`
- Updated on every game over
- Displayed in game over screen
- Persists between game sessions

---

### stb_image.h
**Purpose**: Single-header image loading library

**What It Does**:
- Decodes image files into raw RGB/RGBA pixel data
- Handles multiple formats automatically
- Memory management for pixel buffers
- Used by Texture.h internally

**Key Functions Used**:
```cpp
stbi_load(filename, &width, &height, &channels, 0)  // Load image
stbi_image_free(data)                               // Free memory
```

**Integration**:
- `#define STB_IMAGE_IMPLEMENTATION` in one .cpp file
- Included by Texture.h
- No separate compilation needed

---

## How Files Work Together

### Initialization Flow
```
main.cpp starts
    ↓
Initialize GLFW + OpenGL (glad)
    ↓
Create Renderer2D instance
    ↓
Load Texture instances (uses stb_image.h)
    ↓
Start sf::Music (SFML audio)
    ↓
Load GameData from JSON
    ↓
Create Player instance
    ↓
Enter main game loop
```

### Main Game Loop Flow
```
While window open:
    ↓
Calculate deltaTime
    ↓
Handle Input (GLFW) → Update Player state
    ↓
Update Physics:
    - Player.update() (applies gravity)
    - Move metros left by gameSpeed
    - Move obstacles left
    - Move coins left
    ↓
Check Collisions:
    - Platform detection (GameObject metros)
    - Obstacle hits (if not Player.isInvincible())
    - Coin collection (with Player.hasDoubleCoinBonus())
    ↓
Render Everything:
    - Renderer2D.drawQuad() for sprites
    - Renderer2D.drawText() for UI
    - Uses Texture instances
    ↓
Handle State Transitions:
    - START_SCREEN → CHARACTER_SELECT
    - CHARACTER_SELECT → PLAYING (sets Player.headIndex)
    - PLAYING → GAME_OVER (saves with GameData)
    - GAME_OVER → CHARACTER_SELECT (reset)
```

### Rendering Pipeline
```
For each frame:
    Renderer2D receives draw calls
        ↓
    Set transform matrix (screen → NDC)
        ↓
    Bind texture if provided (Texture class)
        ↓
    Upload data to shader
        ↓
    Draw quad geometry
        ↓
    Text uses bitmap font (drawChar loops)
```

### Data Flow Example: Player Jump
```
User presses SPACE
    ↓
main.cpp detects input
    ↓
Calls player.jump()
    ↓
Player.h sets velocityY = -12
    ↓
Player.h sets isJumping = true
    ↓
Each frame: Player.update() applies gravity
    ↓
velocityY += 0.5 (pulls down)
    ↓
y += velocityY (move position)
    ↓
When y >= groundY: reset to ground
    ↓
Renderer2D.drawQuad() draws at new position
```

### Ability System Flow
```
User presses Q
    ↓
main.cpp calls player.activateAbility()
    ↓
Player.h checks cooldown
    ↓
Sets abilityActive = true
    ↓
Sets timer based on character
    ↓
Each frame: Player.update() decrements timer
    ↓
main.cpp queries ability status:
    - isInvincible() → skip obstacle collision
    - hasDoubleCoinBonus() → coinValue *= 2
    - getPlayerSpeedMultiplier() → gameSpeed *= 1.8
    ↓
When timer reaches 0: abilityActive = false
    ↓
Start cooldown countdown
```

## Game Mechanics

### Platform System
- 8 Metro platforms spawn in a loop
- First platform is 600px wide (safe start area)
- Remaining platforms are 350px wide with 80px gaps
- Player must keep center (x + width/2) over platform
- If center is over gap → start falling → game over

### Falling Detection
```cpp
float playerCenterX = player.x + player.width / 2;
bool onPlatform = false;
for (metro in metros) {
    if (playerCenterX > metro.x && playerCenterX < metro.x + metro.width) {
        onPlatform = true;
    }
}
if (!onPlatform && player.y > metroY + 50) {
    GAME_OVER;
}
```

### Speed Progression
- Base speed: 3.0 pixels/frame
- Increases by 0.5 every 10 seconds
- All entities move by `effectiveSpeed = gameSpeed * multipliers`
- Multipliers: ability-based speed changes

### Collision Detection
- **Obstacles**: Axis-Aligned Bounding Box (AABB)
  ```cpp
  if (player.x < obs.x + obs.width &&
      player.x + player.width > obs.x &&
      player.y < obs.y + obs.height &&
      player.y + player.height > obs.y) {
      // Collision!
  }
  ```
- **Coins**: Simple point-in-rect
- **Platforms**: Center-point check

## Build Instructions

### Dependencies
```bash
# Install required libraries
sudo apt-get install libglfw3-dev libsfml-dev
```

### Compilation
```bash
g++ -o metro_runner main.cpp libs/glad/src/glad.c \
    -Ilibs/glad/include \
    -lglfw -lGL -ldl -lsfml-audio
```

### Run
```bash
./metro_runner
```

## Controls

### Main Menu
- **Any Key**: Start → Go to character select

### Character Select
- **LEFT/RIGHT Arrow**: Choose character
- **SPACE**: Confirm and start game

### In Game
- **SPACE / UP / W**: Jump
- **Q**: Activate character ability
- **ESC**: Quit game

### Game Over
- **SPACE**: Return to character select

## Character Abilities Reference

| Character | Name | Ability | Duration | Effect | Best For |
|-----------|------|---------|----------|--------|----------|
| p1.PNG | BIG JOE | Shield | 5s | Invincibility | Aggressive play, risky maneuvers |
| p2.PNG | ALI 3ALOKA | Double Jump | 8s | Jump mid-air | Precision, correcting mistakes |
| p3.PNG | HAMDA | Magnet | 6s | 2x coins | High score chasing |
| P4.PNG | SPEEDY | Dash | 5s | 1.8x speed | Speed running, getting ahead |

All abilities have 8-second cooldown after ending.

## Performance Notes

- Target framerate: 60 FPS (deltaTime-based)
- Texture resolution: Varies by asset (see images)
- Audio format: MP3 (streamed, not preloaded)
- OpenGL version: 3.3 Core Profile
- Window size: 1280x720 (not resizable)

## Known Issues

- Segmentation fault on exit (Exit Code 139)
  - Game runs fine during gameplay
  - Occurs during cleanup phase
  - Does not affect gameplay or data saving
  - Likely related to OpenGL/GLFW cleanup order

## Future Enhancement Ideas

- Add more obstacles types
- Implement power-ups on metro platforms
- Add particle effects for abilities
- Create level themes (different metro lines)
- Add leaderboards
- Implement achievements system
- Add sound effects for jumps, coins, collisions
- Create animated character sprites
- Add combo system for consecutive coin collection
