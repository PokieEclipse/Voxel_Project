#include "game.h"
#include <iostream>

#include <fstream>
#include <json.hpp>

#include "Utility/lodepng.h"

#include "World/world.h"
#include "World/chunk.h"
#include "World/block.h"

#include "Utility/Camera.h"

#include "Entity/Player/Player.h"

extern Camera Utility::camera;

namespace Utility {
	glm::mat4 perspective = glm::perspective(glm::radians(120.0f), (float)1920 / (float)1080, 0.1f, 10000.0f);
}

Game::Game()
{
	Utility::perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);
}

Game::~Game()
{
	delete world;
	delete player;
}

int Game::SetupGame()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creates window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Voxel Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create a window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowUserPointer(window, this);

	glfwMakeContextCurrent(window);

	glewInit();

	glClearColor(0, 0.7, 0.9, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, ResizeWindowCallback);

	LoadTextureAtlas();

	world = new Minecraft::World(this);
	player = new Minecraft::Player(world);

	return 0;
}

void Game::MainLoop()
{
	world->GenerateWorld();

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Update();

		glfwSwapBuffers(window);
	}
}

void Game::ShutdownGame()
{
	shutdownState = true;

	while (!world->finishedThread)
	{
		std::cout << "Waiting for chunk thread to finish..." << std::endl;
	}
	
	glfwSetWindowShouldClose(GetGameWindow(), true);
}

void Game::SetScreenSize(const unsigned int width, const unsigned int height)
{
	WIDTH = width;
	HEIGHT = height;

	Utility::perspective = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10000.0f);
	glViewport(0, 0, width, height);
}

void Game::ProcessInput(GLFWwindow* window)
{
	
}

void Game::Update()
{
	glfwPollEvents();


	world->RenderChunks();
	player->UpdatePlayer();
	world->SetupChunks();

	ProcessInput(window);
	Utility::camera.UpdateCamera();
}

void Game::LoadTextureAtlas()
{
	unsigned int width, height;
	std::vector<unsigned char> image;

	unsigned int error = lodepng::decode(image, width, height, "textures/sprite_sheet.png");

	if (!error) {
		glGenTextures(1, &textureAtlasID);

		glBindTexture(GL_TEXTURE_2D, textureAtlasID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture atlas" << std::endl;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	extern Camera Utility::camera;
	Utility::camera.mouse_callback(window, xpos, ypos);
}

void ResizeWindowCallback(GLFWwindow* window, int width, int height)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

	game->SetScreenSize(width, height);
}


