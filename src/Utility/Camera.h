#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

class Camera {

public:
	glm::mat4 viewSpace = glm::mat4(1.0f);

	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraForwardVector = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUpVector = glm::vec3(0.0, 1.0f, 0.0f);

	float yaw = -90.0f;
	float pitch = 0.0f;

	Camera();

	void UpdateCamera();

	void SetCameraPosition(glm::vec3 pos) { cameraPosition = pos; }

	void mouse_callback(struct GLFWwindow* window, double x, double y);

	int lastX = -1, lastY = 0;
};

namespace Utility {

	extern Camera camera;

}
