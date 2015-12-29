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
static const GLuint WIDTH = 1152, HEIGHT = 864;
static const float SCALE = (float)WIDTH / 800.0f;


// Grid size is defined in play space (which for sizes is the same as window space).
static const uint16_t GRID_SIZE = (int)(50.0f * SCALE);
// Position of top left of play space in window space coordinates.
static const glm::uvec2 PLAY_SPACE_POS = glm::uvec2((int)(250.0f * SCALE), (int)(100.0f * SCALE));
// Position to render score
static const glm::uvec2 SCORE_POS = glm::uvec2((int)(50.0f * SCALE), (int)(50.0f * SCALE));
// Position to render next bubbles.
static const glm::uvec2 NEXT_BUBBLE_POS = glm::uvec2((int)(625.0f * SCALE), (int)(58.0f * SCALE));
// Position to render label for next bubbles.
static const glm::uvec2 NEXT_BUBBLE_LABEL_POS = glm::uvec2((int)(620.0f * SCALE), (int)(180.0f * SCALE));
// Position to render game over.
static const glm::uvec2 GAME_OVER_POS = glm::uvec2((int)(250.0f * SCALE), (int)(200.0f * SCALE));

// FPS for whole game.
static const double TARGET_FPS = 60.0;
static const double TARGET_FRAME_SECONDS = 1.0 / TARGET_FPS;

// FPS and frames for bubble animations. All bubbles have the same number of frames.
static const int8_t BUBBLE_FRAMES = 10;
static const double BUBBLE_FPS = 20.0;
static const double BUBBLE_FRAME_SECONDS = 1.0 / BUBBLE_FPS;

// Size of the play field in grid space.
static const uint16_t GRID_ROWS = 10;
static const uint16_t GRID_COLUMNS = 6;

// Length of bubble chain needed to kill chain.
static const uint8_t CHAIN_DEATH_LENGTH = 4;

static const int8_t BOUNCE_HEIGHT = 6;

static const int8_t FAST_FALL_AMOUNT = (int8_t)(10.0f * SCALE);


enum GameState
{
    MENU,
    BUBBLE_SPAWN,
    PLAYER_CONTROL,
    SCAN_FOR_VICTIMS,
    ANIMATE_DEATHS,
    SCAN_FOR_FLOATERS,
    GRAVITY,
    GAME_OVER
};

enum BubbleColor { RED, GREEN, BLUE, YELLOW };
enum BubbleState { DEAD, IDLE, FALLING, DYING, GHOST };

static const uint8_t MAX_COLOR = YELLOW;

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
static const glm::vec2 UV_SIZE_BUBBLE = glm::vec2(0.1f, 0.25f);
static const glm::vec2 UV_SIZE_WHOLE_IMAGE = glm::vec2(1.0f, 1.0f);

// Rows in texture atlas for different bubble states.
static const uint8_t TEXTURE_ROW_IDLE = 0;
static const uint8_t TEXTURE_ROW_FALLING = 1;
static const uint8_t TEXTURE_ROW_DYING = 2;

static const glm::vec3 BUBBLE_COLORS[] =
{
    // Red
    glm::vec3(1.0f, 0.0f, 0.0f),
    // Green
    glm::vec3(0.0f, 1.0f, 0.0f),
    // Blue
    glm::vec3(0.0f, 0.0f, 1.0f),
    // Yellow
    glm::vec3(1.0f, 1.0f, 0.0f)
};

#endif