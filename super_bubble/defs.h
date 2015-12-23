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

// Window dimensions
static const GLuint WIDTH = 800, HEIGHT = 600;

static const double TARGET_FPS = 60.0;
static const double TARGET_FRAME_SECONDS = 1.0 / TARGET_FPS;

// Length of bubble chain needed to kill chain.
static const uint8_t CHAIN_DEATH_LENGTH = 4;

static const int8_t BOUNCE_HEIGHT = 5;

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
enum BubbleState { DEAD, IDLE, FALLING, DYING };

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
    bool rotate;
    bool drop;
};

static const uint16_t GRID_ROWS = 10;
static const uint16_t GRID_COLUMNS = 6;
// Grid size is defined in play space (which for sizes is the same as window space).
static const uint16_t GRID_SIZE = 50;
// Position of top left of play space in window space coordinates.
static const glm::uvec2 PLAY_SPACE_POS = glm::uvec2(250, 100);

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