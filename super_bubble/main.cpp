#include <iostream>
#include <list>

#include "defs.h"
#include "transforms.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "grid.h"
#include "game_logic.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

static Bubble grid[GRID_COLUMNS][GRID_ROWS];
static GameState state = BUBBLE_SPAWN;
static std::list<Bubble> fallingBubbles;
static Controls controls;

void update(double secondsSinceLastUpdate) {
	switch (state)
	{
	case MENU:
		state = menu();
		break;
	case BUBBLE_SPAWN:
		state = spawnBubble(fallingBubbles);
		break;
	case PLAYER_CONTROL:
		state = controlPlayerBubbles(grid, fallingBubbles, controls, secondsSinceLastUpdate);
		break;
	case SCAN_FOR_VICTIMS:
		state = scanForVictims(grid);
		break;
	case ANIMATE_DEATHS:
		state = animateDeaths(grid, secondsSinceLastUpdate);
		break;
	case SCAN_FOR_FLOATERS:
		state = scanForFloaters(grid, fallingBubbles);
		break;
	case GRAVITY:
		state = gravity(grid, fallingBubbles, secondsSinceLastUpdate);
		break;
	case GAME_OVER:
		state = gameOver();
		break;
	}
}

void draw() {	
	drawSprite(ResourceManager::GetTexture("background"), UV_SIZE_WHOLE_IMAGE, 0, 0, glm::uvec2(0, 0), glm::uvec2(WIDTH, HEIGHT), 0.0f);

	renderGrid(grid);

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
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "madeyes 2015", nullptr, nullptr);
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

	// Load shaders.
	ResourceManager::LoadShader("../shaders/sprite.vs", "../shaders/sprite.frag", nullptr, "sprite");
	// Configure shaders.
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), static_cast<GLfloat>(HEIGHT), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	initSpriteRenderer(ResourceManager::GetShader("sprite"));
	// Load textures.
	ResourceManager::LoadTexture("../resources/textures/background1.png", GL_FALSE, "background");
	ResourceManager::LoadTexture("../resources/textures/bubbles.png", GL_TRUE, "bubbles");
	// Initialise game components.
	initGrid(grid);
	controls.left = false;
	controls.right = false;
	controls.drop = false;
	controls.rotate = false;

	glfwSwapInterval(1);
	
	double frameTime = 0.0;
	double startTime = 0.0;
	uint32_t frame = 0;
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

		draw();

		glfwSwapBuffers(window);

		frame++;
	}

	// Clean up.
	deleteSpriteVertexArrays();
	ResourceManager::Clear();

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
		
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
	
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
	else if (key == GLFW_KEY_UP)
	{
		controls.rotate = pressed;
	}
	else if (key == GLFW_KEY_DOWN)
	{
		controls.drop = pressed;
	}
}