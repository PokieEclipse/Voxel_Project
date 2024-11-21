#include "Camera.h"
#include <glfw/glfw3.h>

Camera::Camera()
{
}

void Camera::UpdateCamera()
{
	viewSpace = glm::lookAt(cameraPosition, cameraPosition + cameraForwardVector, cameraUpVector);
}

void Camera::mouse_callback(GLFWwindow* window, double x, double y)
{
	if (lastX == -1)
	{
		lastX = x;
		lastY = y;
	}

	float xOffset = x - lastX;
	float yOffset = lastY - y;
	lastX = x;
	lastY = y;

	const float sens = 0.1f;
	xOffset *= sens;
	yOffset *= sens;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraForwardVector = glm::normalize(direction);
}

namespace Utility {
	Camera camera;
}