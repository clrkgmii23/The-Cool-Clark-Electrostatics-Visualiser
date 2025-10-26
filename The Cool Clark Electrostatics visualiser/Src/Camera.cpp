#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "utils.h"


Camera::Camera(glm::vec3 camPos, glm::vec3 targetPos, glm::vec3 worldUp, float x_rot, float y_rot, float x_rot_sen, float y_rot_sen,
	float speedX, float speedY) :camPos(camPos), x_rot(x_rot), y_rot(y_rot), worldUp(worldUp), x_rot_sen(x_rot_sen), y_rot_sen(y_rot_sen),
	targetPos(targetPos), speedX(speedX), speedY(speedY)
{
	CalculateVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(camPos, targetPos, camUp);
}

void Camera::CalculateVectors(){
	glm::vec3 camPosSphere;
	camPosSphere.x = cos(glm::radians(x_rot)) * cos(glm::radians(y_rot));
	camPosSphere.y = sin(glm::radians(y_rot));
	camPosSphere.z = sin(glm::radians(x_rot)) * cos(glm::radians(y_rot));
	camPos =  targetPos + camRad * glm::normalize(camPosSphere);
	
	camFront = glm::normalize(targetPos - camPos);
	camRight = normalize(glm::cross(camFront, worldUp));
	camUp = normalize(glm::cross(camRight, camFront));
}

void Camera::HandleMouseInput(float xpos, float ypos, bool isShift) {
	float deltaX = xpos - lastX;
	float deltaY = ypos - lastY;

	if (!isShift){
		x_rot += deltaX * x_rot_sen;
		y_rot += deltaY * y_rot_sen;

		if (y_rot > 89.0f)
			y_rot = 89.0f;

		else if (y_rot < -89.0f)
		y_rot = -89.0f;
	}
	else {
		targetPos -= camRight * deltaX * speedX;
		targetPos += camUp * deltaY * speedY;
	}
	CalculateVectors();
}

void Camera::HandleScroll(float xoffset, float yoffset)
{
	camRad -= yoffset * 0.1f;
	if (camRad < 0.1f)
		camRad = 0.1f;
	else if (camRad > 100.0f)
		camRad = 100.0f;

	CalculateVectors();
}
