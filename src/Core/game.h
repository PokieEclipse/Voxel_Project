#pragma once
#include <glew.h>
#include <glfw/glfw3.h>

#include <string>

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft {
	class World;
	class Player;
}

namespace Utility {
	extern glm::mat4 perspective;
}

class Game {

public:

	Game();
	~Game();
	int SetupGame();
	void MainLoop();
	void ShutdownGame();

	bool IsShuttingDown() const { return shutdownState; };

	// Gets Screen Width
	unsigned int GetScreenWidth() const { return WIDTH; }
	// Gets Screen Height
	unsigned int GetScreenHeight() const { return HEIGHT; }

	// Sets screen size
	void SetScreenSize(const unsigned int width, const unsigned int height);

	// Gets a reference to the game window
	GLFWwindow* GetGameWindow() const { return window; }

	unsigned int GetTextureAtlasID() const { return textureAtlasID; }

	class Minecraft::Player* GetPlayerReference() { return player; }
	class Minecraft::World* GetWorldReference() { return world; }

	void ProcessInput(GLFWwindow* window);

	void Update();

private:
	unsigned int WIDTH = 1920;
	unsigned int HEIGHT = 1080;

	GLFWwindow* window;

	bool shutdownState = false;

	void LoadTextureAtlas();

	unsigned int textureAtlasID;

	// WORLD
	class Minecraft::World* world;

	// PLAYER
	class Minecraft::Player* player;

};

void ResizeWindowCallback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
