#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <utility>
#include <time.h>
#include <stdlib.h>
#include "enet/enet.h"
#include "defs.h"
#include "transforms.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "render_text.h"
#include "grid.h"
#include "game_logic.h"
#include "bubble_net.h"

static void startGame();
static GameState disconnect();
static void update(const double secondsSinceLastUpdate);
static void draw(const double secondsSinceLastUpdate);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

static const char* MENU_STRINGS [] = { "START SINGLE PLAYER", 
                                       "START MULTIPLAYER SERVER",
                                       "JOIN MULTIPLAYER SERVER",
                                       "HELP",
                                       "QUIT" };
static const uint8_t NUM_MENU_ITEMS = 5;
static const uint8_t MENU_START_SINGLE = 0, MENU_START_MULTI = 1, MENU_JOIN_MULTI = 2, MENU_HELP = 3, MENU_QUIT = 4;

static Bubble grid[GRID_COLUMNS][GRID_ROWS];
static GameState state = MENU;
static std::list<Bubble> fallingBubbles;
static Controls controls;
static TextRenderer *text = nullptr;
static uint32_t score = 0;
static std::pair <BubbleColor, BubbleColor> nextColors;
static uint8_t selectedMenuItem = 0;

static double frameTime = 0.0;
static double startTime = 0.0;
static uint32_t frame = 0;

static uint8_t numEnemyBubbles = 0;

static std::string errorMessage;

int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;    

    srand(time(NULL));

    if (enet_initialize() != 0)
    {
        std::cout << "Failed to initialise networking." << std::endl;
        //return 1;
    }

    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Super Bubble", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, keyCallback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders.
    ResourceManager::LoadShader("../shaders/sprite.vs", "../shaders/sprite.frag", nullptr, "sprite");
    // Configure shaders.
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), static_cast<GLfloat>(HEIGHT), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    initSpriteRenderer(ResourceManager::GetShader("sprite"));
    // Load textures.
    ResourceManager::LoadTexture("../resources/textures/background1.png", GL_FALSE, "background");
#ifdef DEBUG
    ResourceManager::LoadTexture("../resources/textures/bubbles_debug.png", GL_TRUE, "bubbles");
#else
    ResourceManager::LoadTexture("../resources/textures/bubbles.png", GL_TRUE, "bubbles");
#endif
    // Load text renderer.
    text = new TextRenderer(WIDTH, HEIGHT);
    text->Load("../fonts/ocraext.ttf", 24);

    // Sync to monitor refresh.
    glfwSwapInterval(1);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        if (frame != 0)
        {
            frameTime = glfwGetTime() - startTime;
        }
        update(frameTime);
        startTime = glfwGetTime();

        draw(frameTime);

        glfwSwapBuffers(window);

        frame++;
    }

    // Clean up.
    deleteSpriteVertexArrays();
    ResourceManager::Clear();

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    shutdownNetwork();
    enet_deinitialize();

    return 0;
}

static void startGame()
{
	nextColors.first = static_cast<BubbleColor>(rand() % (MAX_SPAWN_COLOR + 1));
	nextColors.second = static_cast<BubbleColor>(rand() % (MAX_SPAWN_COLOR + 1));
	initGrid(grid);
	controls.left = false;
	controls.right = false;
	controls.drop = false;
	controls.rotateCW = false;
	score = 0;
	resetGameLogic();
    frameTime = 0.0;
    startTime = 0.0;
    frame = 0;
    state = BUBBLE_SPAWN;
}

static GameState disconnect()
{
    static bool startDisconnect = false;
    if (!startDisconnect)
    {
        // Wait one frame to show disconnecting screen.
        startDisconnect = true;
        return DISCONNECT;
    }
    else
    {
        // Now start shutdown and switch to menu.
        shutdownNetwork();        
        startDisconnect = false;
        return MENU;
    }
}

static void update(const double secondsSinceLastUpdate) {
    NetMessage netMsg = updateNetwork();
    if (netMsg.type == NUM_BUBBLES)
    {
        numEnemyBubbles += netMsg.numBubbles;        
    }
    else if (netMsg.type == DISCONNECTED)
    {        
        errorMessage.assign("Connection lost.");
        state = DISCONNECT;
    }

    switch (state)
    {
    case MENU:      
        // Do nothing - menu is handled by key press call-back.
        break;
    case SERVER_LISTEN:
    case CLIENT_CONNECT:    
        if (netMsg.type == CONNECTED)
        {
            startGame();
        }
        break;
    case DISCONNECT:
        state = disconnect();
        break;        
    case BUBBLE_SPAWN:
        state = spawnBubble(fallingBubbles, nextColors);
        break;
    case PLAYER_CONTROL:
        state = controlPlayerBubbles(grid, fallingBubbles, controls, secondsSinceLastUpdate);
        break;
    case DROP_ENEMY_BUBBLES:
        state = dropEnemyBubbles(grid, fallingBubbles, numEnemyBubbles, secondsSinceLastUpdate);
        break;
    case SCAN_FOR_VICTIMS:
        state = scanForVictims(grid, score);
        break;
    case ANIMATE_DEATHS:
        state = animateDeaths(grid);
        break;
    case SCAN_FOR_FLOATERS:
        state = scanForFloaters(grid, fallingBubbles);
        break;
    case GRAVITY:
        state = gravity(grid, fallingBubbles, secondsSinceLastUpdate);
        break;
    case GAME_OVER:
        state = gameOver(grid);
        break;
    }
}

static void draw(const double secondsSinceLastUpdate) {    
    if (state == MENU || state == SERVER_LISTEN || state == CLIENT_CONNECT)
    {
        float theta = static_cast<float>(frame) / 30.0f;
        glClearColor(MENU_CLEAR_COLOR.r, MENU_CLEAR_COLOR.g, MENU_CLEAR_COLOR.b, MENU_CLEAR_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT);
        text->RenderText("S  U  P  E  R     B  U  B  B  L  E", MENU_TITLE_POS.x, MENU_TITLE_POS.y + (sin(theta) * 15.0f), SCALE, MENU_TITLE_COLOR);
    }
    if (state == DISCONNECT)
    {
        glClearColor(MENU_CLEAR_COLOR.r, MENU_CLEAR_COLOR.g, MENU_CLEAR_COLOR.b, MENU_CLEAR_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT);
        text->RenderText("Disconnecting...", MENU_TITLE_POS.x, MENU_TITLE_POS.y, SCALE, MENU_COLOR);
    }
    else if (state == MENU)
    {
        for (uint8_t i = 0; i < NUM_MENU_ITEMS; i++)
        {
            text->RenderText(MENU_STRINGS[i], MENU_POS.x, MENU_POS.y + (i * MENU_Y_SPACING), SCALE * 2.0f, 
                selectedMenuItem == i ? MENU_SELECTED_COLOR : MENU_COLOR);
        }
        if (errorMessage.length() > 0)
        {
            text->RenderText(errorMessage, ERROR_POS.x, ERROR_POS.y, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        }
	}
    else if (state == SERVER_LISTEN)
    {             
        text->RenderText("Waiting for connection...", MENU_POS.x, MENU_POS.y, SCALE, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (state == CLIENT_CONNECT)
    {
        text->RenderText("Connecting...", MENU_POS.x, MENU_POS.y, SCALE, glm::vec3(1.0f, 0.0f, 0.0f));
    }
	else
	{
		drawSprite(ResourceManager::GetTexture("background"), UV_SIZE_WHOLE_IMAGE, 0, 0, glm::uvec2(0, 0), glm::uvec2(WIDTH, HEIGHT), 0.0f);

		renderGrid(grid, secondsSinceLastUpdate);

		glm::uvec2 renderPos;
		// Render falling sprites.
		for (std::list<Bubble>::iterator it = fallingBubbles.begin(); it != fallingBubbles.end(); it++)
		{
			// The bubbles are defined in play space, but this may be offset from window space, so transform it.
			playSpaceToWindowSpace((*it).playSpacePosition, renderPos);

			drawSprite(
				// The texture atlas to use.
				ResourceManager::GetTexture("bubbles"),
				// Size of source image to extract from texture atlas.
				UV_SIZE_BUBBLE,
				// Column in texture sheet to use.
				(*it).animationFrame,
				// Row in texture sheet to use. Based on current state.
				(*it).state - 1,
				// Render position in window space coordinates.
				renderPos,
				// Size of target rendered image in window.
				glm::uvec2(GRID_SIZE, GRID_SIZE),
				// No rotation.
				0.0f,
				// RGB colour.
                BUBBLE_COLORS[(*it).color],
				// Clip falling sprites to top of play space so they enter smoothly
				PLAY_SPACE_POS.y);
		}

		// Render next bubbles.
		drawSprite(ResourceManager::GetTexture("bubbles"), UV_SIZE_BUBBLE, 0, 0, NEXT_BUBBLE_POS,
			glm::uvec2(GRID_SIZE, GRID_SIZE), 0.0f, BUBBLE_COLORS[nextColors.first], 0);
		drawSprite(ResourceManager::GetTexture("bubbles"), UV_SIZE_BUBBLE, 0, 0, NEXT_BUBBLE_POS + glm::uvec2(0, GRID_SIZE),
			glm::uvec2(GRID_SIZE, GRID_SIZE), 0.0f, BUBBLE_COLORS[nextColors.second], 0);
		text->RenderText("NEXT", NEXT_BUBBLE_LABEL_POS.x, NEXT_BUBBLE_LABEL_POS.y, SCALE, glm::vec3(1.0f, 0.0f, 0.0f));

		// Render score.
		std::ostringstream ss;
		ss << "Score " << score;
		text->RenderText(ss.str(), SCORE_POS.x, SCORE_POS.y, SCALE, glm::vec3(1.0f, 0.0f, 0.0f));

		if (state == GAME_OVER)
		{
			text->RenderText("GAME OVER!", GAME_OVER_POS.x, GAME_OVER_POS.y, 3.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            text->RenderText("PRESS A KEY", GAME_OVER_POS.x + 50.0f, GAME_OVER_POS.y + 100.0f, 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}    
}

// Is called whenever a key is pressed/released via GLFW
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (state != MENU)
        {
            errorMessage.clear();
            state = DISCONNECT;
        }
    }
	else if (state == MENU && action == GLFW_PRESS)
	{
        if (key == GLFW_KEY_UP)
        {
            if (selectedMenuItem > 0)
            {
                selectedMenuItem--;
            }
        }
        else if (key == GLFW_KEY_DOWN)
        {
            if (selectedMenuItem < NUM_MENU_ITEMS - 1)
            {
                selectedMenuItem++;
            }
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
        {
            switch (selectedMenuItem)
            {
            case MENU_START_SINGLE:
                startGame();                
                break;
            case MENU_START_MULTI:
                if (!createServer())
                {                    
                    errorMessage.assign("Server creation failed.");
                    state = MENU;
                }
                else
                {
                    state = SERVER_LISTEN;
                }
                break;
            case MENU_JOIN_MULTI:
                if (!createClient() || !clientConnect("192.168.0.4"))
                {                    
                    errorMessage.assign("Client creation failed.");
                    state = MENU;
                }
                else
                {
                    state = CLIENT_CONNECT;
                }
                break;
            case MENU_HELP:
                break;
            case MENU_QUIT:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            }
            
        }		
	}
	else if (state == GAME_OVER && action == GLFW_PRESS)
	{        
		state = DISCONNECT;
	}
    else
    {
        // In game - so get keys for moving bubbles.
        bool pressed;
        if (action == GLFW_PRESS)
        {
            pressed = true;
        }
        else if (action == GLFW_RELEASE)
        {
            pressed = false;
        }
        else
        {
            return;
        }

        if (key == GLFW_KEY_LEFT)
        {
            controls.left = pressed;
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            controls.right = pressed;
        }
        else if (key == GLFW_KEY_A)
        {
            controls.rotateCW = pressed;
        }
        else if (key == GLFW_KEY_Z)
        {
            controls.rotateACW = pressed;
        }
        else if (key == GLFW_KEY_DOWN)
        {
            controls.drop = pressed;
        } 
    }
}