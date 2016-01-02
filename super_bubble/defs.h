#ifndef DEFS_H
#define DEFS_H

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdint.h>

//#define DEBUG

// Window dimensions
// The aspect ratio is fixed at 4:3. The WIDTH and HEIGHT here must maintain that.
const GLuint WIDTH = 1152, HEIGHT = 864;
const float SCALE = (float)WIDTH / 800.0f;


// Grid size is defined in play space (which for sizes is the same as window space).
const uint16_t GRID_SIZE = (int)(50.0f * SCALE);
// Position of top left of play space in window space coordinates.
const glm::uvec2 PLAY_SPACE_POS = glm::uvec2((int)(250.0f * SCALE), (int)(100.0f * SCALE));
// Position to render score
const glm::uvec2 SCORE_POS = glm::uvec2((int)(50.0f * SCALE), (int)(50.0f * SCALE));
// Position to render next bubbles.
const glm::uvec2 NEXT_BUBBLE_POS = glm::uvec2((int)(625.0f * SCALE), (int)(58.0f * SCALE));
// Position to render label for next bubbles.
const glm::uvec2 NEXT_BUBBLE_LABEL_POS = glm::uvec2((int)(620.0f * SCALE), (int)(180.0f * SCALE));
// Position to render game over.
const glm::uvec2 GAME_OVER_POS = glm::uvec2((int)(250.0f * SCALE), (int)(200.0f * SCALE));
// Menu positions.
const glm::uvec2 MENU_TITLE_POS = glm::uvec2((int)(50.0f * SCALE), (int)(50.0f * SCALE));
const glm::uvec2 MENU_POS = glm::uvec2((int)(50.0f * SCALE), (int)(100.0f * SCALE));
const  uint16_t MENU_Y_SPACING = (int)(40.0f * SCALE);
// Error position.
const glm::uvec2 ERROR_POS = glm::uvec2((int)(20.0f * SCALE), (int)(500.0f * SCALE));

const glm::vec3 MENU_TITLE_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 MENU_COLOR = glm::vec3(0.7f, 0.7f, 0.7f);
const glm::vec3 MENU_SELECTED_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec4 MENU_CLEAR_COLOR = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

// FPS for whole game.
const double TARGET_FPS = 60.0;
const double TARGET_FRAME_SECONDS = 1.0 / TARGET_FPS;

// FPS and frames for bubble animations. All bubbles have the same number of frames.
const int8_t BUBBLE_FRAMES = 10;
const double BUBBLE_FPS = 20.0;
const double BUBBLE_FRAME_SECONDS = 1.0 / BUBBLE_FPS;

// Size of the play field in grid space.
const uint8_t GRID_ROWS = 10;
const uint8_t GRID_COLUMNS = 6;

// Length of bubble chain needed to kill chain.
const uint8_t CHAIN_DEATH_LENGTH = 4;
// Length of bubble chain needed to send to other player.
const uint8_t CHAIN_MIN_SEND_LENGTH = 5;

const int8_t BOUNCE_HEIGHT = 6;

const int8_t FAST_FALL_AMOUNT = (int8_t)(10.0f * SCALE);


enum GameState
{
    MENU,
    HELP,
    TEXT_ENTRY,    
    SERVER_LISTEN,
    CLIENT_CONNECT,
    DISCONNECT,
    BUBBLE_SPAWN,
    PLAYER_CONTROL,
    DROP_ENEMY_BUBBLES,
    SCAN_FOR_VICTIMS,
    ANIMATE_DEATHS,
    SCAN_FOR_FLOATERS,
    GRAVITY,
    WIN,
    GAME_OVER
};

enum BubbleColor { RED, GREEN, BLUE, YELLOW, GHOST };
enum BubbleState { DEAD, IDLE, FALLING, DYING };
const uint8_t MAX_SPAWN_COLOR = YELLOW;

const glm::vec3 BUBBLE_COLORS[] =
{
    // Red
    glm::vec3(1.0f, 0.0f, 0.0f),
    // Green
    glm::vec3(0.0f, 1.0f, 0.0f),
    // Blue
    glm::vec3(0.0f, 0.0f, 1.0f),
    // Yellow
    glm::vec3(1.0f, 1.0f, 0.0f),
    // Ghost
    glm::vec3(0.7f, 0.7f, 0.7f)
};

struct Bubble
{
    // This is the position within the play area.
    glm::ivec2 playSpacePosition;
    BubbleColor color;
    BubbleState state;
    uint8_t animationFrame;
    bool visited;
    int8_t bounceAmount;
    int8_t bounceDir;

    Bubble()
    {
        playSpacePosition.x = 0;
        playSpacePosition.y = 0;
        color = RED;
        state = IDLE;
        animationFrame = 0;
        visited = false;
        bounceAmount = 0;
        bounceDir = 0;
    }    
};

struct Controls
{
    bool left;
    bool right;
    bool rotateCW;
    bool rotateACW;
    bool drop;
};


// UV size of sub images in texture atlases.
const glm::vec2 UV_SIZE_BUBBLE = glm::vec2(0.1f, 0.25f);
const glm::vec2 UV_SIZE_WHOLE_IMAGE = glm::vec2(1.0f, 1.0f);

// Rows in texture atlas for different bubble states.
const uint8_t TEXTURE_ROW_IDLE = 0;
const uint8_t TEXTURE_ROW_FALLING = 1;
const uint8_t TEXTURE_ROW_DYING = 2;


#endif